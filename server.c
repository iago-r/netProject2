#include "common.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

struct ClientData
{
  int client_socket;
};

struct BlogOperation
{
  int client_id;
  int operation_type;
  int server_response;
  char topic[50];
  char content[2048];
};

struct Topic
{
  char topic_name[50];
  int subscribers[11]; // 0 in a position = not sub; 1 = sub; [TODO]: change variable name
  char last_content_published[2048];
  int id_of_the_content_creator;
  struct Topic *next;
};

struct TopicList
{
  struct Topic *head; // = NULL;
  struct Topic *tail; // = NULL;
};

struct TopicList TOPICS;
int SOCKETS[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void usage(int argc, char **argv);
void *client_thread(void *data);

void initializeListOfTopics();
struct Topic *searchTopic(struct BlogOperation *msg);
struct Topic *createTopic(struct BlogOperation *msg);

int connectToServer(struct BlogOperation msg, int socket);
struct Topic *publish(struct BlogOperation *msg);
void listTopics(struct BlogOperation *msg);
void subscribe(struct BlogOperation *msg);
void disconnect_from_server(struct BlogOperation *msg);
void unsubscribe(struct BlogOperation *msg);
void selectCommand(struct BlogOperation *msg, struct Topic **topic_to_publish);
void broadcastMsg(struct BlogOperation msg, struct Topic **topic_to_broadcast);

void printTopicAndCustomStatus(struct Topic **topic,const char *msg); // [DELETE]
void printMsgAndCustomStatus(struct BlogOperation msg_package, const char *msg); // [DELETE]

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    usage(argc, argv);
  }

  initializeListOfTopics(TOPICS);

  struct sockaddr_storage storage;
  if (server_sockaddr_init(argv[1], argv[2], &storage) != 0)
  {
    usage(argc, argv);
  }

  int s;
  s = socket(storage.ss_family, SOCK_STREAM, 0);
  if (s == -1)
  {
    logexit("socket");
  }

  int enable = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != 0)
  {
    logexit("setsockopt");
  }

  struct sockaddr *addr = (struct sockaddr *)(&storage);
  if (bind(s, addr, sizeof(storage)) != 0)
  {
    logexit("bind");
  }

  if (listen(s, 10) != 0) // quantidade de conexões que podem estar pendentes para tratamento
  { 
    logexit("listen");
  }

  while (1)
  {
    struct sockaddr_storage cstorage;
    struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
    socklen_t caddrlen = sizeof(cstorage);

    int client_socket = accept(s, caddr, &caddrlen);
    if (client_socket == -1)
    {
      logexit("accept");
    }

    struct ClientData *cdata = malloc(sizeof(*cdata));
    if (!cdata)
    {
      logexit("malloc");
    }
    cdata->client_socket = client_socket;
    //printf("\n//[SERVER] ..............................\n"); printf("Socket: %i;\n", cdata->client_socket); // [DELETE]
  
    pthread_t tid;
    pthread_create(&tid, NULL, client_thread, cdata);
  }
  exit(EXIT_SUCCESS);
}

