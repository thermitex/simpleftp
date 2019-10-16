#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <netdb.h>
#include <math.h>

#define BUFFER_SIZE 1024
#define DEBUG 0

void die(char *s);
void usage();
char* get_answer(char *msg);
void init_msg_action(char *msg);
int get_digits(int num);
void easy_send(int sockfd, char *msg, size_t size, struct sockaddr_in addr, socklen_t len, int debug_flag);
void easy_recv(int sockfd, char *buffer, struct sockaddr_in *addr, socklen_t len, int debug_flag);

typedef struct packet {
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char* filename;
    char filedata[1000];
} packet_t;

size_t pk_to_msg(packet_t *pk, char *msg);    // transfer the package into a string message
void msg_to_pk(packet_t *pk, char *msg);
void destroy(packet_t *pk);
void write_to_file(packet_t *pk);

#endif