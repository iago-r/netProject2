#ifndef SERVER_LIB_H
#define SERVER_LIB_H

int ID_COUNTER = 1;
char ID_LIST[1024];

void initializeIdList();
int searchValidId();
int assignId();
int dischargeId();

enum op_type {New_connection = 1, New_post, List_topics, Subscribe_topic, Disconnect};

struct BlogOperation {
    int client_id;
    int operation_type;
    int server_response;
    char topic[50];
    char content[2048];
};




























































//==============================================================================================//
/* 
int ID_COUNTER = 0;

typedef struct {
  int id, in_use;
  Connection* next;
} Connection;

typedef struct {
  Connection* head;
  Connection* tail;
} List;

void initializeList(Connection* head, Connection* tail);
void insertNode(Connection* head, Connection* tail);
void display(Connection* head);
 */

//==============================================================================================//



/* 
void insertNode(Connection current_id, int data) {
  Connection* aux = malloc(sizeof(Connection));
  aux->data = data;
  aux->next = nullptr;
  if (head == nullptr) {
  head = aux;
  tail = aux;
  } else {
  tail->next = aux;
  tail = aux;
  }
} */
//searchAvailableID

//==============================================================================================//
/* 
struct Node {
  int data;
  Node* next;
};

void insertNode(int data) {
  Node* aux = new Node;
  aux->data = data;
  aux->next = nullptr;
  if (head == nullptr) {
  head = aux;
  tail = aux;
  } else {
  tail->next = aux;
  tail = aux;
  }
}

void List::removeNode(int data) {
  Node *current = head;
  Node *previous = nullptr;
  while (current != nullptr) {
    if (current->data == data) {
    if (previous == nullptr) { // HEAD
      head = current->next;
    } else if (current->next == nullptr) { //TAIL
      previous->next = nullptr;
      tail = previous;
  } else {
      previous->next = current->next;
  }
    delete current;
    return;
  }
  previous = current;
  current = current->next;
  }
}

void List::display() {
Node *aux = head;
while (aux != nullptr) {
cout << aux->data << "\t";
aux = aux->next;
}
cout << endl;
}


struct List {
  Node* head = nullptr;
  Node* tail = nullptr;

  void insertNode(int data);
  void removeNode(int data);
  void display();
};

 */

#endif