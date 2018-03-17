/* This is a simple webserver.*/

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAXBUF		1024
#define OPEN_MAX	100
#define LISTEN_BACKLOG	 50

/* LT模式 */
#define EPOLL_LT 0
/* ET模式 */
#define EPOLL_ET 1

 #define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

void accept_request(int);
void bad_request(int);
void setNonBlock(int);
void headers(int, const char *);
int startup(short port);
/* 
 set nonblock
*/
void  setNonBlock(int fd) {
	int opts;
	opts = fcntl(fd, F_GETFL);
	if (opts < 0) {
		handle_error("fcntl(fd,GetFl)");
 	}
    opts = opts | O_NONBLOCK;
    if (fcntl(fd, F_SETFL, opts) <0 ) {
    	handle_error("fcntl(fd,Setfl)");
    }
}

/*
 create  socket in port 
 return listenfd 
*/
int startup(short port) {
	int listenfd = 0,reuse = 1;
	struct sockaddr_in serv_addr;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);	
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
        handle_error("setsockopt() error");
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);
	if (bind(listenfd, (struct sockaddr *) &serv_addr,
           sizeof(serv_addr)) == -1)
        handle_error("bind");

   if (listen(listenfd, LISTEN_BACKLOG) == -1)
       handle_error("listen");
   	return  listenfd;
}

int main(int argc, char  *argv[])
{
	int listenfd;
	int port, connfd;
	char buf[MAXBUF];
	char sendBuff[MAXBUF];
	struct sockaddr_in client_addr;
	socklen_t clilen;
	time_t ticks;
	if (argc != 2){
		handle_error("use comand  port");
	}
	port = atoi(argv[1]);
	listenfd = startup(port);	 
 	sprintf(buf, "httpd running on port %d\n", port);
 	puts(buf);
 	while (1) {
		memset(sendBuff, '\0', sizeof(sendBuff));	
		memset(buf, '\0', sizeof(buf));
	 	connfd = accept(listenfd, (struct sockaddr *)&client_addr , &clilen); 
	 	if (connfd == -1)
	 		handle_error("accept");
	 	char *str = inet_ntoa(client_addr.sin_addr);
		printf("connect from %s\n", str);
		printf("accept  fd is %d\n", connfd);
	 	if(read(connfd, buf, sizeof(buf)) < 0)
 			close(connfd);
 		ticks = time(NULL);
 		//send client time
 		//snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
 		//write(connfd, sendBuff, strlen(sendBuff));
		strcpy(sendBuff, "HTTP/1.0 200 OK\r\n");
		send(connfd, sendBuff, strlen(sendBuff), 0);
 		sprintf(sendBuff, "Content-Type: text/html\r\n\r\n");
		send(connfd, sendBuff, strlen(sendBuff), 0);
		strcpy(sendBuff, "hi tinyhttpd\r\n");
		send(connfd, sendBuff, strlen(sendBuff), 0);
		//send(connfd, buf, strlen(buf), 0);

 		close(connfd);
 	}
	return 0;
}