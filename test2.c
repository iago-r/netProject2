#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main(void) {

  initializeListOfTopics(Topics);
  struct BlogOperation msg_received;

  bzero(&msg_received, sizeof(msg_received));
  bzero(&msg_to_send, sizeof(msg_to_send));
  
  // MSG TEST..................................
  msg_received.client_id = 0;
  msg_received.operation_type = 2;
  msg_received.server_response = 0;
  strcpy(msg_received.topic, "");
  strcpy(msg_received.content, "");
  //...........................................

  commandParse(&msg_received);

  printMsg(&msg_received);
  //printMsg(&msg_to_send);
  
  return 0;
}

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