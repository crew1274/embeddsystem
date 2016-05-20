#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>   // for socket
#include <sys/socket.h>  // for socket
#include <netinet/in.h>  // for sockaddr_in
#include <netdb.h>
#include <arpa/inet.h>
#include <err.h>

 
char response[] = "HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html><html><head><title>Hello world!</title>"
"<style>body { background-color: #111 }"
"h1 { font-size:4cm; text-align: center; color: black;"
" text-shadow: 0 0 2mm red}</style></head>"
"<body><h1>Hello world!</h1></body></html>\r\n";


 
int main()
{
	int one = 1, client_fd;
	struct sockaddr_in svr_addr, cli_addr;
	socklen_t sin_len = sizeof(cli_addr);
    
    	int port = 8080;
	svr_addr.sin_family = AF_INET;
	svr_addr.sin_addr.s_addr = INADDR_ANY;
	svr_addr.sin_port = htons(port);

 	//MMN_Black_1  open a socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		err(1, "can't open socket");
 
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));
 	   
	//MMN_Black_2  bind a name to the socket
	if ( bind(sock, (struct sockaddr *) &svr_addr, sizeof(svr_addr)) == -1) {
		close(sock);
		err(1, "Can't bind");
	}
 
	//MMN_Black_3  listen for socket connection and limit the queue of incoming connections
	listen(sock, 5);
	
	while (1) {
		//MMN_Black_4  accept a new connection on a socket
		client_fd = accept(sock, (struct sockaddr *) &cli_addr, &sin_len);
		printf("got connection\n");
 
		if (client_fd == -1) {
		perror("Can't accept");
		continue;
		}
 
		write(client_fd, response, sizeof(response) - 1); /*-1:'\0'*/
		
		//MMN_Black_5 close the file descriptor of the connection to the client
		close(client_fd);
	}	
}
