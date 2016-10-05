
/* UDP Server */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

#define MAXBUF 1024

int main(int argc, char* argv[])
{

    int udpSocket;
    int returnStatus = 0;
    int addrlen = 0;
    struct sockaddr_in udpServer, udpClient;
    char buf[MAXBUF];

    /* check for the right number of arguments */
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    /* create a socket */
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);

    if (udpSocket == -1)
    {
        fprintf(stderr, "Could not create a socket!\n");
        exit(1);
    } 
    else {
        printf("Socket created.\n");
    }

    /* setup the server address and port */
    /* use INADDR_ANY to bind to all local addresses */
    udpServer.sin_family = AF_INET;
    udpServer.sin_addr.s_addr = htonl(INADDR_ANY);

    /* use the port pased as argument */
    udpServer.sin_port = htons(atoi(argv[1]));

    /* bind to the socket */
    returnStatus = bind(udpSocket, (struct sockaddr*)&udpServer, sizeof(udpServer));

    if (returnStatus == 0) {
        fprintf(stderr, "Bind completed!\n");
    } 
    else {
        fprintf(stderr, "Could not bind to address!\n");
        close(udpSocket);
        exit(1);
    }

    while (1)
    {

        addrlen = sizeof(udpClient);

        returnStatus = recvfrom(udpSocket, buf, MAXBUF, 0, 
                                (struct sockaddr*)&udpClient, &addrlen);

        if (returnStatus == -1) {

            fprintf(stderr, "Could not receive message!\n");

        } 
        else {

            printf("Received: %s\n", buf);

            /* a message was received so send a confirmation */
            strcpy(buf, "OK");

            returnStatus = sendto(udpSocket, buf, strlen(buf)+1, 0, 
                                  (struct sockaddr*)&udpClient, sizeof(udpClient));

            if (returnStatus == -1) {
                fprintf(stderr, "Could not send confirmation!\n");
            }
            else {
                printf("Confirmation sent.\n");

            }
        }

    }

    /*cleanup */
    close(udpSocket);
    return 0;
}
