#include "monikor.h"
#include "io_handler.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>

#define MNK_NET_RD_SZ 4096

typedef struct {
  char *send;
  void (*callback)(char *, void *);
  void *data;
  struct addrinfo *ai;
  struct addrinfo *ai_cur;
} monikor_net_handler_data_t;


static int monikor_net_gai(const char *host, const char *port, monikor_net_handler_data_t *handler_data) {
  struct addrinfo hints;

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;
  if (getaddrinfo(host, port, &hints, &handler_data->ai)) {
    freeaddrinfo(handler_data->ai);
    return -1;
  }
  handler_data->ai_cur = handler_data->ai;
  return 0;
}

static int monikor_net_connect(monikor_net_handler_data_t *handler_data) {
  int sock;
  int flags;
  struct addrinfo *ai = handler_data->ai_cur;
  char host[512];
  char port[512];

  if (!getnameinfo(ai->ai_addr, ai->ai_addrlen, host, 512, port, 512, NI_NUMERICHOST|NI_NUMERICSERV))
    monikor_log(LOG_DEBUG, "Trying to connect to %s port %s...\n", host, port);
  if ((sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) == -1
  || (flags = fcntl(sock, F_GETFL, 0)) == -1 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1
  || (connect(sock, ai->ai_addr, ai->ai_addrlen) == -1
  && errno != EINPROGRESS)) {
    return -1;
  }
  return sock;
}

static char *monikor_net_recv(int sock) {
  char *response = NULL;
  size_t response_size = 0;
  int read_bytes;

  do {
    if (!(response = realloc(response, response_size + MNK_NET_RD_SZ + 1)))
      goto err;
    read_bytes = read(sock, response + response_size, MNK_NET_RD_SZ);
    if (read_bytes == -1)
      goto err;
    response_size += read_bytes;
    response[response_size] = 0;
  } while (read_bytes == MNK_NET_RD_SZ);
  return response;

err:
  free(response);
  return NULL;
}

static void handle_io_read(monikor_io_handler_t *handler, uint8_t mode) {
  monikor_net_handler_data_t *handler_data = (monikor_net_handler_data_t *)handler->data;
  char *received;
  void *data;
  void (*callback)(char *, void *);

  (void)mode;
  received = monikor_net_recv(handler->fd);
  data = handler_data->data;
  callback = handler_data->callback;
  close(handler->fd);
  handler->fd = -1;
  handler->mode = 0;
  handler->callback = NULL;
  freeaddrinfo(handler_data->ai);
  free(handler->data);
  handler->data = NULL;
  if (received)
    callback(received, data);
  free(received);
}

static void handle_io_connect(monikor_io_handler_t *handler, uint8_t mode) {
  monikor_net_handler_data_t *handler_data = (monikor_net_handler_data_t *)handler->data;
  int flags;
  int result;
  int fd;
  socklen_t result_len = sizeof(result);

  (void)mode;
  if (getsockopt(handler->fd, SOL_SOCKET, SO_ERROR, &result, &result_len) == -1 || result) {
    handler_data->ai_cur = handler_data->ai_cur->ai_next;
    if (!handler_data->ai_cur
    || (fd = monikor_net_connect(handler_data)) == -1) {
      monikor_log(LOG_ERR, "Cannot connect: %s\n", strerror(errno));
      goto err;
    }
    close(handler->fd);
    handler->fd = fd;
    return;
  }
  if (((flags = fcntl(handler->fd, F_GETFL, 0)) == -1)
  || fcntl(handler->fd, F_SETFL, flags & ~O_NONBLOCK) == -1
  || (handler_data->send &&
  write(handler->fd, handler_data->send, strlen((char *)handler_data->send)) == -1)) {
    monikor_log(LOG_ERR, "Cannot send data: %s\n", strerror(errno));
    goto err;
  }
  handler->mode = MONIKOR_IO_HANDLER_RD;
  handler->callback = &handle_io_read;
  return;

err:
    handler->mode = 0;
    close(handler->fd);
    handler->fd = -1;
    freeaddrinfo(handler_data->ai);
    free(handler_data);
    handler->data = NULL;
    handler->callback = NULL;
}

monikor_io_handler_t *monikor_net_exchange(const char *host, const char *port, char *send,
void (*callback)(char *response, void *data), void *data) {
  monikor_net_handler_data_t *handler_data;
  monikor_io_handler_t *handler;
  int fd;

  if (!(handler_data = malloc(sizeof(*handler_data)))
  || monikor_net_gai(host, port, handler_data)) {
    free(handler_data);
    return NULL;
  }
  if ((fd = monikor_net_connect(handler_data)) == -1) {
    freeaddrinfo(handler_data->ai);
    free(handler_data);
    return NULL;
  }
  handler = monikor_io_handler_new(fd, MONIKOR_IO_HANDLER_WR, &handle_io_connect,
    (void *)handler_data);
  if (!handler) {
    freeaddrinfo(handler_data->ai);
    free(handler_data);
    close(fd);
    return NULL;
  }
  handler_data->send = send;
  handler_data->callback = callback;
  handler_data->data = data;
  return handler;
}
