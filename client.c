/********************************************************
 *
 *                          client.c
 *
 *  This is the starting point for your char client.
 *******************************************************/

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "lib.h"
#include <pthread.h>

int sock;
char buffer[512];
char* ret;
char* username;
char* argvName;



void* sendMSG() {//both will run at the same time using pthread
	if (argvName == NULL) {//so the program works even without username
		strcpy(username, "ANONYMOUS");//DEFAULT ANONYMOUS
		printf("Forgot Username [DEFAULT TO ANONYMOUS]\n");
	}
	else {
		strcpy(username, argvName);
	}
	username = strcat(username, ": ");
	ret = fgets(buffer, 512, stdin);
	char* nBuffer = strcat(username, buffer);

	writeString(sock, nBuffer);
	if (strstr(nBuffer, "exit") != NULL) {//if string includes exit disconnects client from server
		ret = readString(sock);
		printf("%s", ret);
		free(username);
		close(sock);
		exit(1);
	}
	return NULL;
}

void* recieve() {
	while (1) {
		ret = readString(sock);
		if (strcmp(ret, "NULL") == 0) {
			printf("-----------------------------------------\n");
			break;
		}
		else {
			printf("%s", ret);
		}
	}
	return NULL;
}



int main(int argc, char** argv) {
	struct addrinfo hints;
	struct addrinfo* addr;
	struct sockaddr_in* addrinfo;
	int rc;
	
	pthread_t thRecieve, thSend;	
	
	memset(&hints, 0, sizeof(hints));	
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_ADDRCONFIG;



	rc = getaddrinfo("localhost", NULL, &hints, &addr);
	if (rc != 0) {
		printf("Host name lookup failed: %s\n", gai_strerror(rc));
		exit(1);
	}



	addrinfo = (struct sockaddr_in*)addr->ai_addr;
	sock = socket(addrinfo->sin_family, addr->ai_socktype, addr->ai_protocol);

	if (sock < 0) {
		printf("Can't create socket\n");
		exit(1);
	}



	addrinfo->sin_port = htons(55555);
	rc = connect(sock, (struct sockaddr*)addrinfo, addr->ai_addrlen);
	if (rc != 0) {
		printf("Can't connect to server\n");
		exit(1);
	}

	else {//print history of the last 0-12 messeges
		while (1) {
			ret = readString(sock);
			if (strcmp(ret, "NULL") == 0) {
				printf("-----------------------------------------\n");
				break;
			}
			else {
				printf("%s", ret);
			}
		}
	}



	freeaddrinfo(addr);

	username = malloc(sizeof(buffer));
	argvName = argv[1];

	while (1) {
		if (pthread_create(&thRecieve, NULL, recieve, NULL) != 0) {
			printf("error");
		}

		if (pthread_create(&thSend, NULL, sendMSG, NULL) != 0) {
			printf("error");
		}

		if (pthread_join(thRecieve, NULL) != 0) {
			printf("error");
		}
	}
	close(sock);
	exit(0);
}

