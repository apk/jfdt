#include <jfdt/base.h>

struct buf {
  jfdtFd_t fd;
  char buf [1024];
  int start;
  int end;
  struct buf *other;
};

void acpt (jfdtListener_t *l, int fd, void *addr, int adsize) {
  struct buf *b;
  int fd2 = jfdtOpenTcp ("localhost", 6601);
  if (fd2 == -1) {
    jfdtTrace1 ("Closing %d", fd);
    jfdtCloseFd (fd);
    return;
  }
  b = malloc (2 * sizeof (struct buf));
  binit (b, fd, b + 1);
  binit (b + 1, fd2, b);
}

int main () {

  jfdtListener_t lstn;

  jfdtListenerCreateTcp (&lstn, acpt, 0, 6600);

  jfdtServe ();
}
