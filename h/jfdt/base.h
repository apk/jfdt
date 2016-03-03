
#include <sys/time.h>

void jdftServe (void);

typedef struct jfdt_fd {
  int fd;
  void *userdata;
} jfdtFd_t;

typedef struct jdft_listener {
  jfdtFd_t fd;
  void (*acpt) (struct jfdt_listener *, int fd, void *, int);
} jfdtListener_t;

int jfdtListenerCreateTcp (
  struct jfdt_listener *lstn,
  void (*acpt) (struct jfdt_listener *, int fd, void *, int),
  void *ud,
  int port);

void jfdtCloseFd (int);


typedef struct jfdt_timeout {
  struct timeval tm;
} jfdtTimer_t;

typedef struct timeval jfdtTime_t;

extern jfdtTime_t jfdtGetTime ();
