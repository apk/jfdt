
typedef struct jfdt_textbuf {
  char *data;
  int alloc_len;
  int len;
} textBuf_t;

void textBufInit (textBuf_t *b);
void textBufAddName (textBuf_t *b, const char *name);
void textBufAddString (textBuf_t *b, const char *str);
void textBufAddLabelInt (textBuf_t *b, const char *name, int val);
char *textBufFini (textBuf_t *b);

typedef struct jfdt_textscan {
  const char *str;
} textScan_t;

void textScanInit (textScan_t *s, const char *txt);
int textScanIsName (textScan_t *s, const char *n);
const char *textScanGetName (textScan_t *s);
char *textScanGetStr (textScan_t *s);
const char *textScanGetLabel (textScan_t *s);
int textScanGetInt (textScan_t *s, int *v);
int textScanIsEnd (textScan_t *s);
