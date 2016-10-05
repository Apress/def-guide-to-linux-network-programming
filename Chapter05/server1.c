#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
    struct sockaddr_in sAddr;
    fd_set readset, testset;
    int listensock;
    int newsock;
    char buffer[25];
    int result;
    int nread;
    int x;
    int val;
    
    listensock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    val = 1;
    result = setsockopt(listensock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (result < 0) {
        perror("server1");
        return 0;
    }

    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(1972);
    sAddr.sin_addr.s_addr = INADDR_ANY;

    result = bind(listensock, (struct sockaddr *) &sAddr, sizeof(sAddr));
    if (result < 0) {
        perror("server1");
        return 0;
    }

    result = listen(listensock, 5);
    if (result < 0) {
        perror("server1");
        return 0;
    }

    FD_ZERO(&readset);
    FD_SET(listensock, &readset);

    while (1) {
        testset = readset;
        result = select(FD_SETSIZE, &testset, NULL, NULL, NULL);
        if (result < 1) {
            perror("server1");
            return 0;
        }
        for (x = 0; x < FD_SETSIZE; x++) {
            if (FD_ISSET(x, &testset)) {
                if (x == listensock) {
                    newsock = accept(listensock, NULL ,NULL);
                    FD_SET(newsock, &readset);
                } else {
                    nread = recv(x, buffer, 25, 0);
                    if (nread <= 0) {
                        close(x);
                        FD_CLR(x, &readset);
                        printf("client on descriptor #%i disconnected\n", x);
                    } else {
                        buffer[nread] = '\0';
                        printf("%s\n", buffer);
                        send(x, buffer, nread, 0);
                    }  
                }
            }
        }
    }
}
