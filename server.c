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
  int subscribers[11]; // not sub = 0; sub; 1
  struct Topic *next;
};

struct TopicList
{
  struct Topic *head;
  struct Topic *tail;
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
int disconnectfromserver(struct BlogOperation msg, int client_id);
void unsubscribe(struct BlogOperation *msg);

void selectCommand(struct BlogOperation *msg, struct Topic **topic_to_publish);
void broadcast(struct BlogOperation msg, struct Topic **topic_to_broadcast);

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

  while (1)
  {
    // RECEIVING PACKAGE.............................................
    bzero(&msg, sizeof(msg));
    recv(cdata->client_socket, &msg, sizeof(msg), 0);

    // SENDING PACKAGE...............................................
    selectCommand(&msg, &topic_container);
    broadcast(msg, &topic_container);

    if (disconnectfromserver(msg, client_id) == 1)
    {
      break;
    }
  }
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
      {
        strcat(msg->content, "; ");
      }
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
    strcpy(msg->content, "error: already subscribed\n");
  }
  else
  {
    topic_to_subscribe->subscribers[msg->client_id] = 1;
    printf("client %02i subscribed to %s\n", msg->client_id, msg->topic);
  }
}

int disconnectfromserver(struct BlogOperation msg, int client_id)
{
  int current_id;
  if (msg.client_id <= 0 || msg.client_id > 10)
  {
    current_id = client_id;
  }
  else if (msg.operation_type == 5)
  {
    current_id = msg.client_id;
  }
  else
  {
    return 0;
  }

  SOCKETS[current_id] = 0;
  struct Topic *cursor = TOPICS.head;
  while (cursor != NULL)
  {
    cursor->subscribers[current_id] = 0;
    cursor = cursor->next;
  }
  printf("client %02i was disconnected\n", current_id);
  return 1;
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
    break;

  case 3:
    listTopics(msg);
    break;

  case 4:
    subscribe(msg);
    break;

  case 6:
    unsubscribe(msg);
    break;
  }
}

void broadcast(struct BlogOperation msg, struct Topic **topic_to_broadcast)
{
  if (2 == msg.operation_type)
  {
    for (int i = 1; i < 11; i++)
    {
      if ((*topic_to_broadcast)->subscribers[i] == 1 /* && msg.client_id != i */)
      {
        send(SOCKETS[i], &msg, sizeof(msg), 0);
      }
    }
  }
  else
  {
    send(SOCKETS[msg.client_id], &msg, sizeof(msg), 0);
  }
}