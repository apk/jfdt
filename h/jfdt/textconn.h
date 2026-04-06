#include "base.h"
#include "lineio.h"

typedef struct jfdt_textconn {
  struct jfdt_textconn *next;
  struct jfdt_textconnlist *list;
  jfdtLineIo_t io;
  void *userdata;
} jfdtTextConn_t;

typedef struct jfdt_textconnlist {
  struct jfdt_textconn *conns;
  void (*fini) (struct jfdt_textconn *);
  struct jfdt_textconncmd *cmdtable;
  void *userdata;
} jfdtTextConnList_t;

#define TEXTCONN_CMD_HDLR(x) \
  void (x) (jfdtTextConn_t *conn, const char *cmd, const char *args) /* TODO: Should be a command context, for tagging */

#define TEXTCONN_CMD_FAIL(x) \
  /* TODO: Send back a failure code, on what is visible as 'conn' */

void jfdtTextConnSend (jfdtTextConn_t *conn, const char *msg);

void jfdtTextConnListAdd (jfdtTextConnList_t *list, jfdtTextConn_t *conn, int fd, void *userdata);

typedef struct jfdt_textconncmd {
  char *name;
  TEXTCONN_CMD_HDLR(*handler);
  char *help;
} jfdtTextConnCmd_t;

void jfdtTextConnListInit (jfdtTextConnList_t *list, const char *name,
			   void (*fini) (jfdtTextConn_t *conn),
			   jfdtTextConnCmd_t *cmdtable);

void jfdtTextConnListSendToPredicated (jfdtTextConnList_t *list,
				       const char *msg,
				       int (*pred) (jfdtTextConn_t *conn, void *ud),
				       void *ud);
