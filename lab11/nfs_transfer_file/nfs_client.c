#include <netinet/in.h>    	// for sockaddr_in
#include <sys/types.h>    	// for socket
#include <sys/socket.h>   	// for socket
#include <stdio.h>        	// for printf
#include <stdlib.h>        	// for exit
#include <string.h>        	// for bzero
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: ./%s Server IP Address\n",argv[0]);
        exit(1);
    }

    //Creating a client Socket descriptor (TCP)	
    //MMN_Black_1 open a socket
    int client_socket = socket(PF_INET, SOCK_STREAM, 0); 
    if( client_socket < 0)
    {
        printf("Create Socket Failed!\n");
        exit(1);
    }
   
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    
    server_addr.sin_family = PF_INET;
    // Get server IP address
    if(inet_aton(argv[1],&server_addr.sin_addr) == 0)
    {
        printf("Server IP Address Error!\n");
        exit(1);
    }
	
    //MMN_Black_2  the server port number you set in server.c
    server_addr.sin_port = htons(5001); 
    socklen_t server_addr_length = sizeof(server_addr);
    
    //establish a connection with the server 
    if(connect(client_socket,(struct sockaddr*)&server_addr, server_addr_length) < 0)  
    {
        printf("Can Not Connect To %s!\n",argv[1]);
        exit(1);
    }

    char file_name[FILE_NAME_MAX_SIZE+1];
    bzero(file_name, FILE_NAME_MAX_SIZE+1);
    printf("Please Input File Name On Server:\t");
    scanf("%s", file_name);
    
    char buffer[BUFFER_SIZE];
    bzero(buffer,BUFFER_SIZE);
    // copy the "file name" to the buffer which will be sent to the server
	strncpy(buffer, file_name, strlen(file_name)>BUFFER_SIZE?BUFFER_SIZE:strlen(file_name));
    
	// MMN_Black_3  send the content of buffer to the server
	send(client_socket,buffer,BUFFER_SIZE,0);  

    FILE * fp = fopen("output_file.txt","w");  
    if(NULL == fp )
    {
        printf("File:\t%s Can Not Open To Write\n", file_name);
        exit(1);
    }
	
    bzero(buffer,BUFFER_SIZE);
    int length = 0;
	
    //Receive the content of buffer from the socket file descriptor of connection to server 
    //MMN_Black_4
    while( length = recv(client_socket,buffer,BUFFER_SIZE,0))  
    {
        if(length < 0)
        {
            printf("Recieve Data From Server %s Failed!\n", argv[1]);
            break;
        }

        int write_length = fwrite(buffer,sizeof(char),length,fp); 
        if (write_length<length)
        {
            printf("File:\t%s Write Failed\n", file_name);
            break;
        }
        bzero(buffer,BUFFER_SIZE);    
    }
    printf("Recieve File:\t %s From Server[%s] Finished\n",file_name, argv[1]);
    
    close(fp);
	
    //close socket
    close(client_socket);  
    return 0;
}
