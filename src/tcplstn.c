#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "base.h"

static void hdl (jfdtFd_t *fd) {
  jfdtListener_t *lstn = fd->userdata;
  struct sockaddr s;
  socklen_t len = sizeof (s);
  int r = accept (fd->fd, &s, &len);
  if (r == -1) {
    jfdt_trace ("accept failed: %s", jfdtErrorString (jfdtErrnoMap (errno)));
    // TODO: what now?
    return;
  }
  jfdtFdReqIn (fd);
  lstn->acpt (lstn, r, &s, len);
}

int jfdtListenerCreateTcp (
  struct jfdt_listener *lstn,
  void (*acpt) (struct jfdt_listener *, int fd, void *, int),
  void *ud,
  int port)
{
  int r, one = 1;
  struct sockaddr_in6 sin;
  int fd = socket (AF_INET6, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1) {
    return jfdtErrnoMap (errno);
  }

  r = setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof (one));
  if (r == -1) {
    jfdt_trace ("reuse failed");
  }

  memset (&sin, 0, sizeof (sin));
  sin.sin6_family = AF_INET6;
  sin.sin6_port = htons (port);
  r = bind (fd, (struct sockaddr *)&sin, sizeof (sin));
  if (r == -1) {
    r = jfdtErrnoMap (errno);
    close (fd);
    return r;
  }

  r = listen (fd, 13);
  if (r == -1) {
    r = jfdtErrnoMap (errno);
    close (fd);
    return r;
  }

  lstn->acpt = acpt;
  lstn->userdata = ud;

  jfdtFdInit (&lstn->fd, fd, hdl, 0, lstn);
  jfdtFdReqIn (&lstn->fd);

  return 0;
}
