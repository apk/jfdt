#include "lineio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base.h"

static void lineio_hdlr (jfdtFd_t *fd, jfdtFdWhat_t what) {
  jfdtLineIo_t *io = fd->userdata;

  if (what & jfdtFdOut) {
    struct jfdt_lineio_data *wd = io->writedata;
    if (wd) {
      int r = jfdtFdWrite (&io->fd, wd->buf + io->writepos, wd->len - io->writepos);
      if (r < 0) {
        jfdtFdClose (&io->fd);
        io->err (io, "write error");
        return;
      }
      if (r == 0) {
        jfdtFdClose (&io->fd);
        io->err (io, "write eof");
        return;
      }
      io->writepos += r;
      if (io->writepos >= wd->len) {
        io->writedata = wd->next;
        io->writepos = 0;
        free (wd);
      }
      if (io->writedata) {
        jfdtFdReqOut (&io->fd);
      }
    }
  }

  if (what & jfdtFdIn) {
    int r, l;

    if (io->readpos >= io->readalloc) {
      int nl = 2 * io->readalloc / 3 + 512;
      io->readbuf = realloc (io->readbuf, nl);
      io->readalloc = nl;
    }

    l = io->readalloc - io->readpos;

    r = jfdtFdRead (&io->fd, io->readbuf + io->readpos, l);
    if (r <= 0) {
      io->err (io, r < 0 ? "read error" : "read eof");
      jfdtFdClose (&io->fd);
      return;
    }
    io->readpos += r;

    for (r = 0; r < io->readpos; r ++) {
      if (io->readbuf [r] != '\n') continue;
      io->readbuf [r] = 0;
      io->proc (io, io->readbuf);
      if (r + 1 < io->readpos) {
        memmove (io->readbuf, io->readbuf + r + 1, io->readpos - r - 1);
      }
      io->readpos -= r + 1;
      r = -1;
    }
    jfdtFdReqIn (&io->fd);
  }
}

void jfdtLineIoInit (jfdtLineIo_t *io, int fd,
		     void (*proc) (jfdtLineIo_t *io, char *data),
		     void (*err) (jfdtLineIo_t *io, const char *msg),
		     void *userdata) {
  io->proc = proc;
  io->err = err;
  io->readpos = 0;
  io->readbuf = 0;
  io->readalloc = 0;
  io->writedata = 0;
  io->writepos = 0;
  io->userdata = userdata;

  jfdtFdInit (&io->fd, fd, lineio_hdlr, io);
  jfdtFdReqIn (&io->fd);
}

void jfdtLineIoFini (jfdtLineIo_t *io) {
  jfdtFdFini (&io->fd);
  while (io->writedata) {
    struct jfdt_lineio_data *d = io->writedata;
    io->writedata = d->next;
    free (d);
  }
  free (io->readbuf);
}

void jfdtLineIoSend (jfdtLineIo_t *io, const char *msg) {
  int l = strlen (msg);
  struct jfdt_lineio_data *d = malloc (sizeof (struct jfdt_lineio_data) + l);
  struct jfdt_lineio_data **dp;
  strcpy (d->buf, msg);
  d->buf [l] = '\n';
  d->len = l + 1;
  d->next = 0;
  for (dp = &io->writedata; *dp; dp = &(*dp)->next);
  *dp = d;
  jfdtFdReqOut (&io->fd);
}
