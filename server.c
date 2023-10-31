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
int connect_to_server(struct BlogOperation msg, int socket);
void publish(struct BlogOperation *msg);
void listTopics(struct BlogOperation *msg);
void subscribe(struct BlogOperation *msg);
void disconnect_from_server(struct BlogOperation *msg);
void unsubscribe(struct BlogOperation *msg);
void selectCommand(struct BlogOperation *msg, int client_id);
void printMsg(struct BlogOperation *msg);

void dischargeID(int id);

struct TopicList Topics;
struct BlogOperation msg_to_send;

int sockets[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argc, argv);
    }

    initializeListOfTopics(Topics);

    struct sockaddr_storage storage;
    if (server_sockaddr_init(argv[1], argv[2], &storage) != 0) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
    if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != 0) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (bind(s, addr, sizeof(storage)) != 0) {
        logexit("bind");
    }

    if (listen(s, 10) != 0) { // quantidade de conexões que podem estar pendentes para tratamento
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
    
    //int client_id = connect_to_server(msg, cdata->csock);
    struct BlogOperation msg_received;
    int client_id = connect_to_server(msg_received, cdata->csock);
    printf("client %02i connected\n", client_id);
    
    while (1) {
        // CLEANING BUFFERS ............................................
        bzero(&msg_received, sizeof(msg_received));
        bzero(&msg_to_send, sizeof(msg_to_send));

        //RECEIVING PACKAGE.............................................       
        recv(cdata->csock, &msg_received, sizeof(msg_received), 0);
        selectCommand(&msg_received, client_id);

        //SENDING PACKAGE...............................................
        send(cdata->csock, &msg_to_send, sizeof(msg_to_send), 0);
    }
    
    close(cdata->csock);
    dischargeID(client_id);
    pthread_exit(EXIT_SUCCESS);
}

// O servidor deve decidir se envia a resposta a um ou mais computadores,
// ou seja, o send do server tem que ser feito seletivamente


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

int connect_to_server(struct BlogOperation msg, int socket) {
  int id;
  for (id = 1; id < 11; id++) {
    if (sockets[id] == 0) {
      break;
    }
  }

  if (id > 0 && id < 11) {
    do {
      bzero(&msg, sizeof(msg));
      recv(socket, &msg, sizeof(msg), 0);

      if (msg.operation_type == 1 && msg.server_response == 0){
        sockets[id] = socket;

        msg_to_send.client_id = id;
        msg_to_send.operation_type = id;
        strcpy(msg_to_send.topic, "");
        strcpy(msg_to_send.content, "");
        send(socket, &msg_to_send, sizeof(msg_to_send), 0);

        bzero(&msg, sizeof(msg));
        bzero(&msg_to_send, sizeof(msg_to_send));
        return id;
      }
    } while (msg.operation_type != 1 && msg.server_response != 0);
  }
  return -1;
}

void publish(struct BlogOperation *msg) {
  struct Topic *topic_to_publish = searchTopic(msg);
  if (topic_to_publish == NULL) {
    topic_to_publish = createTopic(msg);
  }
  strcpy(topic_to_publish->last_content_published, msg->content);

  msg_to_send.client_id = msg->client_id;    
  strcpy(msg_to_send.topic, topic_to_publish->topic_name);
  strcpy(msg_to_send.content, topic_to_publish->last_content_published);
  printMsg(&msg_to_send);
  printf("new post added in %s by %02i\n", msg_to_send.topic, msg_to_send.client_id);
}

void listTopics(struct BlogOperation *msg) {
  struct Topic *cursor = Topics.head;
  if (cursor == NULL) {
      strcpy(msg_to_send.content, "no topics available");
  }
  else {
    while (cursor != NULL) {
      strcat(msg_to_send.content, cursor->topic_name);
      cursor = cursor->next;
      if (cursor != NULL) {
          strcat(msg_to_send.content, "; ");
      }
    }
    strcat(msg_to_send.content, "\n");
  }

  msg_to_send.client_id = msg->client_id;
  strcpy(msg_to_send.topic, "");
}

void subscribe(struct BlogOperation *msg) {
  struct Topic *topic_to_subscribe = searchTopic(msg);
  if (topic_to_subscribe == NULL) {
    topic_to_subscribe = createTopic(msg);
  }
  topic_to_subscribe->subscribers[msg->client_id] = 1;

  //printf("CHECKPOINT SUBSCRIBE\n");
  msg_to_send.client_id = msg->client_id;
  strcpy(msg_to_send.topic, msg->topic);
  strcpy(msg_to_send.content, "");
  printf("client %02i subscribed to %s\n", msg_to_send.client_id, msg_to_send.topic);
}

void disconnect_from_server(struct BlogOperation *msg){
  msg_to_send.client_id = msg->client_id;
  strcpy(msg_to_send.topic, "");
  strcpy(msg_to_send.content, "");
  printf("client %02i was disconnected\n", msg_to_send.client_id);
}

void unsubscribe(struct BlogOperation *msg) {
    struct Topic *topic_to_unsubscribe = searchTopic(msg);
    if (topic_to_unsubscribe != NULL) {
        topic_to_unsubscribe->subscribers[msg->client_id] = 0;
        printf("client %02i unsubscribed to %s\n", msg->client_id, msg->topic);
    }

    msg_to_send.client_id = msg->client_id;
    strcpy(msg_to_send.topic, "");
    strcpy(msg_to_send.content, "");
}

void selectCommand(struct BlogOperation *msg, int socket){

  msg_to_send.operation_type = msg->operation_type;
  msg_to_send.server_response = 1;
  //msg_to_send.server_response = 1;
  //printf("OP Type %i\n", msg->operation_type);
  switch (msg->operation_type) {
    case 2:
        publish(msg);
        break;
    
    case 3:
        listTopics(msg);
        break;
    
    case 4:
        // OK!!!!!!!!!!!!!
        subscribe(msg);
        break;
    
    case 5:
        disconnect_from_server(msg);
        break;
    
    case 6:
        unsubscribe(msg);
        break;
  }
}

//====================================// FUNCOES DE AUXILIO //====================================//



void dischargeID(int id) {
  if(id > 0 && id < 11) {
    sockets[id] = 0;
  }
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