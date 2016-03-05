#include <stdio.h>
#include <stdarg.h>

#include "base.h"

int jfdtErrnoMap (int e) {
  // TODO: put the errno's string into a map,
  // and return that index, for later lookup.
  // ~ return jfdtErrorMap (strerror (e));
  return -10;
}

int jfdtErrorMap (const char *e) {
  // TODO: put the errno's string into a map,
  // and return that index, for later lookup.
  return -10;
}

const char *jfdtErrorString (int e) {
  // TODO: the lookup
  return "the-error";
}

void jfdtTraceF (const char *file, int line, const char *fmt, ...) {
  va_list ap;
  va_start (ap, fmt);
  fprintf (stderr, "%s:%d:", file, line);
  vfprintf (stderr, fmt, ap);
  fprintf (stderr, "\n");
  va_end (ap);
}

