#include <sys/wait.h>
#include <sys/time.h>

#include "monikor.h"
#include "varnish.h"

static const struct {
  char *varnish_name;
  char *name;
  uint8_t flags;
} metrics[] = {
  {"client_req", "varnish.requests_per_second", MONIKOR_METRIC_TIMEDELTA},
  {"s_pipe", "varnish.requests.pipe", MONIKOR_METRIC_TIMEDELTA},
  {"s_pass", "varnish.requests.pass", MONIKOR_METRIC_TIMEDELTA},
  {"s_resp_hdrbytes", "varnish.bytes.headers", MONIKOR_METRIC_TIMEDELTA},
  {"s_resp_bodybytes", "varnish.bytes.body", MONIKOR_METRIC_TIMEDELTA},

  {"cache_hit", "varnish.cache.hits", MONIKOR_METRIC_DELTA},
  {"cache_hitpass", "varnish.cache.hitpasses", MONIKOR_METRIC_DELTA},
  {"cache_miss", "varnish.cache.misses", MONIKOR_METRIC_DELTA},
  {"n_lru_nuked", "varnish.cache.evictions", MONIKOR_METRIC_DELTA},
  {"n_expired", "varnish.cache.expires", MONIKOR_METRIC_DELTA},

  {"sess_conn", "varnish.connections.total", MONIKOR_METRIC_TIMEDELTA},
  {"sess_drop", "varnish.connections.drop", MONIKOR_METRIC_TIMEDELTA},
  {"sess_fail", "varnish.connections.fail", MONIKOR_METRIC_TIMEDELTA},
  {"sess_closed", "varnish.connections.close", MONIKOR_METRIC_TIMEDELTA},
  {"sess_pipeline", "varnish.connections.pipeline", MONIKOR_METRIC_TIMEDELTA},
  {"sess_herd", "varnish.connections.herd", MONIKOR_METRIC_TIMEDELTA},
  {"sess_queued", "varnish.connections.queued", MONIKOR_METRIC_TIMEDELTA},

  {"bans", "varnish.bans.count", 0},

  {"n_object", "varnish.memory.objects", 0},
  {"g_bytes", "varnish.memory.used", 0},
  {"g_space", "varnish.memory.free", 0},
  {"c_fail", "varnish.memory.alloc_fails", MONIKOR_METRIC_DELTA},
  {"shm_records", "varnish.shm.records", MONIKOR_METRIC_TIMEDELTA},
  {"shm_writes", "varnish.shm.writes", MONIKOR_METRIC_TIMEDELTA},
  {"shm_flushes", "varnish.shm.flushes", MONIKOR_METRIC_TIMEDELTA},

  {"n_backend", "varnish.backends.count", 0},
  {"backend_req", "varnish.backends.requests_per_second", MONIKOR_METRIC_TIMEDELTA},
  {"backend_conn", "varnish.backends.connections.count", MONIKOR_METRIC_TIMEDELTA},
  {"backend_unhealthy", "varnish.backends.connections.unhealthy", MONIKOR_METRIC_TIMEDELTA},
  {"backend_busy", "varnish.backends.connections.busy", MONIKOR_METRIC_TIMEDELTA},
  {"backend_fail", "varnish.backends.connections.fail", MONIKOR_METRIC_TIMEDELTA},
  {"backend_reuse", "varnish.backends.connections.reuse", MONIKOR_METRIC_TIMEDELTA},
  {"backend_recycle", "varnish.backends.connections.recycle", MONIKOR_METRIC_TIMEDELTA},

  {"threads", "varnish.threads.count", 0},
  {"threads_failed", "varnish.threads.failed", MONIKOR_METRIC_DELTA},

  {"esi_errors", "varnish.esi.errors", MONIKOR_METRIC_DELTA},
  {"esi_warnings", "varnish.esi.warnings", MONIKOR_METRIC_DELTA},

  {"uptime", "varnish.uptime", 0},

  {NULL, NULL, 0}
};

#define VARNISH_SMA_TRANSIENT ".Transient."
#define VARNISH_SMA_TRANSIENT_LEN (sizeof(VARNISH_SMA_TRANSIENT) - 1)

size_t varnish_fetch_metrics(varnish_module_t *mod, const char *output) {
  size_t n_metrics = 0;
  char *found;
  char metric_name[256];
  struct timeval clock;

  gettimeofday(&clock, NULL);
  for (size_t i = 0; metrics[i].name; i++) {
    for (found = output; (found = strstr(found, metrics[i].varnish_name)); found += strlen(metrics[i].varnish_name)) {
      if (((found - VARNISH_SMA_TRANSIENT_LEN < output)
          || strncmp(found - VARNISH_SMA_TRANSIENT_LEN, VARNISH_SMA_TRANSIENT, VARNISH_SMA_TRANSIENT_LEN))
          && (isspace(found[strlen(metrics[i].varnish_name)]))) {
        char *endval;
        char *val = found + strlen(metrics[i].varnish_name);
        uint64_t value = strtoull(val, &endval, 10);
        if (val == endval)
          continue;
        if (mod->instance)
          sprintf(metric_name, "%s.%s", metrics[i].name, mod->instance);
        else
          sprintf(metric_name, "%s", metrics[i].name);
        n_metrics += monikor_metric_push(mod->mon, monikor_metric_integer(
          metric_name, &clock, value, metrics[i].flags));
        break;
      }
    }
  }

  return n_metrics;
}

void varnish_poll_metrics(command_exec_t *cmd) {
  varnish_module_t *mod = (varnish_module_t *)cmd->data;
  size_t n_metrics;

  if (monikor_command_failed(cmd->state)) {
    if (WIFEXITED(cmd->status)) {
      monikor_log_mod(LOG_WARNING, MOD_NAME,
        "Varnishstat failed: process exited with code %d\n", WEXITSTATUS(cmd->status));
    } else if (WIFSIGNALED(cmd->status)) {
      monikor_log_mod(LOG_WARNING, MOD_NAME,
        "Varnishstat failed: process killed with signal %d\n", WTERMSIG(cmd->status));
    } else {
      monikor_log_mod(LOG_WARNING, MOD_NAME, "Varnishstat failed");
    }
  } else {
    n_metrics = varnish_fetch_metrics(mod, cmd->output);
    int level = n_metrics > 0 ? LOG_DEBUG : LOG_WARNING;
    monikor_log_mod(level, MOD_NAME, "Got %d varnish metrics\n", n_metrics);
  }
}
