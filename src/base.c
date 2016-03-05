#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "base.h"

static jfdtFd_t *fd_list = 0;

void jfdtFdInit (jfdtFd_t *fd, int desc,
  void (*inhdl) (jfdtFd_t *),
  void (*outhdl) (jfdtFd_t *),
  void *userdata)
{
  fd->next = 0;
  fd->fd = desc;
  fd->flags = 0;
  fd->inhdl = inhdl;
  fd->outhdl = outhdl;
  fd->userdata = userdata;
}


void jfdtFdClose (jfdtFd_t *fd) {
  jfdtFdFini (fd);
  close (fd->fd);
}

void jfdtFdShutdown (jfdtFd_t *fd) {
  shutdown (fd->fd, SHUT_WR);
}

void jfdtFdFini (jfdtFd_t *fd) {
  jfdtFd_t **pp, *p;
  for (pp = &fd_list; (p = *pp); pp = &p->next) {
    if (fd == p) {
      p->next = fd->next;
      break;
    }
  }
}

void jfdtFdReqIn (jfdtFd_t *fd) {
  fd->flags |= 1;
}

void jfdtFdReqOut (jfdtFd_t *fd) {
  fd->flags |= 2;
}

/**
 * returning -1 means EOF; return 0 means no data.
 * Other negative values are errors.
 */
int jfdtFdRead (jfdtFd_t *fd, void *buf, int size) {
  int r = read (fd->fd, buf, size);
  if (r > 0) return r;
  if (r == 0) return -1;
  if (r == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) return 0;
  return jfdtErrnoMap (errno);
}

/**
 * returning 0 means can't write anything,
 * negative returns are errors.
 */
int jfdtFdWrite (jfdtFd_t *fd, void *buf, int size) {
  int r = write (fd->fd, buf, size);
  if (r > 0) return r;
  if (r == 0) return 0;
  if (r == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) return -1;
  return jfdtErrnoMap (errno);
}

void jfdtServe (void) {
  while (1) {
    fd_set rfds;
    fd_set wfds;
    jfdtFd_t *fd;
    int r;
    int n = 0;
    FD_ZERO (&rfds);
    FD_ZERO (&wfds);
    for (fd = fd_list; fd; fd = fd->next) {
      if (fd->flags & 3) {
	if (n <= fd->fd) {
	  n = fd->fd + 1;
	}
	if (fd->flags & 1) {
	  FD_SET (fd->fd, &rfds);
	}
	if (fd->flags & 2) {
	  FD_SET (fd->fd, &wfds);
	}
      }
    }
    r = select (n, &rfds, &wfds, 0, 0);
    if (r == -1) {
      jfdt_trace1 ("select failed: %s", jfdtErrorString (jfdtErrnoMap (errno)));
      exit (5);
    }
    for (fd = fd_list; fd; fd = fd->next) {
      if (FD_ISSET (fd->fd, &rfds)) {
	fd->flags &= ~1;
	fd->inhdl (fd);
      }
      if (FD_ISSET (fd->fd, &wfds)) {
	fd->flags &= ~2;
	fd->outhdl (fd);
      }
    }
  }
}
