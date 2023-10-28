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

int countNumberOfWords(char *line);
int extractCmd(char *cmd_line, char *arg_container);
void commandParse(struct BlogOperation *msg);
void fillContent(int command_type, struct BlogOperation *msg);
void actionResultParse(struct BlogOperation *msg);
void printMsg(struct BlogOperation *msg); // DELETAR

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

    //char buffer[BUFSZ];
    struct BlogOperation msg;
    CLIENT_ID = (s, &msg);
    while (1) {
        //SENDING PACKAGE...............................................
        //bzero(buffer, sizeof(buffer));
        //printf("> "); fgets(buffer, BUFSZ-1, stdin);
        //send(s, buffer, sizeof(buffer), 0);
        bzero(msg, sizeof(msg));
        commandParse(&msg);
        send(s, &msg, sizeof(msg), 0);

        //RECEIVING PACKAGE.............................................       
        //bzero(buffer, sizeof(buffer));
        //recv(s, buffer, sizeof(buffer), 0);
        //printf("%s", buffer);
        bzero(msg, sizeof(msg));
        recv(s, &msg, sizeof(msg), 0);
        //ACTION PARSE -> CREATE!

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

int getID(int socket, struct BlogOperation *msg) {
    do {
        bzero(msg, sizeof(msg));
        msg->client_id = CLIENT_ID;
        msg->operation_type = 1;
        msg->server_response = 0;
        strcpy(msg->topic, "");
        strcpy(msg->content, "");
        send(socket, &msg, sizeof(msg), 0);

        bzero(msg, sizeof(msg));
        recv(socket, &msg, sizeof(msg), 0);
    } while (msg->operation_type != 1 && msg->server_response != 1);
    
    return msg->client_id;
}

//==========================================// FUNCOES //==========================================//

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
  //printf("number of words: %i\n", number_of_words);
  return number_of_words;
}

int extractCmd(char *cmd_line, char *arg_container) {
  
  int number_of_words = countNumberOfWords(cmd_line);
  char cmd1[64], cmd2[64], arg[1024];

  if (number_of_words == 1)
    sscanf(cmd_line, "%s", cmd1);
  else if (number_of_words == 2)
    sscanf(cmd_line, "%s %s", cmd1, arg);
  else if (number_of_words == 3) 
    sscanf(cmd_line, "%s %s %s", cmd1, cmd2, arg);
  else
    return -1;
  
  char actionTypes[7][12] = {"", "", "publish", "list",
                              "subscribe", "exit", "unsubscribe"};
 
  for (int i = 0; i < 7; i++) {
    if (strcmp(cmd1, actionTypes[i]) == 0) {
      if (i == 0 || i == 1) {
        break;
        //return -1;
      }
      else if (i == 2){
        if (number_of_words == 2 ||
            (number_of_words == 3 && (strcmp(cmd2, "in") == 0))) {
          strcpy(arg_container, arg);
          return i;
        }
      }
      else if ((i == 4 || i == 6) && number_of_words == 2) {
        strcpy(arg_container, arg);
        return i;
      }
      else if ((i == 3 && number_of_words == 2 && (strcmp(arg, "topics") == 0)) ||
              (i == 5 && number_of_words == 1)) {
        return i;
      }
    }
  }
  return -1;
}

void fillContent(int command_type, struct BlogOperation *msg) {
  char content_line[2048];

  if (command_type == 2) {
    printf("> "); fgets(content_line, sizeof(content_line), stdin);
    strcpy(msg->content, content_line);
    bzero(content_line, sizeof(content_line));
  }
  else if (command_type == 3 || command_type == 5){
    strcpy(msg->content, "");
    bzero(content_line, sizeof(content_line));
  }   
}

void commandParse(struct BlogOperation *msg){

  msg->client_id = CLIENT_ID;
  msg->server_response = 0;
  
  char cmd_line[2048], arg_container[1024];
  int valid_command;

  do {
    printf("> "); fgets(cmd_line, sizeof(cmd_line), stdin);
    valid_command = extractCmd(cmd_line, arg_container);
    if (valid_command != -1) {
      msg->operation_type = valid_command;

      if (valid_command == 2 || valid_command == 3 || valid_command == 5) {
        strcpy(msg->topic, "");
        fillContent(valid_command, msg);
      }
      else if (valid_command == 2 || valid_command == 3) {
        strcpy(msg->topic, arg_container);
        strcpy(msg->content, "");
      }
    }
  } while (valid_command == -1);
}

void actionResultParse(struct BlogOperation *msg) {
    switch (msg->operation_type) {
        // SERVER.........................publish
        case 2:
            break;
        
        // SERVER.........................list topics
        case 3:
            break;

        // SERVER.........................subscribe
        case 4:
            break;
        
        // SERVER.........................exit
        case 5:
            break;
        
        // SERVER.........................unsubscribe
        case 6:
            break;        
    }

    //printf("new post added in <topic> by <client_id>\n", topics, client);
}

//====================================// FUNCOES DE AUXILIO //====================================//

void printMsg(struct BlogOperation *msg) {
  printf("\n");
  printf("client_id: %i\n", msg->client_id);
  printf("server_response: %i\n", msg->server_response);
  printf("operation_type: %i\n", msg->operation_type);
  printf("msg->topic: %s\n", msg->topic);
  printf("msg->content: %s\n", msg->content);
  printf("\n");
}

//printf("Checkpoint\n");