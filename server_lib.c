#include "server_lib.h"
#include <stdio.h>

void initializeIdList() {
  for (int i = 0; i < 1024; i++) {
    ID_LIST[i] = 0;
  }  
}

int searchValidId() {
  int i;
  for (i = 0; i < ID_COUNTER; i++) {
    if (ID_LIST[i] == 0) {
      break;
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
    ID_LIST[id] = 1;
  }
  else if (id < ID_COUNTER - 1){
    ID_LIST[id] = 1;
  }
  return id;
}

// Call this function in the exit command to liberate a ID
void dischargeId(id) {
  ID_LIST[id] = 0;
}