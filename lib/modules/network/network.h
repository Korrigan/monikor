#ifndef network_H_
#define network_H_

#include "monikor.h"

#define MOD_NAME "network"
#define MNK_NET_MAX_DEV_LEN 10

typedef struct {
  strl_t *ifaces;
} monikor_net_mod_t;

int poll_network_metrics(monikor_t *mon, struct timeval *clock, monikor_net_mod_t *mod);
int poll_tcp_metrics(monikor_t *mon, struct timeval *clock);

#endif /* end of include guard: network_H_ */
