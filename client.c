#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSZ 1024

int CLIENT_ID = 0;

struct BlogOperation {
    int client_id;
    int operation_type;
    int server_response;
    char topic[50];
    char content[2048];
};

void usage(int argc, char** argv);

int main(int argc, char** argv) {
    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != addrparse(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    struct sockaddr* addr = (struct sockaddr *)(&storage);
    if (0 != connect(s, addr, sizeof(storage))) {
        logexit("connect");
    }

    char buffer[BUFSZ];
    while (1) {
        //SENDING PACKAGE...............................................
        bzero(buffer, sizeof(buffer));
        printf("> "); fgets(buffer, BUFSZ-1, stdin);
        send(s, buffer, sizeof(buffer), 0);

        //RECEIVING PACKAGE.............................................       
        bzero(buffer, sizeof(buffer));
        recv(s, buffer, sizeof(buffer), 0);
        printf("%s", buffer);

        // EXIT.........................................................
    }
    close(s);
    exit(EXIT_SUCCESS);
}

void usage(int argc, char **argv) {
    printf("usage %s <server IP> <server port>\n", argv[0]);
    printf("example: %s 127.0.0.1 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}