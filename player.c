#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "player.h"

void printPlayerList(player *list)
{
    player *currentNode = list;
    while (1)
    {
        if(currentNode->next == NULL) break;
        currentNode = currentNode->next;
        printf("LIST: Number:%d  IP: %s:%d. Name: %s\n", currentNode->number, currentNode->ip_addr, currentNode->port, currentNode->name);
    }

}

player *addNewPlayer(player *list, char *ip_addr, int port, char *name, int number)
{
    player *currentNode = (player *)malloc(sizeof(player));
    currentNode = list;
    while (1)
    {
        if (currentNode->next == NULL) break;
        currentNode = currentNode->next;
    }

    if (ip_addr != NULL && name != NULL)
    {
        player *newPl;
        newPl = (player *)malloc(sizeof(player));
        strcpy(newPl->ip_addr, ip_addr);
        newPl->port = port;
        strcpy(newPl->name, name);
        newPl->number = number;
        currentNode->next = newPl;
        newPl->prev = currentNode;
    }

    return list;
}

void setPlayerName(player *list, char *name, int i)
{
    player *currentNode = list;
    while (1)
    {
        if (currentNode->next == NULL) break;
        currentNode = currentNode->next;
        if (i == currentNode->number)
        {
            printf("Client %d set name: %s\n", i, name);
            strcpy(currentNode->name, name);
        }
    }

}

int countPlayer(player *list)
{
    player *currentNode = list;
    int i = 0;
    while (1)
    {
        if(currentNode->next == NULL) break;
        i++;
        currentNode = currentNode->next;
    }
    return i;
}

char *playerInfo(player *list, int index)
{
    player *currentNode = list;
    int i = 0;
    for(i = 0; i < index; i++)
    {
        currentNode = currentNode->next;
    }
    char *info;
    info = ( char *)malloc(100);
    char tmp[100];
    bzero(info, sizeof(info));
    strcat(info, currentNode->ip_addr);
    strcat(info, ":");
    sprintf(tmp, "%d", currentNode->port);
    strcat(info, tmp);
    strcat(info, ":");
    strcat(info, currentNode->name);
    info[strlen(info)] = '\0';
    bzero(tmp, sizeof(tmp));
    return info;
}

char *getPlayerName(player *list, int number)
{
    player *currentNode = list;
    while (1)
    {
        if(currentNode->next == NULL) break;
        currentNode = currentNode->next;
        if (currentNode->number == number)
        {
            return currentNode->name;
        }
    }
    return "#";
}

player *playerDisconnect(player *list, int number)
{
    player *currentNode = list;
    while (1)
    {
        if(currentNode->next == NULL) break;
        currentNode = currentNode->next;
        if (currentNode->number == number)
        {
            if(currentNode->next == NULL)
            {
                player *prevNode = currentNode->prev;
                prevNode->next = NULL;
                currentNode->prev = NULL;
            }
            else
            {
                player *prevNode = currentNode->prev;
                player *nextNode = currentNode->next;
                prevNode->next = nextNode;
                nextNode->prev = prevNode;
                currentNode->prev = NULL;
                currentNode->next = NULL;
            }

            free(currentNode);
        }
    }
    return list;
}