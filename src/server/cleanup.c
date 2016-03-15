#include <stdlib.h>
#include <unistd.h>

#include "server.h"
#include "io_handler.h"


void monikor_server_handler_free(monikor_io_handler_t *handler) {
  if (!handler)
    return;
  free(handler->data);
  free(handler);
}

void monikor_server_cleanup(monikor_server_t *server) {
  if (!server)
    return;
  if (server->handler) {
    monikor_unregister_io_handler(server->mon, server->handler);
    monikor_server_handler_free(server->handler);
  }
  if (server->socket != -1)
    close(server->socket);
  if (server->mon->config->listen.path)
    unlink(server->mon->config->listen.path);
}
