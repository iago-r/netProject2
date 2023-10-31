#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void printMsg(struct BlogOperation *msg);

int main(void) {

  struct BlogOperation msg;
  commandParse(&msg);
  printMsg(&msg);
  
  return 0;
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

      if (valid_command == 2) {
        strcpy(msg->topic, arg_container);
        fillContent(valid_command, msg);
      }
      else if (valid_command == 3 || valid_command == 5) {
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

//printf("Checkpoint\n");

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