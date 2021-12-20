#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#define TAMANHO_msg 1
#define TAMANHO_buff 1

// Variaveis globais dos usuarios
int sockfd = 0;
int flag = 0;
char estado;

void error(const char *msg_error)
{
    perror(msg_error);
    exit(1);
}

//Função que deixa a msg de entrada pronta
void formata_string_entrada (char* arr, int length) 
{
    int i;
    for (i = 0; i < length; i++)
    if (arr[i] == '\n')
    {
        arr[i] = '\0';
        break;
    }
}

void termina_conexao_e_programa(int sig)
{
    close(sockfd);
    exit(0);
}

int main(int argc, char **argv)
{
	if(argc  < 2)
        error("ERRO: Insira o port como argumento");

    if(argc  < 3)
        error("ERRO: Insira sua ação, 'E' para entrar 'S' para sair");

	char ip[11], msg;
    strcpy(ip, "127.0.0.1\0");
	int port = atoi(argv[1]);
    
    msg =  *argv[2];

    msg = toupper(msg);
    if(msg != 'E' && msg != 'S')
        error("ERRO: Entrada inválida inserida. 'E' para entrar 'S' para sair");

    //Adiciona o sinal de saída do programa
	signal(SIGINT, termina_conexao_e_programa);

    //Iniciar configurações do socket
	struct sockaddr_in server_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);


    // Buscar conexão com servidor
    int conec = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (conec == -1)
		error("ERROR: connect\n");


    /****** Conexão estabelecida *******/
	// Envia o estado
	send(sockfd, &msg, sizeof(char), 0);

    //Buffer que vai receber as respostas do servidor
    char buffer[TAMANHO_buff];
    
    //Prepara o buffer
    bzero(buffer, TAMANHO_buff);
    
	int rec = recv(sockfd, buffer, TAMANHO_buff, 0); //Ouve por respostas
            
    if (rec > 0) 
    {
        //A mensagem foi recebida com sucesso
        if(buffer[0] == 'C')    //A entrada no estacionamento foi confirmada
        {
            printf("Entrada no estacionamento liberada!!\n\n");
            termina_conexao_e_programa(2);
        }
        else if(buffer[0] == 'F') //O carro está na fila para entrar.
        {
            printf("Você está na fila agora, aguarde para entrar\n\n");

            //Fica na fila até receber o aceite
            bzero(buffer, TAMANHO_buff);
            rec = recv(sockfd, buffer, TAMANHO_buff, 0);

            if(rec > 0 && buffer[0] == 'C') //O carro conseguiu a vaga!
            {
                printf("Você conseguiu a sua vaga! Entrada no estacionamento liberada!\n\n");
                termina_conexao_e_programa(2);
            }
            else
            {
                printf("Algo deu errado\n");
                termina_conexao_e_programa(2);
            }
        }   
        else if(buffer[0] == 'K')   //A saída foi bem sucedida!
        {
            printf("Obrigado por estacionar conosco, volte sempre!");
            termina_conexao_e_programa(2);
        }
    }  
    else
    {
        error("ERRO ao receber msg\n");
        termina_conexao_e_programa(2);
    }

	close(sockfd);
	return 0;
}