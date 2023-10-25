#ifndef SERVER_LIB_H
#define SERVER_LIB_H

int ID_COUNTER = 1;
char ID_LIST[1024];

void initializeIdList();
int searchValidId();
int assignId();
void dischargeId(int id);

enum op_type {New_connection = 1, New_post, List_topics, Subscribe_topic, Disconnect};

struct BlogOperation {
    int client_id;
    int operation_type;
    int server_response;
    char topic[50];
    char content[2048];
};

#endif