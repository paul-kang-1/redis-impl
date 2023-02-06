#include <assert.h>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

/*
 * Common Dependencies
 */
const size_t k_max_msg = 4096;

static void info(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static int32_t read_full(int fd, char *buf, size_t n) {
  while (n > 0) {
    ssize_t n_bytes = read(fd, buf, n);
    if (n_bytes <= 0) {
      return -1;
    }
    assert((size_t)n_bytes <= n);
    n -= (size_t)n_bytes; // simple sanity check
    buf += n_bytes;       // move pointer up to next location
  }
  return 0;
}

static int32_t write_all(int fd, const char *buf, size_t n) {
  while (n > 0) {
    ssize_t n_bytes = write(fd, buf, n);
    if (n_bytes <= 0)
      return -1;
    assert((size_t)n_bytes <= n);
    n -= (size_t)n_bytes;
    buf += n_bytes;
  }
  return 0;
}
/*
 * END Common Dependencies
 */

/*
 * 4 byte little endian length specifier + msg
 * each message can hold
 *
 * +-----+------+-----+------+--------
 * | len | msg1 | len | msg2 | more...
 * +-----+------+-----+------+--------
 */
static int32_t one_request(int connfd) {
  char rbuf[4 + k_max_msg + 1];
  errno = 0;
  int32_t err = read_full(connfd, rbuf, 4);
  if (err) {
    if (errno) {
      info("read()");
    } else {
      info("EOF");
    }
    return err;
  }

  // TODO: Add big endian support (network)
  uint32_t len = 0;
  memcpy(&len, rbuf, 4);
  if (len > k_max_msg) {
    printf("Msg too long: %d bytes\n", len);
    return -1;
  }

  err = read_full(connfd, rbuf + 4, len);
  if (err) {
    info("msg read()");
    return err;
  }

  rbuf[4 + len] = '\0'; // manually add null terminator
  printf("Client message: %s\n", rbuf + 4);

  char wbuf[4 + k_max_msg];
  const char msg[] = "Hello from server";
  len = (uint32_t)strlen(msg);
  memcpy(wbuf, &len, sizeof(uint32_t));
  memcpy(wbuf + 4, msg, len);

  return write_all(connfd, wbuf, sizeof(uint32_t) + len);
}

int main() {
  int fd, conn_fd;
  socklen_t addr_len;
  struct sockaddr_in addr, client_addr;

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1)
    die("socket allocation failed");

  // set address assigned to socket to be reusable
  const int val{1};
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1)
    die("setsockopt SO_REUSEADDR failed");

  addr.sin_family = AF_INET;
  addr.sin_port = htons(3030);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    die("bind failed");
  if (listen(fd, SOMAXCONN) == -1)
    die("listen failed");

  while (true) {
    addr_len = sizeof(client_addr);
    info("looking for connections...");
    conn_fd = accept(fd, (struct sockaddr *)&client_addr, &addr_len);
    if (conn_fd == -1) {
      info("accept failed");
      continue;
    };
    while (true) {
      if (one_request(conn_fd)) {
        break;
      }
    }
    close(conn_fd);
  }

  return 0;
}
