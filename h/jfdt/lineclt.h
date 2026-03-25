#include "jfdt/base.h"

typedef struct lineclt {
  char *host;
  int port;
  lineIo_t io;
  union {
    timeout_t t;
    connect_t c;
  };

  void (*proc) (struct lineclt *clt, char *data);
  void (*stat) (struct lineclt *clt, char *err);

  void *userdata;
} lineClt_t;
