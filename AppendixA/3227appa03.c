#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>

int main(int argc, char *argv[]) {

    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    char buffer[256] = "";

    struct addrinfo hints, *res;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = PF_UNSPEC;
    /* hints.ai_flags = AI_ADDRCONFIG | AI_CANONNAME; */
    hints.ai_socktype = SOCK_STREAM;

    if (3 != argc) {

        fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
        exit(1);

    }

    /* setup the address structure */
    returnStatus = getaddrinfo(argv[1], argv[2], &hints, &res);

    if(returnStatus) {

      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(returnStatus));
      exit(1);

    }

    /* create the socket      */
    simpleSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (simpleSocket == -1) {

        fprintf(stderr, "Could not create a socket!\n");
        exit(1);

    }
    else {
        fprintf(stderr, "Socket created!\n");
    }

    /*  connect to the address and port with our socket  */
    returnStatus = connect(simpleSocket, res->ai_addr, res->ai_addrlen);

    if (returnStatus == 0) {
        fprintf(stderr, "Connect successful!\n");
    }
    else {
        fprintf(stderr, "Could not connect to address!\n");
        close(simpleSocket);
        exit(1);
    }

    /* get the message from the server   */
    returnStatus = read(simpleSocket, buffer, sizeof(buffer));

    if ( returnStatus > 0 ) {
        printf("%d: %s", returnStatus, buffer);
    } else {
        fprintf(stderr, "Return Status = %d \n", returnStatus);
    }

    close(simpleSocket);
    freeaddrinfo(res);
    return 0;

}

