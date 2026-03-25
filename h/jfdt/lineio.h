typedef struct lineio {
  jfdtFd_t fd;

  char *readbuf;
  int readpos;
  int readalloc;

  struct lineio_data {
    int len;
    struct lineio_data *next;
    char buf [1];
  } *writedata;
  int writepos;

  void (*proc) (struct lineio *io, char *data);

  void *userdata;
} lineIo_t;
