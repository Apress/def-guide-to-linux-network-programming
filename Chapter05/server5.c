#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

void* thread_proc(void *arg);

int main(int argc, char *argv[])
{
    struct sockaddr_in sAddr;
    int listensock;
    int result;
    int nchildren = 1;
    pthread_t thread_id;
    int x;
    int val;
    
    if (argc > 1) {
      nchildren = atoi(argv[1]);
    }

    listensock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    val = 1;
    result = setsockopt(listensock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (result < 0) {
        perror("server5");
        return 0;
    }

    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(1972);
    sAddr.sin_addr.s_addr = INADDR_ANY;

    result = bind(listensock, (struct sockaddr *) &sAddr, sizeof(sAddr));
    if (result < 0) {
        perror("exserver5");
        return 0;
    }

    result = listen(listensock, 5);
    if (result < 0) {
        perror("exserver5");
        return 0;
    }

   for (x = 0; x < nchildren; x++) {
	result = pthread_create(&thread_id, NULL, thread_proc, (void *) listensock);
	if (result != 0) {
	  printf("Could not create thread.\n");
	  return 0;
	}
	sched_yield();
    }

   pthread_join (thread_id, NULL);
}

void* thread_proc(void *arg)
{
  int listensock, sock;
  char buffer[25];
  int nread;

  listensock = (int) arg;

  while (1) {
    sock = accept(listensock, NULL, NULL);
    printf("client connected to child thread %i with pid %i.\n", pthread_self(), getpid());
    nread = recv(sock, buffer, 25, 0);
    buffer[nread] = '\0';
    printf("%s\n", buffer);
    send(sock, buffer, nread, 0);
    close(sock);
    printf("client disconnected from child thread %i with pid %i.\n", pthread_self(), getpid());
  }
}
