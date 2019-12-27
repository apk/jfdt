#include "textconn.h"

static void inhdl (jfdtFd_t *fd) {
  textConn_t *conn = fd->userdata;
}

static void outhdl (jfdtFd_t *fd) {
  textConn_t *conn = fd->userdata;
}

void textConnListAdd (textConnList_t *list, textConn_t *conn, int fd, void *userdata) {
  // TODO: API: We expect an allocated textConn_t here - good?
  conn->list = list;
  conn->userdata = userdata;

  conn->next = list->conns;
  list->conns = conn;

  jfdtFdInit(&conn->fd, fd, inhdl, outhdl, conn);
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