void usage(int argc, char **argv)
{
  printf("usage: %s <v4|v6> <server port>\n", argv[0]);
  printf("example: %s v4 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

void *client_thread(void *data)
{
  struct ClientData *cdata = (struct ClientData *)data;
  struct BlogOperation msg;
  struct Topic *topic_container;

  int client_id = connectToServer(msg, cdata->client_socket);
  printf("client %02i connected\n", client_id);
  //printf("\n//[SERVER][CLIENT_THREAD] ...............\n"); printf("Socket[id: %i]: %i;\n", client_id, cdata->client_socket); // [DELETE]
  while (1)
  {
    // RECEIVING PACKAGE.............................................
    bzero(&msg, sizeof(msg));
    recv(cdata->client_socket, &msg, sizeof(msg), 0);
    printMsgAndCustomStatus(msg, "[SERVER] RECEIVE"); // [DELETE]
    
    // SENDING PACKAGE...............................................
    selectCommand(&msg, &topic_container);
    broadcastMsg(msg, &topic_container); 

    if (5 == msg.operation_type || 0 == msg.client_id)
    {
      break;
    }
  }
  SOCKETS[client_id] = 0;
  close(cdata->client_socket);
  pthread_exit(EXIT_SUCCESS);
}

void initializeListOfTopics()
{
  TOPICS.head = NULL;
  TOPICS.tail = NULL;
}

struct Topic *searchTopic(struct BlogOperation *msg)
{
  struct Topic *cursor = TOPICS.head;
  while (cursor != NULL)
  {
    if (strcmp(cursor->topic_name, msg->topic) == 0)
    {
      return cursor;
    }
    cursor = cursor->next;
  }
  return cursor;
}

struct Topic *createTopic(struct BlogOperation *msg)
{
  struct Topic *cursor = malloc(sizeof(struct Topic));
  strcpy(cursor->topic_name, msg->topic);
  cursor->next = NULL;
  for (int i = 0; i < 11; i++)
  {
    cursor->subscribers[i] = 0;
  }

  if (TOPICS.head == NULL)
  {
    TOPICS.head = cursor;
    TOPICS.tail = cursor;
  }
  else
  {
    TOPICS.tail->next = cursor;
    TOPICS.tail = cursor;
  }
  return cursor;
}

int connectToServer(struct BlogOperation msg, int socket)
{
  int id;
  for (id = 1; id < 11; id++)
  {
    if (SOCKETS[id] == 0)
    {
      break;
    }
  }

  if (id > 0 && id < 11)
  {
    do
    {
      bzero(&msg, sizeof(msg));
      recv(socket, &msg, sizeof(msg), 0);

      if (msg.operation_type == 1 && msg.server_response == 0)
      {
        bzero(&msg, sizeof(msg));
        SOCKETS[id] = socket;
        //printf("\n//[SERVER][CONNECT_TO_SERVER] ...........\n"); printf("SOCKETS[%i]: %i;\n", id, SOCKETS[id]); // [DELETE]
        msg.client_id = id;
        msg.operation_type = 1;
        msg.server_response = 1;
        strcpy(msg.topic, "");
        strcpy(msg.content, "");
        send(socket, &msg, sizeof(msg), 0);
        return id;
      }
    } while (msg.operation_type != 1 && msg.server_response != 0);
  }
  return -1;
}

struct Topic *publish(struct BlogOperation *msg)
{
  struct Topic *topic_to_publish = searchTopic(msg);
  if (topic_to_publish == NULL)
  {
    topic_to_publish = createTopic(msg);
  }
  topic_to_publish->id_of_the_content_creator = msg->client_id;
  strcpy(topic_to_publish->last_content_published, msg->content);
  //printf("\n//[SERVER][PUBLISH] PUBLISH_MSG..........\n"); printTopicAndCustomStatus(topic_to_publish);
  printf("new post added in %s by %02i\n", msg->topic, msg->client_id);
  return topic_to_publish;
}

void listTopics(struct BlogOperation *msg)
{
  bzero(&msg->content, sizeof(msg->content));
  struct Topic *cursor = TOPICS.head;
  if (cursor == NULL)
  {
    strcpy(msg->content, "no topics available\n");
  }
  else
  {
    while (cursor != NULL)
    {
      strcat(msg->content, cursor->topic_name);
      cursor = cursor->next;
      if (cursor != NULL)
        strcat(msg->content, "; ");
    }
    strcat(msg->content, "\n");
  }
}

void subscribe(struct BlogOperation *msg)
{
  struct Topic *topic_to_subscribe = searchTopic(msg);
  if (topic_to_subscribe == NULL)
  {
    topic_to_subscribe = createTopic(msg);
  }
  if (topic_to_subscribe->subscribers[msg->client_id] == 1)
  {
    strcpy(msg->content, "error: already subscribed");
  }
  else
  {
    topic_to_subscribe->subscribers[msg->client_id] = 1;
    printf("client %02i subscribed to %s\n", msg->client_id, msg->topic);
  }
}

void disconnect_from_server(struct BlogOperation *msg)
{
  if (msg->client_id > 0 && msg->client_id < 11)
  {
    //SOCKETS[msg->client_id] = 0;
    struct Topic *cursor = TOPICS.head;
    while (cursor != NULL)
    {
      cursor->subscribers[msg->client_id] = 0;
      cursor = cursor->next;
    }
    printf("client %02i was disconnected\n", msg->client_id);
  }
}

void unsubscribe(struct BlogOperation *msg)
{
  struct Topic *topic_to_unsubscribe = searchTopic(msg);
  if (topic_to_unsubscribe != NULL)
  {
    topic_to_unsubscribe->subscribers[msg->client_id] = 0;
    printf("client %02i unsubscribed to %s\n", msg->client_id, msg->topic);
  }
}

void selectCommand(struct BlogOperation *msg, struct Topic **topic_to_publish)
{
  msg->server_response = 1;
  switch (msg->operation_type)
  {
  case 2:
    *topic_to_publish = publish(msg);
    printTopicAndCustomStatus(topic_to_publish, "[SERVER][SELECT_COMMAND] PUBLISH MSG");
    break;

  case 3:
    listTopics(msg);
    break;

  case 4:
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

void broadcastMsg(struct BlogOperation msg, struct Topic **topic_to_broadcast)
{
  if (2 == msg.operation_type)
  {
    printMsgAndCustomStatus(msg, "[SERVER] [BROADCAST]"); // [DELETE]
    for (int i = 1; i < 11; i++)
    {
      if ((*topic_to_broadcast)->subscribers[i] == 1 && msg.client_id != i)
      {
        send(SOCKETS[i], &msg, sizeof(msg), 0);
      }
    }
  }
  else
  {
    printMsgAndCustomStatus(msg, "[SERVER] [UNICAST]"); // [DELETE]
    send(SOCKETS[msg.client_id], &msg, sizeof(msg), 0);
    if (msg.operation_type == 5)
    {
      SOCKETS[msg.client_id] = 0;
    }
  }
}

void printTopicAndCustomStatus(struct Topic **topic,const char *msg)
{
  int num_of_chars = strlen(msg);
  if (num_of_chars <= 0)
  {
    printf("\n// [PRINT] .........................................\n");
  }
  else
  {
    printf("\n// %s ", msg);
    for (int i = 0; i < 48 - num_of_chars; i++)
      printf(".");
    printf("\n");
  }
  printf("Client -> %i\n", (*topic)->id_of_the_content_creator);
  printf("Topic -> %s\n", (*topic)->topic_name);
  printf("Content -> %s", (*topic)->last_content_published);
  printf("Subscribers -> ");
  for (int i = 1; i < 11; i++)
  {
    printf("[%02i]", (*topic)->subscribers[i]);
    if (i != 10)
    {
      printf(" ");
    }
  }
  printf("\n");
}

void printMsgAndCustomStatus(struct BlogOperation msg_package, const char *msg) // [DELETE]
{
  int num_of_chars = strlen(msg);
  if (num_of_chars <= 0)
  {
    printf("\n// [PRINT] .........................................\n");
  }
  else
  {
    printf("\n// %s ", msg);
    for (int i = 0; i < 48 - num_of_chars; i++)
      printf(".");
    printf("\n");
  }
  printf("client_id: %i\n", msg_package.client_id);
  printf("operation_type: %i\n", msg_package.operation_type);
  printf("server_response: %i\n", msg_package.server_response);
  printf("msg->topic: %s\n", msg_package.topic);
  printf("msg->content: %s\n", msg_package.content);
  printf("\n");
}