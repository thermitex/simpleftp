#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <time.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <netdb.h>
#include <math.h>
#include "common.h"

int main(int argc, char *argv[]) {
	if (argc != 3) {
        usage();
        exit(1);
    }
    int port_num = atoi(argv[2]);       // assign port from input
    char *port = argv[2];
    char *servaddr_str = argv[1];
    char buffer[BUFFER_SIZE];
    struct sockaddr_in servaddr;
    socklen_t len = sizeof(servaddr);

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(servaddr_str);
    servaddr.sin_port = htons(port_num);

    int sockfd;                         // socket file descriptor

    // Open socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) die("socket");

    printf("Initialization completed.\n");

    // Ask for user input
    char filename[100];
    char init_msg[10];
    printf("Enter command:\n");
    scanf("%s %s", init_msg, filename);

    if(access(filename, F_OK) == -1) {
        fprintf(stderr, "File not found. Terminating program.\n");
        exit(1);
    }

    easy_send(sockfd, init_msg, strlen(init_msg), servaddr, len, 1);
    easy_recv(sockfd, buffer, &servaddr, len, 1);
    init_msg_action(buffer);

    // open file
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL) die("fopen");
    #if DEBUG
    printf("[fopen] File successfully opened.\n"); 
    #endif

    // get total packages
    fseek(fp, 0L, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    unsigned int total_pk = file_size / 1000 + 1;

    // start packaging
    int pk_count = 1;
    int eof = 0;
    float total_rtt = 0;
    while(1) {
        packet_t *pk = (packet_t *)malloc(sizeof(packet_t));
        size_t ret = fread(pk->filedata, sizeof(char), 1000, fp);
        pk->filename = filename;
        pk->size = ret;
        pk->total_frag = total_pk;
        if (pk_count > total_pk) die("package number estimation error");
        pk->frag_no = pk_count;
        if (ret < 1000) {
            pk->filedata[ret] = '\0';
            eof = 1;
        };
        pk_count++;
        #if DEBUG
        printf("\n==========package detail==========\n");
        printf("total_frag: %d\nfrag_no: %d\nsize: %d\nfilename: %s\n", 
                pk->total_frag, pk->frag_no, pk->size, pk->filename);
        #endif

        // packaging data
        char *pk_msg = malloc(get_digits(pk->total_frag) + \
                                get_digits(pk->frag_no) + \
                                get_digits(pk->size) + \
                                strlen(filename) + pk->size + 10);
        size_t pk_len = pk_to_msg(pk, pk_msg);
        #if DEBUG
        printf("final_msg:\n%s", pk_msg);
        printf("\n==================================\n"); 
        #endif

        // send packet to server
        while(1) {
            easy_send(sockfd, pk_msg, pk_len, servaddr, len, 0);
            clock_t rtt_start = clock();
            #if DEBUG
            printf("[sendto] Package sent, waiting for acknowledgement...\n"); 
            #endif

            easy_recv(sockfd, buffer, &servaddr, len, 1);
            if (strcmp(buffer, "ACK") == 0) {
                clock_t rtt_end = clock();
                float rtt = (float) (rtt_end - rtt_start) * 1000 / CLOCKS_PER_SEC;
                total_rtt += rtt;
                #if DEBUG
                printf("RTT measured: %f msec\n", rtt);
                #endif
                break;
            }
        }
        free(pk_msg);

        free(pk);
        if (eof) break;
    }

    printf("All packages sent successfully.\n");

    printf("Average RTT: %f msec\n", (float)(total_rtt / (float)pk_count));

}

void usage() {
    fprintf(stderr, "Usage: deliver <server address> <server port number>\n");
}

void init_msg_action(char *msg) {
    if (strcmp(msg, "yes") == 0) {
        printf("A file transfer can start.\n");
    } else {
        fprintf(stderr, "Server refused the transfer.\n");
        exit(1);
    }
}
