/***********************************************
 *
 *                server.c
 *
 *  This is the starting point for your chat server
 **********************************************/

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include "lib.h"

int client[FD_SETSIZE];
int countofClients = 0;

struct Node {//using linked list to keep track of msgs on server
    char* message;
    struct Node* next;
};
void printToALLClient(struct Node* head) {//sends to all clients
	int i = 0;
	for (i = 0; i < countofClients; i++) {
		int conn = client[i];
		struct Node* Cur = head;
		writeString(conn, "\n-----------------------------------------\n");
		writeString(conn, "               Server Chat               \n");
		while (Cur != NULL) {
			writeString(conn, Cur->message);
			Cur = Cur->next;
		}
		writeString(conn, "NULL");
	}
}
void printToClient(int conn, struct Node* head) {//print to new clients
    struct Node* Cur = head;
    writeString(conn, "-----------------------------------------\n");
    writeString(conn, "               Server Chat               \n");
    while (Cur != NULL) {
        writeString(conn, Cur->message);
        Cur = Cur->next;
    }
    writeString(conn, "NULL");
}
void deleteNode(struct Node** head)
{
    struct Node* temp = *head;
    *head = (*head)->next;
    free(temp);
}

int listlenght(struct Node* head)//lenght of list
{
    int lenght = 0;
    struct Node* Cur = head;
    while (Cur != NULL)
    {
        lenght++;
        Cur = Cur->next;
    }
    return lenght;
}

void push(struct Node** head, char* newmsg)//pushes msgs to end
{
    struct Node* nNode = malloc(sizeof(struct Node));
    nNode->message = newmsg;
    nNode->next = NULL;
    if (*head == NULL)
        *head = nNode;
    else
    {
        struct Node* lNode = *head;
        while (lNode->next != NULL)
        {
            lNode = lNode->next;
        }
        lNode->next = nNode;
    }
}


int main(int argc, char** argv) {
	int sock, conn;
	int i;
	int rc;
	struct sockaddr address;
	socklen_t addrLength = sizeof(address);
	struct addrinfo hints;
	struct addrinfo* addr;
	fd_set readfds, allfds;
	int maxfd, maxi;
	int nready;
	char* newmsg;
	char* leftmsg = "Disconnected From Server\n";
	struct Node* head = NULL;

	memset(&hints, 0, sizeof(hints));

	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
	if ((rc = getaddrinfo("localhost", "55555", &hints, &addr))) {
		printf("host name lookup failed: %s\n", gai_strerror(rc));
		exit(1);
	}


	sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if (sock < 0) {
		printf("Can't create socket\n");
		exit(1);
	}


	i = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));

	rc = bind(sock, addr->ai_addr, addr->ai_addrlen);
	if (rc < 0) {
		printf("Can't bind socket\n");
		exit(1);
	}

	freeaddrinfo(addr);

	rc = listen(sock, 5);
	if (rc < 0) {
		printf("Listen failed\n");
		exit(1);
	}

	maxfd = sock;
	maxi = -1;
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;
	FD_ZERO(&allfds);
	FD_SET(sock, &allfds);

	while (1) {
		readfds = allfds;
		nready = select(maxfd + 1, &readfds, NULL, NULL, NULL);
		if (FD_ISSET(sock, &readfds)) {
			conn = accept(sock, (struct sockaddr*)&address, &addrLength);
			countofClients++;
			for (i = 0; i < FD_SETSIZE; i++) {
				if (client[i] < 0) {
					client[i] = conn;
					break;
				}
			}
			if (conn < 0) {
				printf("Accept failed");
			}
			else {
				writeString(conn, "Wellcome To Server: Type [exit] To Leave\n");
				printToClient(conn, head);
			}
			FD_SET(conn, &allfds);
			if (conn > maxfd)
				maxfd = conn;
			if (i > maxi)
				maxi = i;
			if (--nready <= 0)
				continue;
		}
		for (i = 0; i <= maxi; i++) {
			conn = client[i];
			if (conn < 0)
				continue;
			if (FD_ISSET(conn, &readfds)) {
				newmsg = readString(conn);
				if (strstr(newmsg, "exit") != NULL) {//leaves chat when you type exit
					writeString(conn, leftmsg);
					close(conn);
					client[i] = -1;
					FD_CLR(conn, &allfds);
				}
				else {
					if (listlenght(head) == 12) {//history of 12 msgs
						deleteNode(&head);
					}
					push(&head, newmsg);
					printToALLClient(head);
					printf("%s", newmsg);//print to server terminal all inputs
				}
			}
		}
	}
	close(sock);
}