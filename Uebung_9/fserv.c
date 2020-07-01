#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>

#define BACKLOG 1
#define BUFSIZE 64

int inet_aton(const char *cp, struct in_addr *addr);

int main(int argc, char* argv[]) {
    pid_t childPid;
    struct in_addr addr;
    int socketFd, connFd, requestFd, recvBytes, port, readBytes;
    struct sockaddr_in socketAddr;

    if(argc < 3) {
        printf("Please enter ./tcp-server <ip> <port>\n");
        exit(EXIT_FAILURE);
    } else {
        inet_aton(argv[1], &addr);
        port = atoi(argv[2]);
    }

    // Create socket
    if((socketFd = socket(AF_INET, SOCK_STREAM, 0)) != -1) {
        printf("Server socket created.\n");
    } else {
        printf("Server socket creation failed.\n");
        exit(EXIT_FAILURE);
    }

    socketAddr.sin_family = AF_INET;
    socketAddr.sin_addr.s_addr = addr.s_addr;
    socketAddr.sin_port = htons(port);

    if((bind(socketFd, (struct sockaddr*)&socketAddr, sizeof(socketAddr))) != -1) {
        printf("Bound socket.\n");
    } else {
        printf("Socket binding failed.\n");
        exit(EXIT_FAILURE);
    }

    if(listen(socketFd, BACKLOG) == -1) {
        printf("Error listening.\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Listening on Port %d\n", port);
    }

    while(1) {
        if((connFd = accept(socketFd, NULL, NULL)) == -1) {
            perror("Error accepting connection.\n");
            exit(EXIT_FAILURE);
        } else {
            printf("Connection accepted.\n");
        }
        if((childPid = fork()) == 0) {
            connInstance(connFd, socketFd);
        }   
    }
}

void connInstance(int connFd, int socketFd) {
    int recvBytes, readBytes, requestFd;
    char msgBuf[BUFSIZE];
    char fileBuf[BUFSIZE];
    while(1) {
        memset(msgBuf, 0, BUFSIZE);
        recvBytes = recv(connFd, msgBuf, BUFSIZE, 0);

        if (recvBytes == -1) {
            printf("Error receiving msg.\n");
        } else if (recvBytes > 0) {

            if(strcmp(msgBuf, "QUIT\n") == 0) {
                printf("Closed connection.\n");
                close(connFd);
                return;
            }

            requestFd = open(msgBuf, O_RDONLY);

            if (requestFd == -1) {

                printf("File not found.\n");

            } else {

                while((readBytes = read(requestFd, fileBuf, BUFSIZE)) > 0) {
                    send(connFd, fileBuf, readBytes, 0);
                    memset(fileBuf, 0, BUFSIZE);
                }

                send(connFd, "\n", 2, 0);
            }
        }
    }
}