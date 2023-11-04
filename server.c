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
int SUBSCRIBERS[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//[TODO]: DIMINUIR O NUMERO DE FUNCOES
void usage(int argc, char **argv);
void *client_thread(void *data);
void initializeListOfTopics();
struct Topic *searchTopic(struct BlogOperation *msg);
struct Topic *createTopic(struct BlogOperation *msg);
int connectToServer(struct BlogOperation msg, int socket);
void publish(struct BlogOperation *msg);
void listTopics(struct BlogOperation *msg);
void subscribe(struct BlogOperation *msg);
void disconnect_from_server(struct BlogOperation *msg);
void unsubscribe(struct BlogOperation *msg);
void selectCommand(struct BlogOperation *msg);
void broadcastMsg(struct BlogOperation *msg);

void printMsg(struct BlogOperation *msg);
void printMsg2(struct Topic *msg);

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

  if (listen(s, 10) != 0)
  { // quantidade de conexões que podem estar pendentes para tratamento
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
  //struct Topic topic_with_content_to_broadcast;

  int client_id = connectToServer(msg, cdata->client_socket);
  printf("client %02i connected\n", client_id);
  while (1)
  {
    // RECEIVING PACKAGE.............................................
    bzero(&msg, sizeof(msg));
    recv(cdata->client_socket, &msg, sizeof(msg), 0);

    // SENDING PACKAGE...............................................
    selectCommand(&msg);
    //printf("Checkpoint[RETURN FROM SELECT]: \n");printMsg2(&topic_with_content_to_broadcast);
    //broadcastMsg(&msg); 
    send(cdata->client_socket, &msg, sizeof(msg), 0);

    if (5 == msg.operation_type)
    {
      break;
    }
  }
  close(cdata->client_socket);
  pthread_exit(EXIT_SUCCESS);
}

// O servidor deve decidir se envia a resposta a um ou mais computadores,
// ou seja, o send do server tem que ser feito seletivamente
//======================================// PRONTO //======================================//

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
        printf("Socket[%i]: %i\n", id, SOCKETS[id]);
        msg.client_id = id;
        msg.operation_type = 1;
        strcpy(msg.topic, "");
        strcpy(msg.content, "");
        send(socket, &msg, sizeof(msg), 0);
        return id;
      }
    } while (msg.operation_type != 1 && msg.server_response != 0);
  }
  return -1;
}

//========================================================================================//

//struct Topic *publish(struct BlogOperation *msg)
void publish(struct BlogOperation *msg)
{
  struct Topic *topic_to_publish = searchTopic(msg);
  if (topic_to_publish == NULL)
  {
    topic_to_publish = createTopic(msg);
  }
  //topic_to_publish->id_of_the_content_creator = msg->client_id;
  //strcpy(topic_to_publish->last_content_published, msg->content);
  for (int i = 1; i < 11; i++)
  {
    SUBSCRIBERS[i] = topic_to_publish->subscribers[i];
  }
  printf("new post added in %s by %02i\n", msg->topic, msg->client_id);
  //return topic_to_publish;
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
    topic_to_subscribe = createTopic(msg);
  topic_to_subscribe->subscribers[msg->client_id] = 1;
  printf("client %02i subscribed to %s\n", msg->client_id, msg->topic);
}

void disconnect_from_server(struct BlogOperation *msg)
{
  if (msg->client_id > 0 && msg->client_id < 11)
  {
    SOCKETS[msg->client_id] = 0;
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

void selectCommand(struct BlogOperation *msg)
{
  msg->server_response = 1; // DONT NEED TO CHANGE THE OPERATION TYPE, BECAUSE ITS THE SAME
  switch (msg->operation_type)
  {
  case 2:
    publish(msg);
    //printf("Checkpoint[SELECT COMMAND]: \n");printMsg2(topic_with_content_to_broadcast);
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

void broadcastMsg(struct BlogOperation *msg) {
  if (2 == msg->operation_type)
  {
    //printf("Checkpoint[Broadcast]: \n");printMsg2(topic_with_content_to_broadcast);
    /* bzero(&msg->client_id, sizeof(msg->client_id));
    bzero(&msg->topic, sizeof(msg->topic));
    bzero(&msg->content, sizeof(msg->content));
    msg->client_id = topic_with_content_to_broadcast->id_of_the_content_creator;
    strcpy(msg->topic, topic_with_content_to_broadcast->topic_name);
    strcpy(msg->content, topic_with_content_to_broadcast->last_content_published); */
    //printf("X %i %s %s",  msg->client_id, msg->topic, msg->content);
    for (int i = 1; i < 11; i++)
    {
      printf("Sub[%i]<%i>\n", i, SUBSCRIBERS[i]);
/*       if (1 == SUBSCRIBERS[i] && i != msg->client_id)
      {
        send(SOCKETS[i], &msg, sizeof(msg), 0);
      }  */
    }
  }
  else
  {
    printf("Checkpoint [A], %i\n", msg->client_id);
    printf("BROADCAST -> Socket[%i]: %i\n", msg->client_id, SOCKETS[msg->client_id]);
    printMsg(msg);
    //printf("Socket[%i]: %i\n", id, SOCKETS[id]);
    
    send(SOCKETS[msg->client_id], &msg, sizeof(msg), 0);
  }
}




//====================================// FUNCOES DE AUXILIO //====================================//

void printMsg(struct BlogOperation *msg)
{
  printf("\n");
  printf("client_id: %i\n", msg->client_id);
  printf("server_response: %i\n", msg->server_response);
  printf("operation_type: %i\n", msg->operation_type);
  printf("msg->topic: %s\n", msg->topic);
  printf("msg->content: %s\n", msg->content);
  printf("\n");
}

void printMsg2(struct Topic *msg)
{
  printf("\n");
  printf("client_id: %i\n", msg->id_of_the_content_creator);
  printf("msg->topic: %s\n", msg->topic_name);
  printf("msg->content: %s\n", msg->last_content_published);
  printf("\n");
}