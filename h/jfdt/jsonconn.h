#include "jfdt/base.h"
#include "jfdt/json.h"

typedef struct jfdt_jsonconn {
  jfdtFd_t fd;
  struct jdft_jsonconn *next;
  void *userdata;
} jfdtJsonConn_t;

typedef struct nux_jsonconnlist {
  struct jdft_jsonconn *list;
} jfdtJsonConnList_t;

typedef struct jfdt_jsonlstn {
  jfdtFd_t fd;
} jfdtJsonListener_t;
