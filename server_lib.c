#include "server_lib.h"
#include <stdio.h>
/* 
int ID_COUNTER = 0;
char ID_LIST[1024];
 */
void initializeIdList() {
  for (int i = 0; i < 1024; i++) {
    ID_LIST[i] = 0;
  }  
}

int searchValidId() {
  int i;
  for (i = 0; i < ID_COUNTER; i++) {
    if (ID_LIST[i] == 0) {
      return i;
    }
  }
  return i;
}

int assignId() {
  int id = searchValidId();
  
  if (id == ID_COUNTER) {
    return -1;
  }
  else if (id == ID_COUNTER - 1) {
    ID_COUNTER++;
    ID_LIST[id]++;
  }
  else if (id < ID_COUNTER - 1){
    ID_LIST[id]++;
  }
}

// Call this function in the exit command to liberate a ID
/* int dischargeId() {

}
 */



























//==============================================================================================//
/* 
void initializeList(Connection* head, Connection* tail) {
  head = NULL;
  tail = NULL;
}

void insertNode(Connection* head, Connection* tail) {
  Connection* aux = malloc(sizeof(Connection));
  ID_COUNTER++;
  aux->id = ID_COUNTER;
  aux->next = NULL;
  
  if (head == NULL) {
    head = aux;
    tail = aux;
  }
  else {
    tail->next = aux;
    tail = aux;
  }
}

void display(Connection* head) {
  Connection* aux = head;
  printf("ID list: ");
  while (aux != NULL) {
    printf("%i ", aux->id);
    aux = aux->next;
  }
  printf("\n");
}
 */