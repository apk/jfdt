#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>

#include "base.h"

/**
 * Do a synchronous TCP connect.
 */
int jfdtOpenTcp (const char *host, int port) {
  int err = 0;
  int r;
  char serv [32];
  struct addrinfo hints, *res, *c;
  sprintf (serv, "%d", port);
  memset (&hints, 0, sizeof (hints));
  hints.ai_flags = AI_NUMERICSERV;
  r = getaddrinfo (host, serv, 0, &res);
  if (r == -1) {
    return jfdtErrnoMap (errno);
  }
  for (c = res; c; c = c->ai_next) {
    int fd = socket (c->ai_family, SOCK_STREAM, c->ai_protocol);
    if (fd == -1) {
      if (err == 0)  {
	err = jfdtErrnoMap (errno);
      }
      continue;
    }
    r = connect (fd, c->ai_addr, c->ai_addrlen);
    if (r == -1) {
      if (err == 0)  {
	err = jfdtErrnoMap (errno);
      }
      close (fd);
      continue;
    }
    freeaddrinfo (res);
    return fd;
  }
  freeaddrinfo (res);
  if (err == 0) {
    err = jfdtErrorMap ("no-addr");
  }
  return err;
}

void jfdtCloseFd (int fd) {
  close (fd);
}
