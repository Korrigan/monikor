#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "monikor.h"
#include "redis.h"


void *redis_init(monikor_t *mon, monikor_config_dict_t *config) {
  redis_module_t *mod;

  (void)mon;
  if (!(mod = malloc(sizeof(*mod))))
    return NULL;
  mod->mon = mon;
  mod->io_handler = NULL;
  mod->host = monikor_config_dict_get_scalar(config, "redis.host");
  mod->port = monikor_config_dict_get_scalar(config, "redis.port");
  if (!mod->host)
    mod->host = MONIKOR_REDIS_DEFAULT_HOST;
  if (!mod->port)
    mod->port = MONIKOR_REDIS_DEFAULT_PORT;
  return mod;
}

void redis_cleanup(monikor_t *mon, void *data) {
  redis_module_t *mod = (redis_module_t *)data;

  if (mod->io_handler) {
    monikor_unregister_io_handler(mon, mod->io_handler);
    if (mod->io_handler->fd != -1)
      close(mod->io_handler->fd);
    free(mod->io_handler->data);
    free(mod->io_handler);
  }
  free(mod);
}

int redis_poll(monikor_t *mon, void *data) {
  redis_module_t *mod = (redis_module_t *)data;

  if (!mod)
    return -1;
  if (mod->io_handler) {
    monikor_unregister_io_handler(mon, mod->io_handler);
    if (mod->io_handler->fd != -1)
      close(mod->io_handler->fd);
    free(mod->io_handler);
    mod->io_handler = NULL;
  }
  mod->io_handler = monikor_net_exchange(mod->host, mod->port, "INFO\r\n",
    &redis_poll_metrics, (void *)mod);
  if (!mod->io_handler) {
    monikor_log_mod(LOG_ERR, MOD_NAME, "Cannot connect to %s:%s: %s\n",
      mod->host, mod->port, strerror(errno));
    return -1;
  }
  monikor_register_io_handler(mon, mod->io_handler);
  return 0;
}
