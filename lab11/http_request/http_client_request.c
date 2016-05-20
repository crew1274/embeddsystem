#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>  // for struct sockaddr_in AF_IN
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>  // for inet_pton
#include <netinet/in.h> // for IPPROTO_TCP

#define PORT "80"
#define BAIDU_IP "64.233.187.94"  // connect to www.google.com.tw ip
#define GET_INFO "GET / HTTP/1.1\r\n\r\n"  //Send a HTTP Request
#define BUFSIZE 2000

int main()
{
    int sock_local;
    in_port_t port = atoi(PORT);
    struct sockaddr_in server;

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

	//MMN_Black_1  open a socket
    sock_local = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
    assert(sock_local > 0);

    int retVal = -1;
    retVal = inet_pton(AF_INET, BAIDU_IP, &server.sin_addr.s_addr);
    assert(retVal == 1);

	//MMN_Black_2  connect to web server 
    if ( bind(sock_local , (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

	//MMN_Black_3  send a HTTP Request
    ssize_t nByte = send(sock_local, GET_INFO, strlen(GET_INFO), 0);
    if( nByte <= 0)
    {
        perror("send");
        exit(EXIT_FAILURE);
    }

    char BUF[BUFSIZ];
    size_t recived_len = 0;
	
	//MMN_Black_4  receive a HTTP Response
    if ((recived_len = recv(sock_local, BUF, BUFSIZ-1, 0)) < 0)
    {
        perror("recv");
    }
        printf("%s",BUF); 
    
          

    close(sock_local);
    return 0;
}
