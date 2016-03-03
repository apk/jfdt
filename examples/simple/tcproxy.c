#include <jfdt/base.h>

struct buf {
  jfdtFd_t fd;
  char buf [1024];
  int start;
  int end;
  struct buf *other;
};

static void binit (struct buf *b, int fd, struct buf *o) {
  nuxFdInit (&b->fd, fd, req, hdl, b);
  b->start = 0;
  b->end = 0;
  b->other = o;
}

static void acpt (jfdtListener_t *l, int fd, void *addr, int adsize) {
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
