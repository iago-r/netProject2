#include "common.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSZ 1024

struct client_data
{
  int csock, id;
  struct sockaddr_storage storage;
};

struct BlogOperation
{
  int client_id;
  int operation_type;
  int server_response;
  char topic[50];
  char content[2048];
};

void usage(int argc, char **argv);
void *client_thread(void *data);
int getID(int socket, struct BlogOperation msg);
int extractCmd(char *cmd_line, char *arg_container);
void commandParse(struct BlogOperation *msg, int client_id);
void resultParse(struct BlogOperation *msg_received, int client_id);
void printMsgAndCustomStatus(struct BlogOperation msg_package, const char *msg); // [DELETE]

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    usage(argc, argv);
  }

  struct sockaddr_storage storage;
  if (0 != addrparse(argv[1], argv[2], &storage))
  {
    usage(argc, argv);
  }

  int s;
  s = socket(storage.ss_family, SOCK_STREAM, 0);
  if (s == -1)
  {
    logexit("socket");
  }

  struct sockaddr *addr = (struct sockaddr *)(&storage);
  if (0 != connect(s, addr, sizeof(storage)))
  {
    logexit("connect");
  }

  struct client_data *cdata = malloc(sizeof(*cdata));
  if (!cdata)
  {
    logexit("malloc");
  }

  cdata->csock = s;
  memcpy(&(cdata->storage), &storage, sizeof(storage));

  struct BlogOperation msg;
  int client_id = getID(s, msg);
  cdata->id = client_id;

  pthread_t tid;
  pthread_create(&tid, NULL, client_thread, cdata);

  while (1)
  {
    // RECEIVING PACKAGE.............................................
    bzero(&msg, sizeof(msg));
    recv(s, &msg, sizeof(msg), 0);
    printMsgAndCustomStatus(msg, "[CLIENT] RECEIVE"); // [DELETE]
    resultParse(&msg, client_id);
    //printMsgAndCustomStatus(msg, "[CLIENT][RESULT_PARSE]"); // [DELETE]

    // EXIT.........................................................
    if (msg.operation_type == 5)
    {
      break;
    }
  }
  close(s);
  exit(EXIT_SUCCESS);
}

void usage(int argc, char **argv)
{
  printf("usage %s <server IP> <server port>\n", argv[0]);
  printf("example: %s 127.0.0.1 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

int getID(int socket, struct BlogOperation msg)
{
  do
  {
    // REQUEST ID...............................................
    bzero(&msg, sizeof(msg));
    msg.client_id = 0;
    msg.operation_type = 1;
    msg.server_response = 0;
    strcpy(msg.topic, "");
    strcpy(msg.content, "");
    send(socket, &msg, sizeof(msg), 0);
    //printMsgAndCustomStatus(msg, "[getID][client] SEND"); // [DELETE]

    // GET ID...................................................
    bzero(&msg, sizeof(msg));
    recv(socket, &msg, sizeof(msg), 0);
    //printMsgAndCustomStatus(msg, "[getID][client] RECEIVE"); // [DELETE]
  } while (msg.operation_type != 1 && msg.server_response != 1);
  return msg.client_id;
}

void *client_thread(void *data)
{
  struct client_data *cdata = (struct client_data *)data;
  struct BlogOperation msg;
  while (1)
  {
    // SENDING PACKAGE..............................................
    bzero(&msg, sizeof(msg));
    commandParse(&msg, cdata->id);
    send(cdata->csock, &msg, sizeof(msg), 0);
    //printMsgAndCustomStatus(msg, "[CLIENT][CLIENT_THREAD] SEND"); // [DELETE]

    // EXIT.........................................................
    if (msg.operation_type == 5)
    {
      break;
    }
  }
  close(cdata->csock);
  pthread_exit(EXIT_SUCCESS);
}

int extractCmd(char *cmd_line, char *arg_container)
{
  char cmd1[64], cmd2[64], cmd3[1024];
  int number_of_words = sscanf(cmd_line, "%s %s %s", cmd1, cmd2, cmd3);

  if (-1 == number_of_words || 3 < number_of_words)
  {
    return -1;
  }

  char actionTypes[7][12] = {"", "", "publish", "list", "subscribe", "exit", "unsubscribe"};
  for (int i = 2; i < 7; i++)
  {
    if (strcmp(cmd1, actionTypes[i]) == 0)
    {
      if (i == 2 &&
          (number_of_words == 2 || (number_of_words == 3 && (strcmp(cmd2, "in") == 0))))
      {
        if (number_of_words == 2)
        {
          strcpy(arg_container, cmd2);
        }
        else if (number_of_words == 3)
        {
          strcpy(arg_container, cmd3);
        }
        bzero(cmd_line, sizeof(&cmd_line));
        return i;
      }
      else if ((i == 4 || i == 6) && number_of_words == 2)
      {
        strcpy(arg_container, cmd2);
        bzero(cmd_line, sizeof(&cmd_line));
        return i;
      }
      else if ((i == 3 && number_of_words == 2 && (strcmp(cmd2, "topics") == 0)) ||
               (i == 5 && number_of_words == 1))
      {
        bzero(cmd_line, sizeof(&cmd_line));
        return i;
      }
    }
  }
  return -1;
}

void commandParse(struct BlogOperation *msg, int client_id)
{
  msg->client_id = client_id;
  msg->server_response = 0;

  char cmd_line[2048], arg_container[1024];
  int valid_command;
  do
  {
    fgets(cmd_line, sizeof(cmd_line) - 2, stdin);
    valid_command = extractCmd(cmd_line, arg_container);
    if (valid_command != -1)
    {
      msg->operation_type = valid_command;
      if (valid_command == 2)
      {
        strcpy(msg->topic, arg_container);
        fgets(cmd_line, sizeof(cmd_line), stdin);
        strcpy(msg->content, cmd_line);
        bzero(cmd_line, sizeof(cmd_line));
      }
      else if (valid_command == 3 || valid_command == 5)
      {
        strcpy(msg->topic, "");
        strcpy(msg->content, "");
        bzero(cmd_line, sizeof(cmd_line));
      }
      else if (valid_command == 4 || valid_command == 6)
      {
        strcpy(msg->topic, arg_container);
        strcpy(msg->content, "");
      }
    }
  } while (valid_command == -1);
  //printMsgAndCustomStatus(*msg, "[CLIENT][COMMAND_PARSE]"); // [DELETE]
}

void resultParse(struct BlogOperation *msg_received, int client_id)
{
  do
  {
    if (1 == msg_received->server_response)
    {
      // SERVER.............................publish
      if (2 == msg_received->operation_type && client_id != msg_received->client_id)
      {
        printf("new post added in %s by %02i\n", msg_received->topic, msg_received->client_id);
        printf("%s", msg_received->content);
      }
      // SERVER.........................list topics
      else if (3 == msg_received->operation_type)
      {
        printf("%s", msg_received->content);
      }
      else if (4 == msg_received->operation_type && 0 != strlen(msg_received->content))
      {
        printf("%s", msg_received->content);
      }
    }
  } while (1 != msg_received->server_response);
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