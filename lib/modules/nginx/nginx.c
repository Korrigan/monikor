#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "monikor.h"
#include "nginx.h"


void *nginx_init(monikor_t *mon, monikor_config_dict_t *config) {
  (void)mon;
  char *url;

  (void)mon;
  url = monikor_config_dict_get_scalar(config, "nginx.url");
  return url ? url : NGINX_DEFAULT_STATUS_URL;
}

void nginx_cleanup(monikor_t *mon, void *data) {
  (void)mon;
  (void)data;
}

int nginx_poll(monikor_t *mon, void *data) {
  if (monikor_http_get(mon, (char *)data, NGINX_TIMEOUT, &nginx_poll_metrics, (void *)mon))
    return -1;
  return 0;
}
