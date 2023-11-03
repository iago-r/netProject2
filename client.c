#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSZ 1024

struct BlogOperation {
  int client_id;
  int operation_type;
  int server_response;
  char topic[50];
  char content[2048];
};

void usage(int argc, char** argv);

int getID(int socket, struct BlogOperation msg);
int countNumberOfWords(char *line);
int extractCmd(char *cmd_line, char *arg_container);
void commandParse(struct BlogOperation *msg, int client_id);
void resultParse(struct BlogOperation *msg_received, int client_id);
void printMsg(struct BlogOperation *msg); // DELETAR


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

  struct BlogOperation msg;
  int client_id = getID(s, msg);
  while (1) {
    //SENDING PACKAGE...............................................
    bzero(&msg, sizeof(msg));
    commandParse(&msg, client_id);
    send(s, &msg, sizeof(msg), 0);

    //RECEIVING PACKAGE.............................................       
    bzero(&msg, sizeof(msg));
    recv(s, &msg, sizeof(msg), 0);
    resultParse(&msg, client_id);

    // EXIT.........................................................
    if(msg.operation_type == 5) {
      break;
    }
  }
  close(s);
  exit(EXIT_SUCCESS);
}

void usage(int argc, char **argv) {
  printf("usage %s <server IP> <server port>\n", argv[0]);
  printf("example: %s 127.0.0.1 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

int getID(int socket, struct BlogOperation msg) {
  do {
    //REQUEST ID...............................................
    bzero(&msg, sizeof(msg));
    msg.client_id = 0;
    msg.operation_type = 1;
    msg.server_response = 0;
    strcpy(msg.topic, "");
    strcpy(msg.content, "");
    send(socket, &msg, sizeof(msg), 0);

    //GET ID...................................................
    bzero(&msg, sizeof(msg));
    recv(socket, &msg, sizeof(msg), 0);
  } while (msg.operation_type != 1 && msg.server_response != 1);
  return msg.client_id;
}

int countNumberOfWords(char *line) {
  char buffer[2024];
  int number_of_words = 0;
  char *token, *saveptr1;
  strcpy(buffer, line);
  
  token = strtok_r(buffer, " ", &saveptr1);
  if (NULL == token){
    return -1;
  }

  while (token != NULL) {
    token = strtok_r(NULL, " ", &saveptr1);
    number_of_words++;
  }
  return number_of_words;
}

int extractCmd(char *cmd_line, char *arg_container) {
  int number_of_words = countNumberOfWords(cmd_line);
  if (-1 == number_of_words) {
    return -1;
  }
  
  char cmd1[64], cmd2[64], cmd3[1024];
  if (number_of_words == 1)
    sscanf(cmd_line, "%s", cmd1);
  else if (number_of_words == 2)
    sscanf(cmd_line, "%s %s", cmd1, cmd3);
  else if (number_of_words == 3) 
    sscanf(cmd_line, "%s %s %s", cmd1, cmd2, cmd3);
  else
    return -1;
  
  char actionTypes[7][12] = {"", "", "publish", "list",
                              "subscribe", "exit", "unsubscribe"};
  for (int i = 2; i < 7; i++) {
    if (strcmp(cmd1, actionTypes[i]) == 0) {
      if (i == 2 &&
        (number_of_words == 2 || (number_of_words == 3 && (strcmp(cmd2, "in") == 0)))) {
          strcpy(arg_container, cmd3);
          bzero(cmd_line, sizeof(&cmd_line));
          return i;
      }
      else if ((i == 4 || i == 6) && number_of_words == 2) {
        strcpy(arg_container, cmd3);
        bzero(cmd_line, sizeof(&cmd_line));
        return i;
      }
      else if ((i == 3 && number_of_words == 2 && (strcmp(cmd3, "topics") == 0)) ||
        (i == 5 && number_of_words == 1)) {
        bzero(cmd_line, sizeof(&cmd_line));
        return i;
      }
    }
  }
  return -1;
}

void commandParse(struct BlogOperation *msg, int client_id) {
  msg->client_id = client_id;
  msg->server_response = 0;
  
  char cmd_line[2048], arg_container[1024];
  int valid_command;

  do {
    printf("> "); fgets(cmd_line, sizeof(cmd_line) - 2, stdin);
    valid_command = extractCmd(cmd_line, arg_container);
    if (valid_command != -1) {
      msg->operation_type = valid_command;
      if (valid_command == 2) {
        strcpy(msg->topic, arg_container);
        printf("> "); fgets(cmd_line, sizeof(cmd_line), stdin);
        strcpy(msg->content, cmd_line);
        bzero(cmd_line, sizeof(cmd_line));
      }
      else if (valid_command == 3 || valid_command == 5) {
        strcpy(msg->topic, "");
        strcpy(msg->content, "");
        bzero(cmd_line, sizeof(cmd_line));
      }
      else if (valid_command == 4 || valid_command == 6) {
        strcpy(msg->topic, arg_container);
        strcpy(msg->content, "");
      }
    }
  } while (valid_command == -1);
  printf("\n//SENDING PACKAGE........................\n"); printMsg(msg);
}

//void resultParse(struct BlogOperation *msg) {
void resultParse(struct BlogOperation *msg_received, int client_id)
{  
  do
  {
    if (1 == msg_received->server_response)// && (msg_sended->operation_type == msg_received->operation_type))
    {
      switch (msg_received->operation_type)
      {
        // SERVER.............................publish
        case 2:
          if (client_id != msg_received->client_id)
            printf("new post added in %s by %02i\n%s", msg_received->topic, msg_received->client_id, msg_received->topic);                   
          break;
      
        // SERVER.........................list topics
        case 3:
          break;

        // SERVER...........................subscribe
        case 4:
          break;
        
        // SERVER................................exit
        case 5:
          break;
        
        // SERVER.........................unsubscribe
        case 6:
          break;

        default:
          break;
      }
    }
  } while (1 != msg_received->server_response);













    //printf("new post added in <topic> by <client_id>\n", topics, client);
}

//====================================// FUNCOES DE AUXILIO //====================================//

void printMsg(struct BlogOperation *msg) {
  //printf("\n");
  printf("client_id: %i\n", msg->client_id);
  printf("operation_type: %i\n", msg->operation_type);
  printf("server_response: %i\n", msg->server_response);
  printf("msg->topic: %s\n", msg->topic);
  printf("msg->content: %s\n", msg->content);
  printf("\n");
}

//printf("Checkpoint\n");

//[DELETE] printf("\n//GETTING ID..............server response\n"); printMsg(&msg);
//printf("number of words: %i\n", number_of_words);