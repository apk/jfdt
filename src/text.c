#include "textconn.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

void textBufInit (textBuf_t *b) {
  b->alloc_len = 80;
  b->data = malloc (b->alloc_len);
  b->len = 0;
  b->data [b->len] = 0;
}

static make_place (textBuf_t *b, int ex) {
  if (b->len + ex + 2 > b->alloc_len) {
    int nl = 2 * b->alloc_len / 3 + 10;
    b->data = realloc (b->data, nl);
    b->alloc_len = nl;
  }
}

static char hexdig [] = "0123456789abcdef";

void textBufAddName (textBuf_t *b, char *str) {
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

void textBufAddString (textBuf_t *b, char *str) {
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

void textBufAddAsgnInt (textBuf_t *b, char *name, int val) {
  textBufAddName (b, name);
  make_place (b, 30);
  sprintf (b->data + b->len, "=%d", val);
  b->len = strlen (b->data);
}

char *textBufFini (textBuf_t *b) {
  return b->data;
}

void textScanInit (textScan_t *s, const char *txt) {
  s->str = txt;
}

static const char *ignb (const char *s) {
  while (*s == ' ' || *s == '\t') s ++;
  return s;
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

const char *textScanGetName (textScan_t *s) {
  const char *p = ignb (s->str);
  const char *b = p;
  while ((*p >= 'a' && *p <= 'z') || *p == '_') p ++;
  if (p > b) {
    s->str = p;
    return uniqq (b, p);
  }
  return 0;
}

char *textScanGetStr (textScan_t *s) {
  textBuf_t B;
  const char *p = ignb (s->str);
  textBufInit (&B);
  if (*p ++ != '"') return 0;
  while (*p) {
    // TODO: Handle ` sequences
    if (*p == '"') {
      s->str = p + 1;
      return textBufFini (&B);
    }
    make_place (&B, 1);
    B.data [B.len ++] = *p ++;
    printf ("C(%c)\n", p [-1]);
  }
  free (textBufFini (&B));
  return 0;
}
