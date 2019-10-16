#include "common.h"
#include <math.h>

void die(char *s) {
	perror(s);
	exit(1);
}

int get_digits(int num) {
    if (num == 0) return 1;
    int res = 0;
    while (num != 0) {
        num /= 10;
        res++;
    }
    return res;
}

size_t pk_to_msg(packet_t *pk, char *msg) {
    unsigned int char_ptr = 0;
    int size = 0;
    char *buf;

    // total_frag
    size = get_digits(pk->total_frag);
    buf = malloc(size);
    sprintf(buf, "%d", pk->total_frag);
    for (int i = 0; i < size; i++) {
        msg[char_ptr] = buf[i];
        char_ptr++;
    }
    msg[char_ptr++] = ':';

    // frag_no
    size = get_digits(pk->frag_no);
    buf = realloc(buf, size);
    sprintf(buf, "%d", pk->frag_no);
    for (int i = 0; i < size; i++) {
        msg[char_ptr] = buf[i];
        char_ptr++;
    }
    msg[char_ptr++] = ':';

    // size
    size = get_digits(pk->size);
    buf = realloc(buf, size);
    sprintf(buf, "%d", pk->size);
    for (int i = 0; i < size; i++) {
        msg[char_ptr] = buf[i];
        char_ptr++;
    }
    msg[char_ptr++] = ':';

    // filename
    size = strlen(pk->filename);
    sprintf(buf, "%d", pk->size);
    for (int i = 0; i < size; i++) {
        msg[char_ptr] = pk->filename[i];
        char_ptr++;
    }
    msg[char_ptr++] = ':';

    // filedata
    size = pk->size;
    for (int i = 0; i < size; i++) {
        msg[char_ptr] = pk->filedata[i];
        char_ptr++;
    }

    msg[char_ptr] = '\0';

    free(buf);

    return char_ptr;

}

void msg_to_pk(packet_t *pk, char *msg){
    unsigned int char_ptr = 0;
    int colon_cnt = 0;
    pk->total_frag = 0;
    pk->frag_no = 0;
    pk->size = 0;
    int eos = 0;
    while(1) {
        if (msg[char_ptr] == ':') {
            colon_cnt++;
        } else {
            switch(colon_cnt) {
                case 0:
                    pk->total_frag *= 10;
                    pk->total_frag += (int)(msg[char_ptr] - '0');
                    break;
                case 1:
                    pk->frag_no *= 10;
                    pk->frag_no += (int)(msg[char_ptr] - '0');
                    break;
                case 2:
                    pk->size *= 10;
                    pk->size += (int)(msg[char_ptr] - '0');
                    break;
                default:
                    eos = 1;
            }
        }
        char_ptr++;
        if(eos) {
            char_ptr--;
            break;
        }
    }

    // read filename
    pk->filename = malloc(100);
    int fn_idx = 0;
    while(1) {
        if (msg[char_ptr] == ':') {
            pk->filename[fn_idx] = '\0';
            char_ptr++;
            break;
        }
        pk->filename[fn_idx] = msg[char_ptr];
        fn_idx++;
        char_ptr++;
    }

    // read filedata
    int fd_idx = 0;
    for (fd_idx = 0; fd_idx < pk->size; fd_idx++) {
        pk->filedata[fd_idx] = msg[char_ptr];
        char_ptr++;
    }
    if (fd_idx < 1000) pk->filedata[fd_idx] = '\0';

}

void destroy(packet_t *pk) {
    free(pk->filename);
    free(pk);
}

void write_to_file(packet_t *pk) {
    FILE *fp;
    fp = fopen(pk->filename, "a");
    fwrite(pk->filedata, sizeof(char), pk->size, fp);
    fclose(fp);
    #if DEBUG
    printf("File data has been written.\n");
    #endif
}

void easy_send(int sockfd, char *msg, size_t size, struct sockaddr_in addr, socklen_t len, int debug_flag) {
    int status = sendto(sockfd, (const char *)msg, size,
            0, (const struct sockaddr *) &addr, len); 
    if (status < 0) die("sendto");
    if (debug_flag) {
        #if DEBUG
        printf("[sendto] Message sent: %s\n", msg); 
        #endif
    }
}

void easy_recv(int sockfd, char *buffer, struct sockaddr_in *addr, socklen_t len, int debug_flag) {
    int recv_ret; 
    recv_ret = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE,
            MSG_WAITALL, (struct sockaddr *) addr, 
            &len); 
    if (recv_ret < 0) die("recvfrom");
    buffer[recv_ret] = '\0'; 
    if (debug_flag) {
        #if DEBUG
        printf("[recvfrom] Message received: %s\n", buffer); 
        #endif
    }
}