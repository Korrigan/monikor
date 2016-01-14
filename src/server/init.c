#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include "server.h"
#include "io_handler.h"

static int monikor_server_bind(monikor_server_t *server) {
  struct sockaddr_un addr;

  addr.sun_family = AF_UNIX;
  if (!server->mon->config->unix_sock_path
  || strlen(server->mon->config->unix_sock_path) >= sizeof(addr.sun_path))
    return -1;
  strcpy(addr.sun_path, server->mon->config->unix_sock_path);
  unlink(server->mon->config->unix_sock_path);
  if ((server->socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1
  || bind(server->socket, (struct sockaddr *)&addr, sizeof(addr))
  || chmod(server->mon->config->unix_sock_path, 0666)
  || listen(server->socket, MONIKOR_SRV_MAX_CLIENTS))
    return -1;
  monikor_log(LOG_INFO, "Monikor agent listening on socket %s\n",
    server->mon->config->unix_sock_path);
  return 0;
}

monikor_io_handler_t *monikor_server_handler_new(monikor_server_t *server, monikor_client_t *client) {
  monikor_server_handler_t *srv_handler;
  monikor_io_handler_t *handler;

  if (!(srv_handler = malloc(sizeof(*srv_handler))))
    return NULL;
  handler = monikor_io_handler_new(client ? client->socket : server->socket,
    MONIKOR_IO_HANDLER_RD,
    &monikor_server_handle_any, srv_handler);
  if (!handler) {
    free(srv_handler);
    return NULL;
  }
  srv_handler->server = server;
  srv_handler->client = client;
  srv_handler->handler = handler;
  return handler;
}

void monikor_server_handler_free(monikor_io_handler_t *handler) {
  if (!handler)
    return;
  free(handler->data);
  free(handler);
}

void monikor_client_init(monikor_client_t *client) {
  client->header.version = 0;
  client->header.count = 0;
  client->header.data_size = 0;
  client->data = NULL;
  client->socket = MONIKOR_CLIENT_INACTIVE_SOCKET;
}

int monikor_server_init(monikor_server_t *server, monikor_t *mon) {
  monikor_io_handler_t *handler = NULL;

  server->mon = mon;
  for (size_t i = 0; i < MONIKOR_SRV_MAX_CLIENTS; i++) {
    monikor_client_init(&server->clients[i]);
  }
  server->n_clients = 0;
  if (monikor_server_bind(server) ||
  !(handler = monikor_server_handler_new(server, NULL))) {
    monikor_server_handler_free(handler);
    if (handler)
      close(server->socket);
    return -1;
  }
  monikor_register_io_handler(mon, handler);
  return 0;
}
