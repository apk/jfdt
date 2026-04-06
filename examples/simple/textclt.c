#include <stdio.h>

#include <jfdt/lineclt.h>

static void io_proc (jfdtLineClt_t *clt, const char *data) {
  printf ("prod(%s)\n", data ? data : "<ok>");
}

static void io_stat (jfdtLineClt_t *clt, const char *err) {
  printf ("stat(%s)\n", err ? err : "<ok>");
  if (!err) {
    jfdtLineCltSend (clt, "sig \"A\" \"B\"");
  }
}

int main (int argc, char **argv) {
  jfdtLineClt_t clt;

  jfdtLineCltInit (&clt, "localhost", 8430,
		   3000,
		   io_proc,
		   io_stat,
		   0);
  jfdtServe ();
}
