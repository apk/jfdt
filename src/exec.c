#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "exec.h"

extern char **environ;

static int fdpair [2] = { -1, -1 };

static jfdtExec_t *execlist = 0;

static void setnb (int fd) {
  fcntl (fd, F_SETFL, fcntl (fd, F_GETFL) | O_NONBLOCK);
}

static void sigchld (int n) {
  int save_errno = errno;
  //(void)write (fdpair [1], "", 1);
  (void)write (fdpair [1], "", 1);
  errno = save_errno;
}

static void fdhdl (jfdtFd_t *fd) {
  char dummy [64];
  jfdtFdReqIn (fd);
  jfdt_trace ("fdhdl...");
  while (read (fdpair [0], dummy, sizeof (dummy)) > 0);
  while (1) {
    jfdtExec_t *exe, **pp;
    int status;
    int r = waitpid (-1, &status, WNOHANG);
    if (r == -1) break;
    jfdt_trace ("pid: %d", r);
    for (pp = &execlist; ; pp = &exe->next) {
      if (!(exe = *pp)) {
	jfdt_trace ("stray wait: %d %x", r, status);
	break;
      }
      if (exe->pid == r) {
	*pp = exe->next;
	exe->term (exe, status);
	break;
      }
    }
  }
}

int jfdtExecDo (jfdtExec_t *exe,
		void (*term) (jfdtExec_t *exe, int status),
		void (*inter) (jfdtExec_t *exe, void *xud),
		char *prog,
		char **argv,
		char **env,
		void *userdata,
		void *xud) {
  int r;
  jfdt_trace ("pair - %d, %d", fdpair [0], fdpair [1]);
  if (fdpair [0] == -1) {
    static struct sigaction act;
    static jfdtFd_t fd;
    if (pipe (fdpair) == -1) {
      return -1;
    }
    jfdt_trace ("pair + %d, %d", fdpair [0], fdpair [1]);
    setnb (fdpair [0]);
    setnb (fdpair [1]);
    memset (&act, 0, sizeof (act));
    act.sa_handler = sigchld;
    sigaction (SIGCHLD, &act, 0);
    jfdtFdInit (&fd, fdpair [0], fdhdl, 0, 0);
    jfdtFdReqIn (&fd);
  }
  r = fork ();
  if (r == 0) {
    /* Child */
    if (inter) {
      inter (exe, xud);
    }
    if (!env) {
	env = environ;
    }
    if (!prog) {
      prog = argv [0];
    }
    execve (prog, argv,  env);
    _exit (1);
  } else if (r > 0) {
    /* Parent */
    exe->pid = r;
    exe->term = term;
    exe->userdata = userdata;
    exe->next = execlist;
    execlist = exe;
    return r;
  } else {
    return r;
  }
}

void jfdtExecFini (jfdtExec_t *);
