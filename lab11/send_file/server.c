#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

int main(void)
{
    int listenfd = 0;
    int connfd = 0;
    struct sockaddr_in serv_addr;
    char sendBuff[1025];
    int numrv;

    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);
	
	//MMN_Black_1 open a socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    printf("Socket retrieve success\n");    

    //MMN_Black_2 bind a name to the socket 
    bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));

    //MMN_Black_3 listen for socket connection and limit the queue of incoming connections
    if(listen(listenfd, 10) == -1)
    {
        printf("Failed to listen\n");
        return -1;
    }


    while(1)
    {
	//MMN_Black_4 accept a new connection on a socket
        connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL);

        /* Open the file that we wish to transfer */
        FILE *fp = fopen("lena.bmp","rb");
        if(fp==NULL)
        {
            printf("File opern error");
            return 1;   
        }   

        /* Read data from file and send it */
        while(1)
        {
            /* First read file in chunks of 256 bytes */
            unsigned char buff[256]={0};
            int nread = fread(buff,1,256,fp);
            printf("Bytes read %d \n", nread);        

            /* If read was success, send data. */
            if(nread > 0)
            {
                printf("Sending \n");
				
		//MMN_Black_5 write the content of buffer to the file descriptor of connection to the client 	
                write(connfd, buff, nread);
            }

            /*
             * There is something tricky going on with read .. 
             * Either there was error, or we reached end of file.
             */
            if (nread < 256)
            {
                if (feof(fp)){
                    printf("End of file\n");
		    break;
		}
                if (ferror(fp))
                    printf("Error reading\n");
                break;
            }


        }

        close(connfd);
        sleep(1);
    }


    return 0;
}
