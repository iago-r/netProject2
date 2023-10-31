#include "common.h"
//#include "server_lib.h"

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

struct BlogOperation {
    int client_id;
    int operation_type;
    int server_response;
    char topic[50];
    char content[2048];
};

struct Topic {
    char topic_name[50];
    int subscribers[11]; // 0 in a position = not sub; 1 = sub;
    char last_content_published[2048];
    struct Topic* next;
};

struct TopicList{
    struct Topic* head;// = NULL;
    struct Topic* tail;// = NULL;
};

void initializeListOfTopics();
struct Topic* searchTopic(struct BlogOperation *msg);
struct Topic* createTopic(struct BlogOperation *msg);
void publish(struct BlogOperation *msg);
void subscribe(struct BlogOperation *msg);
void unsubscribe(struct BlogOperation *msg);
void listTopics();
void commandParse(struct BlogOperation *msg);
void printMsg(struct BlogOperation *msg);

void assignID();

struct TopicList Topics;
struct BlogOperation msg_to_send;

int sockets[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int main(int argc, char **argv) {
    initializeListOfTopics(Topics);
    
    if (argc < 3) {
        usage(argc, argv);
    }

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
    
    //printf("client %2.i connected\n", id);
    printf("client connected\n");
    
    char buffer[BUFSZ];
    struct BlogOperation msg_received;
    while (1) {
        //RECEIVING PACKAGE.............................................       
        // bzero(buffer, sizeof(buffer));
        // recv(cdata->csock, buffer, sizeof(buffer), 0);
        // printf("%s", buffer);
        bzero(buffer, sizeof(buffer));
        recv(cdata->csock, buffer, sizeof(buffer), 0);
        //commandParse();

        //SENDING PACKAGE...............................................
        bzero(buffer, sizeof(buffer));
        printf("> "); fgets(buffer, BUFSZ-1, stdin);
        send(cdata->csock, buffer, sizeof(buffer), 0);

        // EXIT.........................................................













        // CLEANING BUFFERS ............................................
        bzero(&msg_received, sizeof(msg_received));
        bzero(&msg_to_send, sizeof(msg_to_send));

        //RECEIVING PACKAGE.............................................       
        recv(cdata->csock, buffer, sizeof(buffer), 0);
        commandParse(&msg_received);

        
        send(cdata->csock, buffer, sizeof(buffer), 0);

    }
    //dischargeId(id);
    close(cdata->csock);
    pthread_exit(EXIT_SUCCESS);
}

// O servidor deve decidir se envia a resposta a um ou mais computadores,
// ou seja, o send do server tem que ser feito seletivamente



struct connection {
    int status;
    int csock;
    int subscriptions[100];
};

struct connection connections[11];


void initializeListOfTopics() {
    Topics.head = NULL;
    Topics.tail = NULL;
}

struct Topic* searchTopic(struct BlogOperation *msg) {
    struct Topic *cursor = Topics.head;
    while (cursor != NULL) {
        if (strcmp(cursor->topic_name, msg->topic) == 0) {
            return cursor;
        }
        cursor = cursor->next;
    }
    return cursor;
}

struct Topic* createTopic(struct BlogOperation *msg) {
    struct Topic* cursor = malloc(sizeof(struct Topic)) ;
    strcpy(cursor->topic_name, msg->topic);
    for (int i = 0; i < 11; i++)
        cursor->subscribers[i] = 0;
    cursor->next = NULL;
    if (Topics.head == NULL) {
        Topics.head = cursor;
        Topics.tail = cursor;
    }
    else {
        Topics.tail->next = cursor;
        Topics.tail = cursor;
    }
    return cursor;
}

void publish(struct BlogOperation *msg) {
    struct Topic *topic_to_publish = searchTopic(msg);
    if (topic_to_publish == NULL) {
        topic_to_publish = createTopic(msg);
    }
    strcpy(topic_to_publish->last_content_published, msg->content);
}

void subscribe(struct BlogOperation *msg) {
    struct Topic *topic_to_subscribe = searchTopic(msg);
    if (topic_to_subscribe == NULL) {
        topic_to_subscribe = createTopic(msg);
    }
    topic_to_subscribe->subscribers[msg->client_id] = 1;
}

void unsubscribe(struct BlogOperation *msg) {
    struct Topic *topic_to_unsubscribe = searchTopic(msg);
    if (topic_to_unsubscribe != NULL) {
        topic_to_unsubscribe->subscribers[msg->client_id] = 0;
    }
}

void listTopics() {
    struct Topic *cursor = Topics.head;
    if (cursor == NULL) {
        strcpy(msg_to_send.content, "no topics available");
    }
    while (cursor != NULL) {
        strcat(msg_to_send.content, cursor->topic_name);
        cursor = cursor->next;
        if (cursor != NULL) {
            strcat(msg_to_send.content, "; ");
        }
    }
    strcat(cursor->topic_name, "\n");
}




void displayTopics() {
    struct Topic *cursor = Topics.head;
    if (cursor == NULL) {
        printf("no topics available");
    }
    while (cursor != NULL) {
        printf("%s", cursor->topic_name);
        cursor = cursor->next;
        if (cursor != NULL) {
            printf("; ");
        }
    }
    printf("\n");
}
/* 
 */
void commandParse(struct BlogOperation *msg){


  switch (msg->operation_type)
  {
    // CONNECT
    case 1:
      assignID();
      break;

    // PUBLISH
    case 2:
      publish(msg);
      //listTopics(Topics);
      break;
    
    /* 
    // LIST
    case 3:
      msg_to_send.operation_type = 1;
      msg_to_send.server_response = 1;
      strcpy(msg_to_send.topic, "");
      strcpy(msg_to_send.content, "");
      break;
    
    // SUBSCRIBE
    case 4:
      break;
    
    // EXIT
    case 5:
      break;
    
    // UNSUBSCRIBE
    case 6:
      break;
     */
    // ERRORS
    default:
      break;
  }  
}
/* 
int* assignID() {
  for (int i = 1; i < 11; i++) {
    if (sockets[i] == 0) {
      return &sockets[i];
    }
  }
  return NULL;
} */

//printf("Checkpoint\n");

//====================================// FUNCOES DE AUXILIO //====================================//

void assignID() {
  // ESTUDAR UMA FORMA DE ATRIBUIR UM ID PARA O CLIENT AQUI
  msg_to_send.client_id = 1;
  //=======================================================
  msg_to_send.operation_type = 1;
  msg_to_send.server_response = 1;
  strcpy(msg_to_send.topic, "");
  strcpy(msg_to_send.content, "");
}

void printMsg(struct BlogOperation *msg) {
  printf("\n");
  printf("client_id: %i\n", msg->client_id);
  printf("server_response: %i\n", msg->server_response);
  printf("operation_type: %i\n", msg->operation_type);
  printf("msg->topic: %s\n", msg->topic);
  printf("msg->content: %s\n", msg->content);
  printf("\n");
}