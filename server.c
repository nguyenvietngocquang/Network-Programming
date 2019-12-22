#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "server_params.h"
#include "player.h"
#include "room.h"
#include "checkwin.h"

player *list;
room roomList[MAX_ROOM];
int location [2];

char *get_params(char command[])
{
    int i = 0, j;
    while (command[i] != ' ')
    {
        i++;
    }
    i++;

    char *params = malloc(LENGTH_MSG);
    for (j = 0; i + j < strlen(command); j++)
    {
        *(params + j) = command[i + j];
    }
    *(params + j) = '\0';
    return params;
}

void send_msg_room(int playerNumber, int roomNumber, char *msg)
{
    if(countPlayerInRoom(roomList, roomNumber) == 2)
    {
        if(roomList[roomNumber].Player1 == playerNumber)
        {
            send(roomList[roomNumber].Player2, msg, strlen(msg), 0);
        }
        else if(roomList[roomNumber].Player2 == playerNumber)
        {
            send(roomList[roomNumber].Player1, msg, strlen(msg), 0);
        }
    }
}

int main()
{
    printf("Waiting client!\n");
    //Playerlist
    list = (player *)malloc(sizeof(player));

    setDefault(roomList, MAX_ROOM);

    // Tao server socket
    int serverSocket =  socket(PF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        perror("CREATE SOCKET");
        exit(0);
    }

    int i = 1;
    int check = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int));
    if (check == -1)
    {
        perror("Set reuse");
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(5000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    socklen_t len = sizeof(struct sockaddr_in);
    check = bind(serverSocket, (struct sockaddr *)&serverAddress, len);
    if (check == -1)
    {
        perror("BIND ERROR!");
        exit(0);
    }

    check = listen(serverSocket, 10);
    if (check == -1)
    {
        perror("Listen ERROR!");
        exit(1);
    }


    struct sockaddr_in clientAddress;
    len = sizeof(struct sockaddr_in);
    fd_set readfds; // tập readfds để check các socket
    fd_set masterfds; // một tập để lưu lại những thay đổi của tập readfds
    FD_ZERO(&masterfds);
    FD_ZERO(&readfds);
    FD_SET(serverSocket, &masterfds); // add serverSock vào tập masterfds
    int max_fd = serverSocket;
    struct timeval timeout;
    timeout.tv_sec = 90;
    timeout.tv_usec = 0;
    int n_select;

    do
    {
        memcpy(&readfds, &masterfds, sizeof(masterfds));
        n_select = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (n_select < 0)
        {
            perror("SELECT");
            exit(0);
        }
        else if (n_select == 0)
        {
            printf("Time out\n");
        }
        else
        {
            for (i = 0; i <= max_fd; i++)
            {
                if (FD_ISSET(i, &readfds))
                {
                    int close_fd = 0;
                    if (i == serverSocket)
                    {
                        int newCon = accept(serverSocket, (struct sockaddr *)&clientAddress, &len);
                        printf("Connection accepted.\n");
                        FD_SET(newCon, &masterfds);
                        if (newCon > max_fd) max_fd = newCon;
                    }
                    else
                    {
                        char message[LENGTH_MSG];
                        char response[LENGTH_MSG];
                        char msg_temp1[LENGTH_MSG], msg_temp2[LENGTH_MSG];
                        printf("Receive data in socket %d\n", i);
                        int nrecv = recv(i, message, 100, 0);
                        if (nrecv == -1)
                        {
                            printf("In socket %d\n", i);
                            perror("RECEIVE");
                            close_fd = 1;
                        }
                        else if (nrecv != 0)
                        {
                            int isCommand = 0, temp, roomNumber, playersRoom;
                            int x, y;
                            message[nrecv] = 0;
                            printf("%s\n", message);
                            if(strcmp(message, "/disconnect") == 0)
                            {
                                isCommand = 1;
                                printf("[-]Disconnect from %s:%d\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
                                list = playerDisconnect(list, i);
                                bzero(message, sizeof(message));
                                break;
                            }
                            if(strcmp(message, "/confirm_connection") == 0)
                            {
                                isCommand = 1;
                                list = addNewPlayer(list, inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port), "\0", i);
                                bzero(message, sizeof(message));
                                break;
                            }
                            if (strcmp(message, "/list") == 0)
                            {
                                isCommand = 1;
                                printPlayerList(list);
                                int idx = 0;
                                int n = countPlayer(list);
                                char sendBuf[LENGTH_MSG];
                                bzero(sendBuf, sizeof(sendBuf));
                                for (idx = 1; idx <= n; idx++)
                                {
                                    bzero(sendBuf, sizeof(sendBuf));
                                    strcpy(sendBuf, playerInfo(list, idx));
                                    send(i, sendBuf, LENGTH_MSG, 0);
                                    bzero(sendBuf, sizeof(sendBuf));
                                }
                                char tmp[100];
                                strcpy(tmp, "/endlist");
                                send(i, tmp, sizeof(tmp), 0);
                                bzero(tmp, sizeof(tmp));
                            }
                            if (strstr(message, "/setName"))
                            {
                                isCommand = 1;
                                printf("%s\n", get_params(message));
                                setPlayerName(list, get_params(message), i);
                                printPlayerList(list);
                                bzero(message, sizeof(message));

                            }
                            if (strstr(message, "/chooseRoom"))
                            {
                                isCommand = 1;
                                sprintf(response, "room_list: ");
                                for (temp = 0; temp < MAX_ROOM; temp++)
                                {
                                    sprintf(response + strlen(response), "%d-%d#", temp + 1, countPlayerInRoom(roomList, temp));
                                }
                                puts(response);
                                send(i, response, strlen(response), 0);
                            }
                            if (strstr(message, "/enterRoom"))
                            {
                                printf("Function EnterRoom\n");
                                isCommand = 1;
                                roomNumber = atoi(get_params(message));
                                printf("Request for enter room %d\n", roomNumber);
                                if (enterRoom(i, roomList, roomNumber) == 1)
                                {
                                    printRoomList(roomList);
                                    printf("Success!\n");
                                    char response[100] = "join_room_success\0";
                                    send(i, response, strlen(response), 0);
                                    sprintf(response, "wait_player: ");
                                    sprintf(msg_temp1, "refresh_room: ");
                                    sprintf(msg_temp2, "refresh_room: ");
                                    if(countPlayerInRoom(roomList, roomNumber) == 1)
                                    {
                                        puts(response);
                                        send(i, response, strlen(response) + 1, 0);
                                    }
                                    else if(roomList[roomNumber].Player1 == i && roomList[roomNumber].Player2 != 0)
                                    {
                                        sprintf(msg_temp1 + strlen(msg_temp1), "%s", getPlayerName(list, i));
                                        sprintf(msg_temp2 + strlen(msg_temp2), "%s", getPlayerName(list, roomList[roomNumber].Player2));
                                        send(roomList[roomNumber].Player2, msg_temp1, strlen(msg_temp1), 0);
                                        send(i, msg_temp2, strlen(msg_temp2), 0);
                                        puts(msg_temp1);
                                        puts(msg_temp2);
                                    }
                                    else if(roomList[roomNumber].Player2 == i && roomList[roomNumber].Player1 != 0)
                                    {
                                        sprintf(msg_temp1 + strlen(msg_temp1), "%s", getPlayerName(list, roomList[roomNumber].Player1));
                                        sprintf(msg_temp2 + strlen(msg_temp2), "%s", getPlayerName(list, i));
                                        send(i, msg_temp1, strlen(msg_temp1), 0);
                                        send(roomList[roomNumber].Player1, msg_temp2, strlen(msg_temp2) + 1, 0);
                                        puts(msg_temp1);
                                        puts(msg_temp2);
                                    }
                                    if(countPlayerInRoom(roomList, roomNumber) == 2)
                                    {
                                        for (y = 0; y <= 9; y++)
                                        {
                                            for (x = 0; x <= 9; x++)
                                            {
                                                roomList[roomNumber].Board[x][y] = 'E';
                                            }
                                        }
                                        roomList[roomNumber].turn = roomList[roomNumber].Player1;
                                        send(roomList[roomNumber].Player1, "your_turn: ", strlen("your_turn"), 0);
                                        send(roomList[roomNumber].Player2, "opponent_turn: ", strlen("opponent_turn"), 0);
                                    }

                                }
                                else
                                {
                                    printf("Can't join the room!\n");
                                    sprintf(response, "join_room_error: ");
                                    for (temp = 0; temp < MAX_ROOM; temp++)
                                    {
                                        sprintf(response + strlen(response), "%d-%d#", temp + 1, countPlayerInRoom(roomList, temp));
                                    }
                                    puts(response);
                                    send(i, response, strlen(response), 0);
                                }
                            }
                            if (strcmp(message, "/leaveRoom") == 0)
                            {
                                printf("Function Leave Room!\n");
                                isCommand = 1;
                                if (leaveRoom(i, roomList) == 1)
                                {
                                    printRoomList(roomList);
                                    printf("Leave room succeed\n");
                                }
                                else
                                {
                                    printf("Can't leave the room!\n");
                                };
                            }
                            if (strstr(message, "/play"))
                            {
                                printf("Function PlayGame\n");
                                int temp_data = atoi(get_params(message));
                                location[0] = temp_data % 10;
                                location[1] = (temp_data - location[0]) / 10;
                                playersRoom = inRoom(i, roomList);
                                int bWon;
                                sprintf(response, "new_play: ");
                                if(roomList[playersRoom].Player1 == i)
                                {
                                    roomList[playersRoom].Board[location[0]][location[1]] = 'X';
                                    bWon = checkWin(location[0], location[1], roomList[playersRoom].Board, 'X');
                                    sprintf(response + strlen(response), "%s", get_params(message));
                                    send(roomList[playersRoom].Player2, response, strlen(response), 0);
                                    if(bWon == 1)
                                    {
                                        send(roomList[roomNumber].Player1, "you_won_game: ", strlen("you_won_game: "), 0);
                                        send(roomList[roomNumber].Player2, "you_lose_game: ", strlen("you_lose_game: "), 0);
                                        puts("Player 1 win");
                                    }
                                    else
                                    {
                                        roomList[roomNumber].turn = roomList[roomNumber].Player2;
                                        send(roomList[roomNumber].Player2, "your_turn: ", strlen("your_turn"), 0);
                                        send(roomList[roomNumber].Player1, "opponent_turn: ", strlen("opponent_turn") + 1, 0);
                                    }

                                }
                                else if(roomList[playersRoom].Player2 == i)
                                {
                                    roomList[playersRoom].Board[location[0]][location[1]] = 'O';
                                    bWon = checkWin(location[0], location[1], roomList[playersRoom].Board, 'O');
                                    sprintf(response + strlen(response), "%s", get_params(message));
                                    send(roomList[playersRoom].Player1, response, strlen(response), 0);
                                    if(bWon == 1)
                                    {
                                        send(roomList[roomNumber].Player2, "you_won_game: ", strlen("you_won_game: "), 0);
                                        send(roomList[roomNumber].Player1, "you_lose_game: ", strlen("you_lose_game: "), 0);
                                        puts("Player 2 win");
                                    }
                                    else
                                    {
                                        roomList[roomNumber].turn = roomList[roomNumber].Player1;
                                        send(roomList[roomNumber].Player1, "your_turn: ", strlen("your_turn"), 0);
                                        send(roomList[roomNumber].Player2, "opponent_turn: ", strlen("opponent_turn"), 0);
                                    }
                                }
                            }
                            if (strstr(message, "/new_message"))
                            {
                                printf("Function Message\n");
                                isCommand = 1;
                                sprintf(response, "new_opponent_message: ");
                                playersRoom = inRoom(i, roomList);
                                if(roomList[playersRoom].Player1 == i)
                                {
                                    sprintf(response + strlen(response), "%s: %s", getPlayerName(list, i), get_params(message));
                                    send(roomList[playersRoom].Player2, response, strlen(response), 0);
                                }
                                else if(roomList[playersRoom].Player2 == i)
                                {
                                    sprintf(response + strlen(response), "%s: %s", getPlayerName(list, i), get_params(message));
                                    send(roomList[playersRoom].Player1, response, strlen(response), 0);
                                }
                            }
                            if (strstr(message, "/new_world_message"))
                            {
                                printf("Function World Message\n");
                                isCommand = 1;
                                sprintf(response, "world_message %s: %s", getPlayerName(list, i), get_params(message));
                                printf("%s\n", response);

                                int other_player;

                                player *current_node = list;
                                while(1)
                                {
                                    if(strcmp(current_node->name, "\0") != 0 && current_node->number != i)
                                    {
                                        printf("Send world message for %d: %s\n", current_node->number, response);
                                        send(current_node->number, response, sizeof(response), 0);
                                    }
                                    if( current_node->next == NULL) break;
                                    current_node = current_node->next;
                                }

                            }
                            if(isCommand == 0)
                            {
                                printf("Client:  %s\n", message);
                                send(i, message, strlen(message), 0);
                                bzero(message, sizeof(message));
                            }

                        }
                        else
                        {
                            close_fd = 1;
                        }
                    }
                    if (close_fd == 1)
                    {
                        FD_CLR(i, &masterfds);
                        close(i);
                    }
                }
            }
        }
    }
    while(1);
    return 0;
}