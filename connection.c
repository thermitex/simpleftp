#include "connection.h"

int socket_open(int *sockfd, struct sockaddr_in *servaddr, int port, char *serverip) {
    servaddr->sin_family = AF_INET;
    servaddr->sin_addr.s_addr = inet_addr(serverip);
    servaddr->sin_port = htons(port);

    // Open socket
    return (*sockfd = socket(AF_INET, SOCK_STREAM, 0)) >= 0;
}

int socket_send(int sockfd, void *msg, size_t len) {
    char *ptr = (char*) msg;
    while (len > 0)
    {
        int ret = send(socket, ptr, len, 0);
        if (ret < 1) return ret;
        ptr += ret;
        len -= ret;
    }
    return 1;
}
int socket_recv(int sockfd, void *buffer, size_t len) {
    ssize_t bytesRead = 0;
    while (bytesRead < len) {
        ssize_t ret = recv(sockfd, buffer, len, 0);
        if (ret < 1) return ret;
        bytesRead += ret;
    }
}