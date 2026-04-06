#include "textconn.h"
#include "textbuf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void textBufInit (textBuf_t *b) {
  b->alloc_len = 80;
  b->data = malloc (b->alloc_len);
  b->len = 0;
  b->data [b->len] = 0;
}

static void make_place (textBuf_t *b, int ex) {
  if (b->len + ex + 2 > b->alloc_len) {
    int nl = 2 * b->alloc_len / 3 + 10;
    b->data = realloc (b->data, nl);
    b->alloc_len = nl;
  }
}

static char hexdig [] = "0123456789abcdef";

void textBufAddName (textBuf_t *b, const char *str) {
  int cnt = 0;
  make_place (b, 3 * strlen (str));
  if (b->len > 0) b->data [b->len ++] = ' ';
  while (*str) {
    int c = *str ++;
    /* Very restrictive for now */
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || (cnt > 0 && c >= '0' && c <= '9')) {
      b->data [b->len ++] = c;
    } else {
      b->data [b->len ++] = '`';
      b->data [b->len ++] = hexdig [(c >> 4) & 15];
      b->data [b->len ++] = hexdig [c & 15];
    }
    cnt ++;
  }
  b->data [b->len] = 0;
}

void textBufAddString (textBuf_t *b, const char *str) {
  make_place (b, 3 * strlen (str) + 2);
  if (b->len > 0) b->data [b->len ++] = ' ';
  b->data [b->len ++] = '"';
  while (*str) {
    int c = *str ++;
    /* Restrictive for now */
    if (c >= ' ' && c <= '~' && c != '"' && c != '`') {
      b->data [b->len ++] = c;
    } else {
      b->data [b->len ++] = '`';
      b->data [b->len ++] = hexdig [(c >> 4) & 15];
      b->data [b->len ++] = hexdig [c & 15];
    }
  }
  b->data [b->len ++] = '"';
  b->data [b->len] = 0;
}

void textBufAddLabelInt (textBuf_t *b, const char *name, int val) {
  textBufAddName (b, name);
  make_place (b, 30);
  sprintf (b->data + b->len, ": %d", val);
  b->len = strlen (b->data);
}

char *textBufFini (textBuf_t *b) {
  b->data [b->len] = 0;
  return b->data;
}

void textScanInit (textScan_t *s, const char *txt) {
  s->str = txt;
}

static int isspc (char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static int isspx (char c) {
  return isspc (c) || c == 0;
}

static const char *ignb (const char *s) {
  while (isspc (*s)) s ++;
  return s;
}

static int isal (char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

struct uniq {
  struct uniq *next;
  char buf [1];
} *uniqlist = 0;

static const char *uniqq (const char *b, const char *e) {
  struct uniq *u;
  for (u = uniqlist; u; u = u->next) {
    if (strlen (u->buf) == e - b &&
	!strncmp (u->buf, b, e - b)) {
      return u->buf;
    }
  }
  u = malloc (sizeof (struct uniq) + (e - b));
  u->next = uniqlist;
  uniqlist = u;
  strncpy (u->buf, b, e - b);
  u->buf [e - b] = 0;
  return u->buf;
}

int textScanIsName (textScan_t *s, const char *n) {
  const char *p = ignb (s->str);
  const char *b = p;
  while (*n) {
    if (*n != *p) {
      return 0;
    }
    n ++;
    p ++;
  }
  if (isspx (*p)) {
    s->str = p;
    return 1;
  }
  return 0;
}

const char *textScanGetName (textScan_t *s) {
  const char *p = ignb (s->str);
  const char *b = p;
  if (isal (*p)) {
    p ++;
    while (isal (*p) || (*p >= '0' && *p <= '9') || *p == '_' || *p == '-' || *p == '/') p ++;
    if (isspx (*p)) {
      s->str = p;
      return uniqq (b, p);
    }
  }
  return 0;
}

const char *textScanGetLabel (textScan_t *s) {
  const char *p = ignb (s->str);
  const char *b = p;
  if (isal (*p)) {
    p ++;
    while (isal (*p) || (*p >= '0' && *p <= '9') || *p == '_' || *p == '-' || *p == '/') p ++;
    if (*p == ':') {
      s->str = p + 1;
      return uniqq (b, p);
    }
  }
  return 0;
}

int textScanIsEnd (textScan_t *s) {
  const char *p = ignb (s->str);
return *p == 0;
}

int textScanGetInt (textScan_t *s, int *v) {
  const char *p = ignb (s->str);
  char *e = p;
  long r = strtol (p, &e, 10);
  if (e > p && isspx (*e)) {
    *v = r;
    s->str = e;
    return 1;
  }
  return 0;
}

char *textScanGetStr (textScan_t *s) {
  textBuf_t B;
  const char *p = ignb (s->str);
  textBufInit (&B);
  if (*p ++ != '"') return 0;
  while (*p) {
    // TODO: Handle ` sequences - or any escapes?
    if (*p == '"') {
      s->str = p + 1;
      return textBufFini (&B);
    }
    make_place (&B, 1);
    B.data [B.len ++] = *p ++;
  }
  free (textBufFini (&B));
  return 0;
}
