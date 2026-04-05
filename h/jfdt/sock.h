#include "jfdt/base.h"

int jfdtOpenTcp (const char *host, int port);

typedef struct {
  int v6;
  unsigned char addr [16];
} jfdtSockAddrIn_t;
