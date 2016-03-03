#include <stdlib.h>

#include <jfdt/base.h>
#include <jfdt/sock.h>

struct buf {
  jfdtFd_t fd;
  char buf [1024];
  int start;
  int end;
  int eof;
  struct buf *other;
};

/* Question: Should jfdtFd_t require that we ran into
 * an WOULDBLOCK, or can we always ask?
 *
 * Also, jfdtFd_t should offer read/write routines that encapsulate
 * the whole WOULDBLOCK/EINTR/etc stuff.
 */

static void bkillone (struct buf *b) {
  jfdtFdClose (&b->fd);
}

static void bkill (struct buf *b) {
  bkillone (b);
  if (b->other != b) {
    bkillone (b->other);
    if (b->other == b + 1) {
      free (b);
    }
  } else {
    free (b);
  }
}

static void breqs (struct buf *b) {
  if (b->end < sizeof (b->buf) && !b->eof) {
    /* Space to read */
    /* Might try immediate read? */
    jfdtFdReqIn (&b->fd);
  }
  if (b->end > b->start || b->eof == 1) {
    /* Data to write */
    /* XXX Might try an immediate write... */
    jfdtFdReqOut (&b->other->fd);
  }
}

static void inhdl (jfdtFd_t *fd) {
  struct buf *b = fd->userdata;
  int r = jfdtFdRead (fd, b->buf + b->end, sizeof (b->buf) - b->end);
  if (r > 0) {
    b->end +=r;
  } else if (r == 0) {
    /* EWOULDBLOCK? */
    b->eof |= 1;
  } else {
    bkill (b);
    return;
  }
  breqs (b);
}

static void outhdl (jfdtFd_t *fd) {
  struct buf *b = fd->userdata;
  if (b->end > b->start) {
    int r = jfdtFdWrite (&b->other->fd, b->buf + b->start, b->end - b->start);
    if (r > 0) {
      b->start +=r;
      if (b->start == b->end) {
	b->start = 0;
	b->end = 0;
      }
    } else if (r == 0) {
      /* EWOULDBLOCK? */
      bkill (b);
    } else {
      bkill (b);
      return;
    }
    breqs (b);
  } else if (b->eof) {
    jfdtFdShutdown (&b->other->fd);
    b->eof = 2;
  }
}

static void binit (struct buf *b, int fd, struct buf *o) {
  jfdtFdInit (&b->fd, fd, inhdl, outhdl, b);
  jfdtFdReqIn (&b->fd);
  b->start = 0;
  b->end = 0;
  b->eof = 0;
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
  return 0;
}

/*
  Local variables:
  compile-command: "gcc -Wall -o tcproxy -I ../../h tcproxy.c"
  End:
*/
