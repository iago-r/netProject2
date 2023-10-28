#include "common.h"
#include "server_lib.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#define BUFSZ 1024

struct client_data {
    int csock;
    struct sockaddr_storage storage;
};

void usage(int argc, char **argv);
void* client_thread(void* data);

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argc, argv);
    }

    //initializeIdList();

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
    //if(0 != setsockopt(s, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(int))) {
    if(0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("bind");
    }

    if (0 != listen(s, 10)) { // quantidade de conexões que podem estar pendentes para tratamento
        logexit("listen");
    }
    
    while (1) {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);
        
        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logexit("accept");
        }
        
        struct client_data* cdata = malloc(sizeof(*cdata));
        if (!cdata) {
            logexit("malloc");
        }
        cdata->csock = csock;
        memcpy(&(cdata->storage), &cstorage, sizeof(cstorage));
        
        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, cdata);
        
    }
    exit(EXIT_SUCCESS);
}

void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

void* client_thread(void* data) {
    struct client_data* cdata = (struct client_data *)data;
    //struct sockaddr *caddr = (struct sockaddr *)(&cdata->storage);

   /*  int id = assignId();
    if (-1 == id) {
        logexit("id");
    } */
    
    //printf("client %2.i connected\n", id);
    printf("client connected\n");
    
    char buffer[BUFSZ];
    while (1) {
        //RECEIVING PACKAGE.............................................       
        bzero(buffer, sizeof(buffer));
        recv(cdata->csock, buffer, sizeof(buffer), 0);
        printf("%s", buffer);

        //SENDING PACKAGE...............................................
        bzero(buffer, sizeof(buffer));
        printf("> "); fgets(buffer, BUFSZ-1, stdin);
        send(cdata->csock, buffer, sizeof(buffer), 0);

        // EXIT.........................................................
    }
    //dischargeId(id);
    close(cdata->csock);
    pthread_exit(EXIT_SUCCESS);
}

// O servidor deve decidir se envia a resposta a um ou mais computadores,
// ou seja, o send do server tem que ser feito seletivamente

struct connections {
    int id;
    int csock;
    int subscriptions[20];
};