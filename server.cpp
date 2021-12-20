#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<queue>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

#define BUFFER_SIZE 1 //Tamanho das mensagens
#define num_max_fila 5

int num_vagas = 5;

void error(const char *msg_error)
{
    perror(msg_error);
    exit(1);
}

int main(int argc, char *argv[])
{
    int vagas_ocupadas = 0;

    //Recebe o nome do arquivo e numero de PORT
    if(argc < 2)
        error("ERRO: Insira o nro do PORT");

    int sockfd, newSockFd, portNum, n;
    struct sockaddr_in serv_addr, cli_addr;
    char buffer[BUFFER_SIZE];
    socklen_t clilen;

    //Criar o socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
        error("ERRO: Criação do socket mal sucedida");

    //Configurando o servidor
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portNum = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portNum); //Função htons garante que serv e cli se comuniquem em um mesmo "endian"

    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        error("Erro na configuração do servidor");
    }
    //Configuração inicial do servidor concluída

    //Preparar o servidor para "ouvir"
    listen(sockfd, num_max_fila + num_vagas); //Numero limite de clientes
    clilen = sizeof(cli_addr);

    //Coloca um valor inválido por padrão
    newSockFd = -4;
    
    //Variável da mensagem a ser enviada para o cliente
    char msg;
    int tam; //Guarda o tamanho da fila
    std::queue<int> fila_est; //Fila de pessoas aguardando vaga

    //Agora o programa entra em loop, e faz o que deve ser feito
    while(1)
    {
        //Aceitar a conexão
        newSockFd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if(newSockFd < 0)
            error("Erro ao aceitar clizente");
        else
        {
            //Ler a mensagem do cliente
            int receive = recv(newSockFd, buffer, BUFFER_SIZE, 0);
            if (receive > 0)
            {
	            if(strlen(buffer) < 0)
                    continue;

                if(buffer[0] == 'E')    //O usuario deseja entrar
                {
                    if(vagas_ocupadas < num_vagas)   //O cliente vai conseguir entrar
                    {
                        vagas_ocupadas++;
                        msg = 'C';
                        send(newSockFd, &msg, sizeof(msg), 0);
                        //O programa cliente terminará a conexão automaticamente
                    }
                    else            //O cliente não vai conseguir entrar
                    {
                        //Ficará na fila
                        msg = 'F';
                        send(newSockFd, &msg, sizeof(msg), 0); //Avisa o cliente que ele ficará na fila
                        
                        //Codigo da fila aqui, provavelmente envolvendo thread
                        fila_est.push(newSockFd);
                    }
                }
                if(buffer[0] == 'S' && vagas_ocupadas > 0)  //O usuario desesja sair
                {
                    vagas_ocupadas--;

                    //Saída bem sucedida
                    msg = 'K';
                    send(newSockFd, &msg, sizeof(msg), 0);
                
                    if(!fila_est.empty()) //Se tiverem elementos na fila
                    {
                        vagas_ocupadas++;

                        //Resgata o proximo elemento da fila
                        newSockFd = fila_est.front();
                        fila_est.pop();

                        msg = 'C';
                        send(newSockFd, &msg, sizeof(msg), 0);
                        //O programa cliente terminará a conexão automaticamente
                    }
                        
                }
            }
        }

        tam = fila_est.size();
        printf("\n Vagas disponíveis: %d\n", num_vagas - vagas_ocupadas);
        if(!fila_est.empty())
            printf("Quantidade de pessoas na fila: %d\n", tam);
        fflush(stdout);
    }

    close(newSockFd);
    return 0;
}
