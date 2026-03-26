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

  void *userdata;
} jfdtLineIo_t;
