#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <netdb.h>
#include "transmit.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        usage();
        exit(1);
    }
    int port_num = atoi(argv[1]);       // assign port from input
    char *port = argv[1];
    char buffer[BUFFER_SIZE];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);

    int sockfd;                         // socket file descriptor

    // Empty addresses
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Set server details
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(port_num); 

    // Open socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) die("socket");

    // Bind socket
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) die("bind");

    printf("Initialization completed. Listening for connection...\n");

    while(1) {
        easy_recv(sockfd, buffer, &cliaddr, len, 1);
        char *answer_msg = get_answer(buffer);
        easy_send(sockfd, answer_msg, strlen(answer_msg), cliaddr, len, 1);

        // ftp started
        if (strcmp(answer_msg, "yes") == 0) {
            printf("Connection accepted. Waiting for incoming packets...\n");
            int file_exist_flag = 0;
            while(1) {
                easy_recv(sockfd, buffer, &cliaddr, len, 0);
                answer_msg = "ACK";
                easy_send(sockfd, answer_msg, strlen(answer_msg), cliaddr, len, 1);

                packet_t *pk = (packet_t *)malloc(sizeof(packet_t));
                msg_to_pk(pk, buffer);
                printf("*****Packet received %d/%d*****\n", pk->frag_no, pk->total_frag);
                #if DEBUG
                printf("\n========filedata starts========\n%s\n=========filedata ends=========\n\n", pk->filedata);
                #endif
                if(pk->frag_no == 1 && access(pk->filename, F_OK) == 0) file_exist_flag = 1;
                if (file_exist_flag) {
                    printf("File already exists. Skipping writing process.\n");
                } else {
                    write_to_file(pk);
                }
                if (pk->frag_no == pk->total_frag) {
                    destroy(pk);
                    break;
                }
                destroy(pk);
            }
            printf("All packets received. Listening for new connection...\n"); 
        }
    }

}

void usage() {
    fprintf(stderr, "Usage: server <UDP listen port>\n");
}

char* get_answer(char *msg) {
    if (strcmp("ftp", msg) == 0) {
        return "yes";
    } else {
        return "no";
    }
    return "ACK";
}