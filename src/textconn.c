#include "textconn.h"

#include <stdio.h>
#include <string.h>

static void killconn (jfdtTextConn_t *conn);

static void textconn_err (jfdtLineIo_t *io, const char *msg) {
  (void)msg;
  killconn (io->userdata);
}

static void textconn_lineproc (jfdtLineIo_t *io, char *data) {
  jfdtTextConn_t *conn = io->userdata;
  textScan_t S;
  const char *cmd;

  textScanInit (&S, data);
  cmd = textScanGetName (&S);
  if (cmd) {
    struct jfdt_textconncmd *ce;
    for (ce = conn->list->cmdtable; ce->name; ce ++) {
      if (!strcmp (ce->name, cmd)) {
        printf ("Have command (%s)...\n", cmd);
        ce->handler (conn, cmd, S.str);
        break;
      }
    }
    if (!ce->name) {
      printf ("Bad command (%s)...\n", data);
    }
  }
}

static void killconn (jfdtTextConn_t *conn) {
  struct jfdt_textconn **pp;
  for (pp = &conn->list->conns; *pp; pp = &(*pp)->next) {
    if (*pp == conn) {
      *pp = conn->next;
      break;
    }
  }
  jifdtLineIoFini (&conn->io);
  conn->list->fini (conn);
}

void jfdtTextConnListAdd (jfdtTextConnList_t *list, jfdtTextConn_t *conn, int fd, void *userdata) {
  conn->list = list;
  conn->userdata = userdata;

  conn->next = list->conns;
  list->conns = conn;

  jfdtLineIoInit (&conn->io, fd, textconn_lineproc, textconn_err, conn);
}

void jfdtTextConnListInit (jfdtTextConnList_t *list, const char *name,
			   void (*fini) (jfdtTextConn_t *conn),
			   jfdtTextConnCmd_t *cmdtable)
{
  list->conns = 0;
  list->fini = fini;
  list->cmdtable = cmdtable;
}

void jfdtTextConnSend (jfdtTextConn_t *conn, const char *msg) {
  jfdtLineIoSend (&conn->io, msg);
}

void jfdtTextConnListSendToPredicated (jfdtTextConnList_t *list,
				       const char *msg,
				       int (*pred) (jfdtTextConn_t *conn, void *ud),
				       void *ud) {
  jfdtTextConn_t *conn;
  for (conn = list->conns; conn; conn = conn->next) {
    if (pred (conn, ud)) {
      jfdtTextConnSend (conn, msg);
    }
  }
}
