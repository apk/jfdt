#include "textconn.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void textBufInit (textBuf_t *b) {
  b->alloc_len = 80;
  b->data = malloc (b->alloc_len);
  b->len = 0;
  b->data [b->len] = 0;
}

void textBufAddName (textBuf_t *b, char *name) {
  textBufAddString (b, name);
}

static make_place (textBuf_t *b, int ex) {
  if (b->len + ex + 2 > b->alloc_len) {
    int nl = 2 * b->alloc_len / 3 + 10;
    b->data = realloc (b->data, nl);
    b->alloc_len = nl;
  }
}

static char hexdig [] = "0123456789abcdef";

void textBufAddString (textBuf_t *b, char *str) {
  make_place (b, 3 * strlen (str));
  if (b->len > 0) b->data [b->len ++] = ' ';
  while (*str) {
    int c = *str ++;
    /* Very restrictive for now */
    if ((c >= 'a' && c <= 'z') || c == '_') {
      b->data [b->len ++] = c;
    } else {
      b->data [b->len ++] = '%';
      b->data [b->len ++] = hexdig [(c >> 4) & 15];
      b->data [b->len ++] = hexdig [c & 15];
    }
  }
  b->data [b->len] = 0;
}

void textBufAddAsgnInt (textBuf_t *b, char *name, int val) {
  textBufAddString (b, name);
  make_place (b, 30);
  sprintf ("=%d", b->data + b->len, val);
  b->len = strlen (b->data);
}

char *textBufFini (textBuf_t *b) {
  return b->data;
}
