typedef struct textconn {
  struct textconn *next;
  struct textconnlist *list;
  void *userdata;
} textConn_t;

typedef struct textconnlist {
  struct textconn *conns;
  void (*fini) (struct textconn *);
  void *userdata;
} textConnList_t;

#define TEXTCONN_CMD_HDLR(x) \
  void (x) (textConn_t *conn, char *args, char *full) /* TODO: Should be a command context, for tagging */

#define TEXTCONN_CMD_FAIL(x) \
  /* TODO: Send back a failure code, on what is visible as 'conn' */

void textConnListAdd (textConnList_t *list, textConn_t *conn, int fd, void *userdata);

typedef struct textconncmd {
  char *name;
  TEXTCONN_CMD_HDLR(*handler);
  char *help;
} textConnCmd_t;

void textConnListInit (textConnList_t *list, const char *name, void (*fini) (textConn_t *conn), textConnCmd_t *cmdtable);

typedef struct textbuf {
  char *data;
  int alloc_len;
  int len;
} textBuf_t;

void textBufInit (textBuf_t *b);
void textBufAddName (textBuf_t *b, char *name);
void textBufAddString (textBuf_t *b, char *str);
void textBufAddAsgnInt (textBuf_t *b, char *name, int val);
char *textBufFini (textBuf_t *b);


/* Semi-unrelated stuff */

typedef struct acceptor {
  int fd;
} acceptor_t;

const char *acceptorInitTCP (struct acceptor *a, void (*newconn) (acceptor_t *a, int fd), int port);
