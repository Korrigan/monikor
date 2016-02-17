#ifndef redis_H_
#define redis_H_

#include "monikor.h"

#define MOD_NAME "redis"
#define MONIKOR_REDIS_DEFAULT_HOST "localhost"
#define MONIKOR_REDIS_DEFAULT_PORT "6379"

typedef struct {
  char *host;
  char *port;
  monikor_t *mon;
  monikor_io_handler_t *io_handler;
} redis_module_t;

void redis_poll_metrics(char *status, void *data);

#endif /* end of include guard: redis_H_ */
