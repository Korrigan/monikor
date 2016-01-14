#ifndef MONIKOR_CONFIG_H_
# define MONIKOR_CONFIG_H_

#include <stdio.h>
#include <yaml.h>

#include "strl.h"

#define MONIKOR_DEFAULT_CONFIG_PATH "/etc/monikor/monikor.yml"
#define MONIKOR_DEFAULT_MODULES_PATH "/usr/lib/monikor/modules"
#define MONIKOR_DEFAULT_POLL_INTERVAL 60
#define MONIKOR_DEFAULT_SERVER_TIMEOUT 5
#define MONIKOR_QUANTA_TOKEN_MAX_LENGTH 42
#define MONIKOR_HOSTID_MAX_LENGTH 42


typedef enum {
  DICT,
  LIST,
  SCALAR
} monikor_config_elem_type_t;


typedef struct monikor_config_dict_s {
  char *key;
  monikor_config_elem_type_t type;
  union {
    struct monikor_config_dict_s *dict;
    strl_t *list;
    char *value;
  } value;
  struct monikor_config_dict_s *next;
} monikor_config_dict_t;


typedef struct {
  monikor_config_dict_t *full_config;
  char *config_path;

  struct {
    char *path;
    strl_t *modules;
  } modules;

  char *hostid;
  char *quanta_token;

  char *server_url;
  int server_timeout;

  char *unix_sock_path;

  int log_level;
  int poll_interval;

} monikor_config_t;


monikor_config_t *monikor_config_new(void);
void monikor_config_free(monikor_config_t *config);
monikor_config_t *monikor_load_config(const char *config_path);
int monikor_setup_config(monikor_config_t *config);

monikor_config_dict_t *monikor_config_dict_new(void);
void monikor_config_dict_free(monikor_config_dict_t *dict);
char *monikor_config_dict_get_scalar(monikor_config_dict_t *dict, const char *key);
strl_t *monikor_config_dict_get_list(monikor_config_dict_t *dict, const char *key);
monikor_config_dict_t *monikor_config_dict_get_dict(monikor_config_dict_t *dict, const char *key);

int monikor_parse_config_file(FILE *fh, monikor_config_t *cfg);
int config_parse_key(yaml_parser_t *parser, const char *key, monikor_config_dict_t *dict);
int config_parse_mapping(yaml_parser_t *parser, monikor_config_dict_t *dict);
int config_parse_list(yaml_parser_t *parser, strl_t *list);


#endif /* end of include guard: MONIKOR_CONFIG_H_ */
