#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>

#include "base.h"

int jfdtListenerCreateTcp (
  struct jfdt_listener *lstn,
  void (*acpt) (struct jfdt_listener *, int fd, void *, int),
  void *ud,
  int port)
{
  int r;
  struct sockaddr_in6 sin;
  int fd = socket (AF_INET6, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1) {
    return jfdtErrnoMap (errno);
  }

  memset (&sin, 0, sizeof (sin));
  sin.sin6_family = AF_INET6;
  sin.sin6_port = port;
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
  return fd;
}
