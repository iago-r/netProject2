if (number_of_words == 2) {
    sscanf(cmd_line, "%s %s", cmd1, arg);
    printf("2 words: %s %s\n", cmd1, arg);
  }
  else if (number_of_words == 3) {
    sscanf(cmd_line, "%s %s %s", cmd1, cmd2, arg);
    printf("3 words: %s %s %s\n", cmd1, cmd2, arg);
  }
  





































int extractCmd(char *cmd_container, char *arg_container, char* cmd_line) {
  
  int number_of_words = countNumberOfWords(cmd_line);
  char *dump_token, *saveptr1;

  if (number_of_words == 2) {
    cmd_container = strtok_r(cmd_line, " ", &saveptr1);
    dump_token = strtok_r(NULL, " ", &saveptr1);
    //printf("[Number of words]: %i -> <cmd>: %s <arg>: %s", number_of_words, cmd_container, dump_token);
  }
  else if (number_of_words == 3) {
    cmd_container = strtok_r(cmd_line, " ", &saveptr1);
    dump_token = strtok_r(NULL, " ", &saveptr1);
    arg_container = strtok_r(NULL, " ", &saveptr1);
    //printf("[Number of words]: %i -> <cmd>: %s %s <arg>: %s", number_of_words, cmd_container, dump_token, arg_container);
  }

  if (number_of_words == 2 || number_of_words == 3)
    return number_of_words;
  else
    return -1;
}

int detectType(char *cmd_container, char *arg_container) {
  
  char actionTypes[7][12] = {"", "", "publish in", "list topics",
                              "subscribe", "exit", "unsubscribe"};

  for (int i = 0; i < 9; i++) {
      if (strcmp(cmd_container, actionTypes[i]) == 0) {
          return i;
      }
  }
  return -1;
}


/* #include <stdio.h>
#include <stdlib.h>
#include <string.h>

int countNumberOfWords(char *line);

int main(void) {

  char cmd[2048];
  printf("> "); fgets(cmd, sizeof(cmd), stdin);
  printf("[string]: %s", cmd);

  printf("Number of words: %i\n", countNumberOfWords(cmd));

  printf("[string]: %s", cmd);
    //printf("%s", token);

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
  return number_of_words;
} */





// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// int main(void) {

//   char cmd[2048];
//   printf("> "); fgets(cmd, sizeof(cmd), stdin);
//   printf("[string]: %s", cmd);
  
//   for (int i = 0; cmd[i] != '\0'; i++) {
//     /* code */
//   }
  
//   char *token, *saveptr1;
//   token = strtok_r(cmd, " ", &saveptr1);

//   while (token != NULL)
//     token = strtok_r(NULL, " ", &saveptr1);
//     //printf("%s", token);

//   return 0;
// }




/* #include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {

    char cmd[2048];
    char actionTypes[7][12] = {"", "", "publish", "list", "subscribe", "exit", "unsubscribe"};

    while (1) {
      
      printf("> "); scanf("%s", cmd);
      
      int i;
      for (i = 0; i < 7; i++) {
          if (strcmp(cmd, actionTypes[i]) == 0) {
            if (i == 0 || i == 1) {               // invalid cmd
              break;
            }
            else if (i == 2) {
              memset(cmd,0,strlen(cmd));
              scanf("%s", cmd);
              if (strcmp(cmd, "in") == 0) {
                printf("Detected: %s\n", actionTypes[i]); break;
              }
              else {
                printf("Command not found\n");
              }
            }
            else if (i == 3) {

              }
            }
            
              printf("Detected: %s\n", actionTypes[i]); break;
              //return i;
          }
      }
      if (i == 7) {
        printf("Command not found\n");
      } 
      
      memset(cmd,0,strlen(cmd));
    }
    return 0;
    //return -1;
}

 */




// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// int
// main(void)
// {
//     char actionTypes[7][12] = {"", "", "publish", "list", "subscribe", "exit", "unsubscribe"};
//     char cmd[2048], *token, *saveptr1;
// /*     char actionTypes[7][12] = {"", "", "publish in", "list topics",
//                                 "subscribe", "exit", "unsubscribe"}; */
    
//     printf("> "); fgets(cmd, sizeof(cmd), stdin);

//     token = strtok_r(cmd, " ", &saveptr1);
//     while (token != NULL) {
//         printf("%s", token);




