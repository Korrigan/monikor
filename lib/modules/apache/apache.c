#include <stdlib.h>

#include "monikor.h"
#include "apache.h"


void apache_init(void) {
}

void apache_cleanup(void) {
}

monikor_metric_list_t *apache_poll(void) {
  monikor_metric_list_t *metrics;
  struct timeval now;

  gettimeofday(&now, NULL);
  if (!(metrics = monikor_metric_list_new()))
    return NULL;
  if (!apache_poll_metrics(metrics, &now)) {
    monikor_metric_list_free(metrics);
    return NULL;
  }
  return metrics;
}
