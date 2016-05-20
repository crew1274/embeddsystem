#include <netinet/in.h>   	// for sockaddr_in
#include <sys/types.h>    	// for socket
#include <sys/socket.h>    	// for socket
#include <stdio.h>        	// for printf
#include <stdlib.h>        	// for exit
#include <string.h>        	// for bzero

#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512

int main(int argc, char **argv)
{
    //setting socket address
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr)); 				
    server_addr.sin_family = PF_INET;  // POSIX Internet
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    
	//MMN_Black_1  set the server port number 1025~65535  
	server_addr.sin_port = htons(5001);  // server port
	
    //Creating a BSD Socket descriptor
    //MMN_Black_2  open a socket , return the file descriptor of the socket
    int server_socket = socket(PF_INET, SOCK_STREAM , 0);  
    if( server_socket == -1)
    {
        printf("Create Socket Failed!\n");
        exit(1);
    }
 
   int opt =1;
   setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

   
    //Binding an Address to Socket
    //MMN_Black_3	
    if( bind( server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)))  
    {
        printf("Server Bind Port : %d Failed!\n", server_addr.sin_port); 
        exit(1);
    }

    //Listening on  an INET socket
    //MMN_Black_4
    if ( listen( server_socket, LENGTH_OF_LISTEN_QUEUE) )  
    {
        printf("Server Listen Failed!"); 
        exit(1);
    }
    printf("initial success !! Listening ...\n");
	
    while (1) 
    {
        //define the client socket address structure 
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);

        //接受一个到server_socket代表的socket的一個連接
        //當server accept client 端 socket connect 請求
		//則會產生一個新的 socket 結構(new_server_socket)，用於server/client 間的通訊 
        //MMN_Black_5
	int new_server_socket = accept( server_socket,(struct sockaddr*)&client_addr,&length);  
        if ( new_server_socket < 0)
        {
            printf("Server Accept Failed!\n");
            break;
        }
        
        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);
		
	// Receive the "file name" from the file descriptor of connection to client 
        length = recv(new_server_socket,buffer,BUFFER_SIZE,0);
        if (length < 0)
        {
            printf("Server Recieve Data Failed!\n");
            break;
        }
        char file_name[FILE_NAME_MAX_SIZE+1];
        bzero(file_name, FILE_NAME_MAX_SIZE+1);
		
	// Copy the file name from buffer to the fine_name array
        strncpy(file_name, buffer, strlen(buffer)>FILE_NAME_MAX_SIZE?FILE_NAME_MAX_SIZE:strlen(buffer));

        printf("%s\n",file_name);
        FILE * fp = fopen(file_name,"r");  
        if(NULL == fp )
        {
            printf("File:\t%s Not Found\n", file_name);
        }
        else
        {
            bzero(buffer, BUFFER_SIZE);
            int file_block_length = 0;
            while( (file_block_length = fread(buffer,sizeof(char),BUFFER_SIZE,fp))>0)  
            {
                printf("file_block_length = %d\n",file_block_length);
                
				//將 buffer 資料send to new_server_socket(client)
                		//MMN_Black_6
				if( send(new_server_socket,buffer,file_block_length,0)<0) 
                {
                    printf("Send File:\t%s Failed\n", file_name);
                    break;
                }
                bzero(buffer, BUFFER_SIZE);
            }
            fclose(fp);
            printf("File:\t%s Transfer Finished\n",file_name);
        }
        //關閉 client 端連結
        close(new_server_socket);  
    }
    //關閉 server socket
    close(server_socket); 
    return 0;
}
