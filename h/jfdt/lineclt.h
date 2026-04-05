#include "base.h"
#include "lineio.h"

typedef struct jfdt_lineclt {
  const char *host;
  int port;
  union {
    jfdtTimer_t t;
    jfdtLineIo_t io;
    /* connect_t c;  / * for async connect */
  };

  enum { jfdtLineClt_waiting, jfdtLineClt_connected } state;

  void (*proc) (struct jfdt_lineclt *clt, char *data);
  void (*stat) (struct jfdt_lineclt *clt, char *err);

  void *userdata;
} jfdtLineClt_t;

void jfdtLineCltInit (jfdtLineClt_t *clt, const char *host, int port,
                      void (*proc) (jfdtLineClt_t *clt, char *data),
                      void (*stat) (jfdtLineClt_t *clt, char *err),
                      void *userdata);
void jfdtLineCltFini (jfdtLineClt_t *clt);
void jfdtLineCltSend (jfdtLineClt_t *clt, const char *msg);
