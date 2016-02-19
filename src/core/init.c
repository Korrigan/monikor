#include <sys/time.h>
#include <curl/curl.h>

#include "monikor.h"


int monikor_init(monikor_t *mon, char *config_path) {
  size_t mod_size;

  mon->modules.count = 0;
  monikor_logger_init(MONIKOR_LOG_DEFAULT);
  if (!(mon->config = monikor_load_config(config_path)))
    return 1;
  monikor_logger_init(mon->config->log_level);
  curl_global_init(CURL_GLOBAL_ALL);
  monikor_io_handler_list_init(&mon->io_handlers);
  if (monikor_register_signals(mon)) {
    monikor_log(LOG_CRIT, "Cannot register signal handlers\n");
    return 1;
  }
  mod_size = mon->config->modules.modules->size * sizeof(*mon->modules.modules);
  mon->modules.count = 0;
  if (!(mon->modules.modules = malloc(mod_size))
  || !(mon->metrics = monikor_metric_store_new())) {
    monikor_log(LOG_CRIT, "Cannot allocate memory\n");
    return 1;
  }
  if (monikor_load_all_modules(mon))
    return 1;
  if (mon->config->unix_sock_path)
    monikor_server_init(&mon->server, mon);
  gettimeofday(&mon->last_clock, NULL);
  mon->flags = 0;
  return 0;
}
