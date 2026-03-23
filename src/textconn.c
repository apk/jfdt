#include "textconn.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void killconn (textConn_t *conn) {
  struct textconn_data *d;
  struct textconn **pp;
  for (pp = &conn->list->conns; *pp; pp = &(*pp)->next) {
    if (*pp == conn) {
      *pp = conn->next;
      break;
    }
  }
  jfdtFdClose (&conn->fd);
  while ((d = conn->writedata)) {
    conn->writedata = d->next;
    free (d);
  }
  conn->list->fini (conn);
}

static void inhdl (jfdtFd_t *fd) {
  textConn_t *conn = fd->userdata;

  int r, l;

  // TODO: Should do this before requesting read.
  if (conn->readpos >= conn->readalloc) {
    int nl = 2 * conn->readalloc / 3 + 512;
    conn->readbuf = realloc (conn->readbuf, nl);
    conn->readalloc = nl;
  }

  l = conn->readalloc - conn->readpos;

  r = jfdtFdRead (&conn->fd, conn->readbuf + conn->readpos, l);
  if (r == -1) {
    killconn (conn);
    return;
  }
  if (r < 0) {
    killconn (conn);
    return;
  }
  conn->readpos += r;

  for (r = 0; r < conn->readpos; r ++) {
    textScan_t S;
    const char *cmd;
    if (conn->readbuf [r] != '\n') continue;
    /* Line end here, use line */
    conn->readbuf [r] = 0;
    textScanInit (&S, conn->readbuf);
    cmd = textScanGetName (&S);
    if (cmd) {
      /* Got a command (otherwise the line is  garbage) */
      struct textconncmd *ce;
      for (ce = conn->list->cmdtable; ce->name; ce ++) {
	if (!strcmp (ce->name, cmd)) {
	  /* Have command, handle */
	  printf ("Have command (%s)...\n", cmd);
	  ce->handler (conn, cmd, S.str);
	  break;
	}
      }
      if (!ce->name) {
	/* Unknown command, error out */
	printf ("Bad command (%s)...\n", conn->readbuf);
      }
    }
    if (r + 1 < conn->readpos) {
      memmove (conn->readbuf, conn->readbuf + r + 1, conn->readpos - r - 1);
    }
    conn->readpos -= r + 1;
  }
  jfdtFdReqIn (&conn->fd);
}

static void outhdl (jfdtFd_t *fd) {
  textConn_t *conn = fd->userdata;
  struct textconn_data *wd = conn->writedata;
  if (wd) {
    int r = jfdtFdWrite (&conn->fd, wd->buf + conn->writepos, wd->len - conn->writepos);
    if (r < 0) {
      killconn (conn);
      return;
    } 
    if (r == 0) {
      killconn (conn);
      return;
    }
    conn->writepos += r;
    if (conn->writepos >= wd->len) {
      conn->writedata = wd->next;
      conn->writepos = 0;
      free (wd);
    }
    if (conn->writedata) {
      jfdtFdReqOut (&conn->fd);
    }
  }
}

void textConnListAdd (textConnList_t *list, textConn_t *conn, int fd, void *userdata) {
  // TODO: API: We expect an allocated textConn_t here - good?
  conn->list = list;
  conn->userdata = userdata;

  conn->next = list->conns;
  list->conns = conn;

  conn->readpos = 0;
  conn->readbuf = 0;
  conn->readalloc = 0;

  jfdtFdInit(&conn->fd, fd, inhdl, outhdl, conn);
  // TODO: Should request with buffer space only.
  jfdtFdReqIn (&conn->fd);
}

void textConnListInit (textConnList_t *list, const char *name,
		       void (*fini) (textConn_t *conn),
		       textConnCmd_t *cmdtable)
{
  list->conns = 0;
  list->fini = fini;
  list->cmdtable = cmdtable;
}

void textConnSend (textConn_t *conn, const char *msg) {
  int l = strlen (msg);
  struct textconn_data **dp, *d = malloc (sizeof (struct textconn) + l);
  strcpy (d->buf, msg);
  d->buf [l] = '\n'; // TODO: Separator hacking.
  d->len = l + 1;
  for (dp = &conn->writedata; *dp; dp = &(*dp)->next);
  d->next = *dp;
  *dp = d;
  jfdtFdReqOut (&conn->fd);
}

void textConnListSendToPredicated (textConnList_t *list,
				   const char *msg,
				   int (*pred) (textConn_t *conn, void *ud),
				   void *ud) {
  textConn_t *conn;
  for (conn = list->conns; conn; conn = conn->next) {
    if (pred (conn, ud)) {
      textConnSend (conn, msg);
    }
  }
}
