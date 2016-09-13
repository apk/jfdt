
#ifndef jfdt_base_h
#define jfdt_base_h

#include <sys/time.h>

extern int jfdtDebugLevel;

void jfdtServe (void);

void jfdtTraceF (const char *file, int line, const char *fmt, ...);

#define jfdt_trace(fmt...) (jfdtTraceF (__FILE__, __LINE__, fmt))

typedef struct jfdt_fd {
  struct jfdt_fd *next;
  int fd;
  int flags;
  void (*inhdl) (struct jfdt_fd *);
  void (*outhdl) (struct jfdt_fd *);
  void *userdata;
} jfdtFd_t;

void jfdtFdInit (
  jfdtFd_t *fd, int desc,
  void (*inhdl) (jfdtFd_t *),
  void (*outhdl) (jfdtFd_t *),
  void *userdata);

void jfdtFdReqIn (jfdtFd_t *);
void jfdtFdReqOut (jfdtFd_t *);
int jfdtFdWrite (jfdtFd_t *, void *, int);
int jfdtFdRead (jfdtFd_t *, void *, int);
void jfdtFdFini (jfdtFd_t *); /* Deactivate completely */
void jfdtFdClose (jfdtFd_t *); /* Fini, and close the fd */
void jfdtFdShutdown (jfdtFd_t *); /* Shutdown output direction */

typedef struct jfdt_listener {
  jfdtFd_t fd;
  void (*acpt) (struct jfdt_listener *, int fd, void *, int);
  void *userdata;
} jfdtListener_t;

int jfdtListenerCreateTcp (
  struct jfdt_listener *lstn,
  void (*acpt) (struct jfdt_listener *, int fd, void *, int),
  void *ud,
  int port);

void jfdtCloseFd (int);

typedef struct timeval jfdtTime_t;

extern int jfdtTimeLessThan (jfdtTime_t, jfdtTime_t);
extern void jfdtTimeAddFrac (jfdtTime_t *, int n, int d);
extern void jfdtTimeAddSecs (jfdtTime_t *, int s);
extern void jfdtTimeSub (jfdtTime_t *, jfdtTime_t);

typedef struct jfdt_timer {
  struct jfdt_timer *next;
  struct timeval tm;
  void (*f) (struct jfdt_timer *t, jfdtTime_t tm);
  void *userdata;
} jfdtTimer_t;

extern jfdtTime_t jfdtGetTime ();
extern void jfdtTimerInit (jfdtTimer_t *,
			   void (*fire) (jfdtTimer_t *, jfdtTime_t now),
			   void *userdata);
extern void jfdtTimerSet (jfdtTimer_t *, jfdtTime_t);
extern void jfdtTimerUnset (jfdtTimer_t *);

int jfdtErrnoMap (int n);
int jfdtErrorMap (const char *t);
const char *jfdtErrorString (int e);

#endif