//       for (int i = 0; i < 9; i++) {
//         if (strcmp(token, actionTypes[i]) == 0) {
//             return i;
//         }
//       }




//         token = strtok_r(NULL, " ", &saveptr1);
//     }
//     exit(EXIT_SUCCESS);
// }






// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// int
// main(void)
// {
//     char actionTypes[7][12] = {"", "", "publish in", "list topics",
//                                 "subscribe", "exit", "unsubscribe"};
    
//     //char *cmd, *token;
//     char cmd[2048];
//     //char cmd[2048], *token;
//     char *saveptr1;

//     printf("> "); fgets(cmd, sizeof(cmd), stdin);

//     char* token = strtok(cmd, " ");
 
//     // Keep printing tokens while one of the
//     // delimiters present in str[].
//     while (token != NULL) {
//         printf(" %s\n", token);
//         token = strtok(NULL, " - ");
//     }
// /*     for ( ; ; *cmd = '\0') {
//     //while (cmd != NULL) {
//         token = strtok(cmd, " ");
//         //token = strtok_r(cmd, ' ', &saveptr1);
//         if (token == NULL){
//             printf("Checkpoint\n");
//             break;
//         }
//         printf("%s\n", token);
//     } */
//     exit(EXIT_SUCCESS);
// }


/* #include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char *argv[])
{
    char *str1, *token;
    char *saveptr1, *saveptr2;
    int j;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s 'string' 'delim' \n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    for (j = 1, str1 = argv[1]; ; j++, str1 = NULL) {
        token = strtok_r(str1, argv[2], &saveptr1);
        if (token == NULL)
            break;
        printf("%d: %s\n", j, token);
    }

    exit(EXIT_SUCCESS);
} */





/* main(int argc, char *argv[])
{
    char *str1, *str2, *token, *subtoken;
    char *saveptr1, *saveptr2;
    int j;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s string delim subdelim\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    for (j = 1, str1 = argv[1]; ; j++, str1 = NULL) {
        token = strtok_r(str1, argv[2], &saveptr1);
        if (token == NULL)
            break;
        printf("%d: %s\n", j, token);

        for (str2 = token; ; str2 = NULL) {
            subtoken = strtok_r(str2, argv[3], &saveptr2);
            if (subtoken == NULL)
                break;
            printf(" --> %s\n", subtoken);
        }
    }

    exit(EXIT_SUCCESS);
} */

/* #include <stdio.h>
#include <string.h>

int main(void) {
  

  char buffer[2048];
  char actionTypes[7][16] = {"", "", "publish in", "list topics",
                            "subscribe\n", "exit", "unsubscribe"};
  
  int i;
  for (;;) {
    printf("> "); fgets(buffer, sizeof(buffer), stdin);//scanf("%s", buffer);

    strtok()

    printf("My string: %s\n", buffer);
    for (i = 0; i < 7; i++) {
        if (strcmp(buffer, actionTypes[i]) == 0) {
            printf("Detected: %s\n", actionTypes[i]); break;
        }
    }
    if (i == 7) {
      printf("Command not found\n");
    }  
  }
    
  return 0;
} */
/* 
enum op_type {New_connection = 1, New_post, List_topics, Subscribe_topic, Disconnect};

unsigned getbits(unsigned x, int p, int n)
{
    return (x >> (p+1-n)) & ~(~0 << n);
}

void bin(unsigned n)
{
    unsigned i;
    for (i = 1 << 31; i > 0; i = i / 2)
        (n & i) ? printf("1") : printf("0");
}

void setBit(unsigned number, int p) {
  number |= (1 << p);
  unsigned i;
  for (i = 1 << 31; i > 0; i = i / 2)
    (number & i) ? printf("1") : printf("0");
}


int main(void){
  //printf("%i\n", New_connection);
  bin(10);
  printf("\n%i\n", getbits(10,2,1));
  setBit(10, 0);
  printf("\n");

  return 0;
} */



/* 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char *argv[])
{
    char *str1, *token;
    char *saveptr1, *saveptr2;
    int j;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s 'string' 'delim' \n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    for (j = 1, str1 = argv[1]; ; j++, str1 = NULL) {
        token = strtok_r(str1, argv[2], &saveptr1);
        if (token == NULL)
            break;
        printf("%d: %s\n", j, token);
    }

    exit(EXIT_SUCCESS);
}
 */