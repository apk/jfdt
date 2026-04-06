#include "lineclt.h"

#include <string.h>
#include <stdio.h>

#include "base.h"
#include "sock.h"

static void lineclt_set_timer (jfdtLineClt_t *clt);
static void lineclt_io_proc (jfdtLineIo_t *io, char *data);
static void lineclt_io_err (jfdtLineIo_t *io, const char *msg);

static void lineclt_tmr (jfdtTimer_t *t, jfdtTime_t now) {
  jfdtLineClt_t *clt = t->userdata;
  int fd;

  fd = jfdtOpenTcp (clt->host, clt->port);
  if (fd < 0) {
    printf ("Direct restart\n");
    lineclt_set_timer (clt);
    return;
  }

  clt->state = jfdtLineClt_connected;
  jfdtLineIoInit (&clt->io, fd, lineclt_io_proc, lineclt_io_err, clt);

  clt->stat (clt, 0);
}

static void lineclt_set_timer (jfdtLineClt_t *clt) {
  jfdtTime_t delay = jfdtGetTime ();
  jfdtTimeAddMillis (&delay, clt->delay);
  jfdtTimerInit (&clt->t, lineclt_tmr, clt);
  jfdtTimerSet (&clt->t, delay);
}

void jfdtLineCltInit (jfdtLineClt_t *clt, const char *host, int port,
		      int delay,
                      void (*proc) (jfdtLineClt_t *clt, const char *data),
                      void (*stat) (jfdtLineClt_t *clt, const char *err),
                      void *userdata) {
  clt->host = host;
  clt->port = port;
  clt->delay = delay > 0 ? delay : 30000;
  clt->proc = proc;
  clt->stat = stat;
  clt->userdata = userdata;
  clt->state = jfdtLineClt_waiting;

  lineclt_set_timer (clt);
}

void jfdtLineCltFini (jfdtLineClt_t *clt) {
  switch (clt->state) {
  case jfdtLineClt_waiting:
    jfdtTimerUnset (&clt->t);
    break;
  case jfdtLineClt_connected:
    jfdtLineIoFini (&clt->io);
    break;
  }
}

void jfdtLineCltSend (jfdtLineClt_t *clt, const char *msg) {
  switch (clt->state) {
  case jfdtLineClt_connected:
    jfdtLineIoSend (&clt->io, msg);
    break;
  case jfdtLineClt_waiting:
    break;
  }
}

static void lineclt_io_proc (jfdtLineIo_t *io, char *data) {
  jfdtLineClt_t *clt = io->userdata;
  clt->proc (clt, data);
}

static void lineclt_io_err (jfdtLineIo_t *io, const char *msg) {
  jfdtLineClt_t *clt = io->userdata;

  jfdtLineIoFini (&clt->io);
  clt->state = jfdtLineClt_waiting;

  lineclt_set_timer (clt);
  clt->stat (clt, msg ? msg : "?err");
}
