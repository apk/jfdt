#include "base.h"

typedef struct jfdt_textconn {
  struct jfdt_textconn *next;
  struct jfdt_textconnlist *list;
  jfdtFd_t fd;

  char *readbuf;
  int readpos;
  int readalloc;

  struct jfdt_textconn_data {
    int len;
    struct jfdt_textconn_data *next;
    char buf [1];
  } *writedata;
  int writepos;

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

typedef struct jfdt_textbuf {
  char *data;
  int alloc_len;
  int len;
} textBuf_t;

void textBufInit (textBuf_t *b);
void textBufAddName (textBuf_t *b, char *name);
void textBufAddString (textBuf_t *b, char *str);
void textBufAddAsgnInt (textBuf_t *b, char *name, int val);
char *textBufFini (textBuf_t *b);

typedef struct jfdt_textscan {
  const char *str;
} textScan_t;

void textScanInit (textScan_t *s, const char *txt);
const char *textScanGetName (textScan_t *s);
char *textScanGetStr (textScan_t *s);
