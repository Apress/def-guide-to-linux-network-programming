#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>


int daemonize()
{
  pid_t pid;
  long n_desc;
  int i;

  if ((pid = fork()) != 0) {
    exit(0);
  }

  setsid();

  if ((pid = fork()) != 0) {
    exit(0);
  }

  chdir("/");
  umask(0);

  n_desc = sysconf(_SC_OPEN_MAX);
  for (i = 0; i < n_desc; i++) {
    close(i);
  }

  return 1;
}

int main(int argc, char **argv)
{
  daemonize();

  openlog("test_daemon", LOG_PID, LOG_USER);
  syslog(LOG_INFO, "%s", "Hello World!");
  closelog();

  return 1;
}
