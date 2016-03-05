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
  struct buf *o = b->other;
  jfdt_trace ("bkill");
  bkillone (b);
  if (o != b) {
    bkillone (o);
    if (o == b + 1) {
      free (b);
    }
  } else {
    free (b);
  }
}

static void breqs (struct buf *b) {
  struct buf *o = b->other;
  if (b->eof > 1 && o->eof > 1) {
    bkill (b);
    return;
  }
  if (b->end < sizeof (b->buf) && !b->eof) {
    /* Space to read */
    /* Might try immediate read? */
    jfdtFdReqIn (&b->fd);
  }
  if (o->end > o->start || o->eof == 1) {
    /* Data to write */
    /* XXX Might try an immediate write... */
    jfdtFdReqOut (&b->fd);
  }
}

static void inhdl (jfdtFd_t *fd) {
  struct buf *b = fd->userdata;
  int r = jfdtFdRead (fd, b->buf + b->end, sizeof (b->buf) - b->end);
  if (r >= 0) {
    b->end +=r;
  } else if (r == -1) {
    b->eof |= 1;
  } else {
    bkill (b);
    return;
  }
  breqs (b);
}

static void outhdl (jfdtFd_t *fd) {
  struct buf *b = fd->userdata;
  struct buf *o = b->other;
  if (o->end > o->start) {
    int r = jfdtFdWrite (&b->fd, o->buf + o->start, o->end - o->start);
    if (r >= 0) {
      o->start +=r;
      if (o->start == o->end) {
	o->start = 0;
	o->end = 0;
      }
    } else {
      bkill (b);
      return;
    }
    breqs (b);
  } else if (o->eof) {
    jfdtFdShutdown (&b->fd);
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
#if 0
  int fd2 = jfdtOpenTcp ("localhost", 6601);
  if (fd2 < 0) {
    jfdt_trace ("Closing %d", fd);
    jfdtCloseFd (fd);
    return;
  }
  b = malloc (2 * sizeof (struct buf));
  binit (b, fd, b + 1);
  binit (b + 1, fd2, b);
#else
  b = malloc (sizeof (struct buf));
  binit (b, fd, b);
#endif
}

int main () {

  jfdtListener_t lstn;

  int r = jfdtListenerCreateTcp (&lstn, acpt, 0, 6600);
  if (r < 0) {
    jfdt_trace ("Failed to create listener: %s", jfdtErrorString (r));
    exit (1);
  }
  jfdt_trace ("main loop");

  jfdtServe ();
  return 0;
}
