#include <jfdt/exec.h>
#include <stdlib.h>

static jfdtTimer_t tim;
static jfdtExec_t exec;

static void set_timer () {
  jfdtTime_t t = jfdtGetTime ();
  jfdtTimeAddSecs (&t, 1);
  jfdtTimerSet (&tim, t);
}

static void term (jfdtExec_t *exe, int status) {
  jfdt_trace ("status %x", status);
  set_timer ();
}

static char *args [] = {
  "/usr/bin/ruby", 
  "-e",
  "puts rand(34**10).to_s(34); sleep 0.5; puts '-'",
  0
};

static void fire (jfdtTimer_t *to, jfdtTime_t now) {
  int r = jfdtExecDo (&exec, term, 0, 0, args, 0, 0, 0);
  if (r == -1) {
    jfdt_trace ("oops");
    exit (1);
  }
  jfdt_trace ("pid: %d", r);
}

int main (int argc, char **argv) {
  jfdtTimerInit (&tim, fire, 0);
  set_timer ();
  jfdtServe ();
  return 0;
}
