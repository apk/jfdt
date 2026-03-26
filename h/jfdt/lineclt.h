#include "jfdt/base.h"

typedef struct jfdt_lineclt {
  char *host;
  int port;
  jfdtLineIo_t io;
  union {
    timeout_t t;
    connect_t c;
  };

  void (*proc) (struct jfdt_lineclt *clt, char *data);
  void (*stat) (struct jfdt_lineclt *clt, char *err);

  void *userdata;
} jfdtLineClt_t;
