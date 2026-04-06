#include "base.h"

typedef struct jfdt_lineio {
  jfdtFd_t fd;

  char *readbuf;
  int readpos;
  int readalloc;

  struct jfdt_lineio_data {
    int len;
    struct jfdt_lineio_data *next;
    char buf [1];
  } *writedata;
  int writepos;

  void (*proc) (struct jfdt_lineio *io, char *data);
  void (*err) (struct jfdt_lineio *io, const char *msg);

  void *userdata;
} jfdtLineIo_t;

void jfdtLineIoInit (jfdtLineIo_t *io, int fd,
		     void (*proc) (jfdtLineIo_t *io, char *data),
		     void (*err) (jfdtLineIo_t *io, const char *msg),
		     void *userdata);
void jfdtLineIoFini (jfdtLineIo_t *io);

void jfdtLineIoSend (jfdtLineIo_t *io, const char *msg);
