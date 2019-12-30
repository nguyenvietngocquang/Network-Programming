#define LENGTH_MSG 1024

#define ROOM_SIZE 2
#define ROOM_NUM 10

typedef struct Room
{
    int id;
    int client_num;
} Room;

typedef struct Client
{
    int connfd;
    int room_id;
    char name[100];
} Client;

int clientSocket;
Room room_arr[ROOM_NUM];
struct Queue *responses;
int iLocation [2];
int flag_turn;