#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
    struct sockaddr_in sAddr;
    int listensock;
    int newsock;
    char buffer[25];
    int result;
    int nread;
    int pid;
    int nchildren = 1;
    int x;
    int val;

    if (argc > 1) {
        nchildren = atoi(argv[1]);
    }

    listensock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    val = 1;
    result = setsockopt(listensock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (result < 0) {
        perror("server3");
        return 0;
    }

    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(1972);
    sAddr.sin_addr.s_addr = INADDR_ANY;

    result = bind(listensock, (struct sockaddr *) &sAddr, sizeof(sAddr));
    if (result < 0) {
        perror("exserver3");
        return 0;
    }

    result = listen(listensock, 5);
    if (result < 0) {
        perror("exserver3");
        return 0;
    }

    for (x = 0; x < nchildren; x++) {
        if ((pid = fork()) == 0) {
            while (1) {
                newsock = accept(listensock, NULL ,NULL);
                printf("client connected to child process %i.\n", getpid());
                nread = recv(newsock, buffer, 25, 0);
                buffer[nread] = '\0';
                printf("%s\n", buffer);
                send(newsock, buffer, nread, 0);
                close(newsock);
                printf("client disconnected from child process %i.\n", getpid());
            }
        }
    }

    wait(NULL);
}
