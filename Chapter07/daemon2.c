#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>

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
  struct passwd *pws;
  const char *user = "nopriv";

  pws = getpwnam(user);
  if (pws == NULL) {
    printf("Unknown user: %s\n", user);
    return 0;
  }

  daemonize();

  setuid(pws->pw_uid);

  while (1) {
    sleep(1);
  }

  return 1;
}
