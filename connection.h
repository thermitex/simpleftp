#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <netdb.h>

#define BUFFER_SIZE         2048
#define DEBUG               1
#define SERVER_CTL_PORT     21
#define SERVER_DTA_PORT     20

int socket_open(int *sockfd, struct sockaddr_in *servaddr, int port, char *serverip);
int socket_send(int sockfd, void *msg, size_t len);
int socket_recv(int sockfd, void *buffer, size_t len);

#endif