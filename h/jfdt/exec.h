#include "base.h"

typedef struct jfdt_exec {
  struct jfdt_exec *next;
  int pid;
  void (*term) (struct jfdt_exec *, int status);
  void *userdata;
} jfdtExec_t;

int jfdtExecDo (jfdtExec_t *,
		void (*term) (jfdtExec_t *exe, int status),
		void (*inter) (jfdtExec_t *exe, void *xud),
		char *prog,
		char **argv,
		char **env,
		void *userdata,
		void *xud);

void jfdtExecFini (jfdtExec_t *);

void jfdtExecSetStrayHandler (void (*f) (int pid, int status));

void jfdtExecAddAsyncHandler (void (*f) (void));
void jfdtExecTriggerAsync (void);
