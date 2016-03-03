
typedef struct json {
  int typref;
#define json_T_nil 1
#define json_T_num 2
#define json_T_str 3
#define json_T_obj 4
#define json_T_arr 5
  union {
    double d;
    char *s;
    struct json_ent {
      struct json_ent *next;
      char *name;
      struct json *val;
    } *g; /* subobject, or zero-terminated array? */
    struct jsonarr {
      int len;
      int alloc_len;
      struct json *val [1];
    } *a;
  } u;
} json_t;

json_t *jsonObj ();
void jsonSetInt (json_t *b, char *n, int v);
void jsonSetString (json_t *b, char *n, char *v);
void jsonSetNil (json_t *b, char *n);
