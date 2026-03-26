#include "textconn.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void killconn (jfdtTextConn_t *conn) {
  struct jfdt_textconn_data *d;
  struct jfdt_textconn **pp;
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

static void hdlr (jfdtFd_t *fd, jfdtFdWhat_t what) {
  jfdtTextConn_t *conn = fd->userdata;

  if (what & jfdtFdOut) {
    struct jfdt_textconn_data *wd = conn->writedata;
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

  if (what & jfdtFdIn) {
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
	/* Got a command (otherwise the line is garbage) */
	struct jfdt_textconncmd *ce;
        for (ce = conn->list->cmdtable; ce->name; ce ++) {
	  if (!strcmp (ce->name, cmd)) {
	    /* Have command, handle */
	    printf ("Have command (%s)...\n", cmd);
	    ce->handler (conn, cmd, S.str);
	    /* TODO: We should return to not execute the ReqIn then,
	     * and we should schedule a 'call me again (if I'm still
	     * alive)' to process more data.
	     */
	    break;
	  }
        }
        if (!ce->name) {
	  /* Unknown command, error out */
	  printf ("Bad command (%s)...\n", conn->readbuf);
        }
      }
      /* TODO: This is going to be interesting to do before calling the handler;
       * perhaps we need an explicit procpos to postpone the move.
       */
      if (r + 1 < conn->readpos) {
        memmove (conn->readbuf, conn->readbuf + r + 1, conn->readpos - r - 1);
      }
      conn->readpos -= r + 1;
    }
    jfdtFdReqIn (&conn->fd);
  }
}

void jfdtTextConnListAdd (jfdtTextConnList_t *list, jfdtTextConn_t *conn, int fd, void *userdata) {
  // TODO: API: We expect an allocated jfdtTextConn_t here - good?
  conn->list = list;
  conn->userdata = userdata;

  conn->next = list->conns;
  list->conns = conn;

  conn->readpos = 0;
  conn->readbuf = 0;
  conn->readalloc = 0;

  jfdtFdInit(&conn->fd, fd, hdlr, conn);
  // TODO: Should request with buffer space only.
  jfdtFdReqIn (&conn->fd);
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
  int l = strlen (msg);
  struct jfdt_textconn_data **dp, *d = malloc (sizeof (struct jfdt_textconn) + l);
  strcpy (d->buf, msg);
  d->buf [l] = '\n'; // TODO: Separator hacking.
  d->len = l + 1;
  for (dp = &conn->writedata; *dp; dp = &(*dp)->next);
  d->next = *dp;
  *dp = d;
  jfdtFdReqOut (&conn->fd);
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
