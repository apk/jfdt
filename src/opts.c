char *jfdtOptsIsPrefix (char *val, char *prefix) {
  while (*prefix) {
    if (*prefix ++ != *val ++) return 0;
  }
  return val;
}

#define isd(x) ((x) >= '0' && (x) <= '9') /* Assumes ASCII or EBCDIC */
#define dvl(x) ((x) - '0')

char *jfdtOptsParseNat (char *val, int *res) {
  char *beg = val;
  int v = 0;
  while (isd (*val)) {
    v = 10 * v + dvl (*val ++);
  }
  if (val > beg) {
    *res = v;
    return val;
  }
  return 0;
}

char *jfdtOptsParseTime (char *val, int *res) {
  char *beg = val;
  int tval = 0;
  while (isd (*val)) {
    int v = 0;
    while (isd (*val)) {
      v = 10 * v + dvl (*val ++);
    }
    switch (*val) {
    case 'h':
      tval += 3600 * v;
      break;
    case 'm':
      tval += 60 * v;
      break;
    case 's':
      tval += v;
      break;
    default:
      return 0;
    }
  }
  if (val > beg) {
    *res = tval;
    return val;
  }
  return 0;
}
