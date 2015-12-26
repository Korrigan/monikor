#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>

#include "logger.h"

static const char *priority_message[] = {
    "EMERG",
    "ALERT",
    "CRITICAL",
    "ERROR",
    "WARNING",
    "NOTICE",
    "INFO",
    "DEBUG"
};

static int _monikor_log_level = LOG_INFO;


void monikor_logger_init(int prio) {
  if (prio != MONIKOR_LOG_DEFAULT)
    _monikor_log_level = prio;
}


void monikor_logger_cleanup(void) {
}


int monikor_vlog(int prio, const char *message, va_list ap) {
  char *full_msg;
  size_t full_msg_len;
  int ret;

  if (prio == MONIKOR_LOG_DEFAULT)
    prio = _monikor_log_level;
  if (prio < MONIKOR_LOG_MIN_PRIO || prio > MONIKOR_LOG_MAX_PRIO)
    return -1;
  if (prio > _monikor_log_level)
    return 0;
  full_msg_len = strlen(priority_message[prio]) + strlen(MONIKOR_LOG_SEP) + strlen(message);
  if (!(full_msg = malloc(full_msg_len + 1)))
    return -1;
  strcpy(full_msg, priority_message[prio]);
  strcat(full_msg, MONIKOR_LOG_SEP);
  strcat(full_msg, message);
  ret = vprintf(full_msg, ap);
  free(full_msg);
  return ret;
}


int monikor_log(int prio, const char *message, ...) {
  int ret;
  va_list ap;

  va_start(ap, message);
  ret = monikor_vlog(prio, message, ap);
  va_end(ap);
  return ret;
}


static char *_format_mod_log_message(const char *mod_name, const char *message) {
  char *full_msg;

  if (!(full_msg = malloc(strlen(mod_name) + strlen(MONIKOR_LOG_SEP) + strlen(message) + 1)))
    return NULL;
  strcpy(full_msg, mod_name);
  strcat(full_msg, MONIKOR_LOG_SEP);
  strcat(full_msg, message);
  return full_msg;
}


int monikor_vlog_mod(int prio, const char *mod_name, const char *message, va_list ap) {
  int ret;
  char *full_msg;

  if (!(full_msg = _format_mod_log_message(mod_name, message)))
    return -1;
  ret = monikor_vlog(prio, full_msg, ap);
  free(full_msg);
  return ret;
}


int monikor_log_mod(int prio, const char *mod_name, const char *message, ...) {
  int ret;
  va_list ap;
  char *full_msg;

  va_start(ap, message);
  if (!(full_msg = _format_mod_log_message(mod_name, message)))
    return -1;
  ret = monikor_vlog(prio, full_msg, ap);
  va_end(ap);
  free(full_msg);
  return ret;
}
