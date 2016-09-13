#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "base.h"

static jfdtFd_t *fd_list = 0;
static jfdtTimer_t *timer_list = 0;

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
  // TODO: should probably append, and warn if already in.
  fd->next = fd_list;
  fd_list = fd;
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

jfdtTime_t jfdtGetTime () {
  struct timeval tv;
  int r = gettimeofday (&tv, 0);
  if (r == -1) {
    exit (1);
  }
  return tv;
}

int jfdtTimeLessThan (jfdtTime_t a, jfdtTime_t b) {
  return (a.tv_sec < b.tv_sec ||
	  (a.tv_sec == b.tv_sec &&
	   a.tv_usec < b.tv_usec));
}

void jfdtTimeAddFrac (jfdtTime_t *t, int n, int d) {
  n *= 1000000;
  n /= d;
  t->tv_sec += n / 1000000;
  t->tv_usec += n % 1000000;
}

void jfdtTimeAddSecs (jfdtTime_t *t, int s) {
  t->tv_sec += s;
}

void jfdtTimeSub (jfdtTime_t *t, jfdtTime_t b) {
  t->tv_sec -= b.tv_sec;
  if (t->tv_usec < b.tv_usec) {
    t->tv_sec -= 1;
    t->tv_usec += 1000000;
  }
  t->tv_usec -= b.tv_usec;
}

void jfdtTimerInit (jfdtTimer_t *t,
		    void (*fire) (jfdtTimer_t *, jfdtTime_t now),
		    void *userdata) {
  t->f = fire;
  t->userdata = userdata;
  t->next = t;
}

void jfdtTimerUnset (jfdtTimer_t *t) {
  jfdtTimer_t **pp, *p;
  if (t->next != t) {
    for (pp = &timer_list; (p = *pp); pp = &p->next) {
      if (p == t) {
	*pp = t->next;
	return;
      }
    }
    jfdt_trace ("timer: unset: oops");
    sleep (3);
  }
}

void jfdtTimerSet (jfdtTimer_t *t, jfdtTime_t d) {
  jfdtTimer_t **pp, *p;
  if (t->next != t) {
    jfdtTimerUnset (t);
  }
  t->tm = d;
  for (pp = &timer_list; (p = *pp); pp = &p->next) {
    if (jfdtTimeLessThan (t->tm, p->tm)) {
      break;
    }
  }
  t->next = p;
  *pp = t;
}

void jfdtServe (void) {
  while (1) {
    fd_set rfds;
    fd_set wfds;
    jfdtFd_t *fd;
    jfdtTimer_t *t;
    struct timeval to, *p = 0;
    int r;
    int n = 0;
    while (t = timer_list) {
      jfdtTime_t now = jfdtGetTime ();
      if (jfdtTimeLessThan (now, t->tm)) {
	to = t->tm;
	jfdtTimeSub (&to, now);
	p = &to;
	break;
      }
      timer_list = t->next;
      t->next = t;
      t->f (t, now);
    }
    if (p) {
      jfdt_trace("T:%d.%d", (int)p->tv_sec, (int)p->tv_usec);
    }
    FD_ZERO (&rfds);
    FD_ZERO (&wfds);
    for (fd = fd_list; fd; fd = fd->next) {
      if (fd->flags & 3) {
	if (n <= fd->fd) {
	  n = fd->fd + 1;
	}
	if (fd->flags & 1) {
	  jfdt_trace ("R:%d", fd->fd);
	  FD_SET (fd->fd, &rfds);
	}
	if (fd->flags & 2) {
	  jfdt_trace ("W:%d", fd->fd);
	  FD_SET (fd->fd, &wfds);
	}
      }
    }
    jfdt_trace ("N:%d", n);
    r = select (n, &rfds, &wfds, 0, p);
    jfdt_trace (" :%d", r);
    if (r == -1) {
      jfdt_trace ("select failed: %s", jfdtErrorString (jfdtErrnoMap (errno)));
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
