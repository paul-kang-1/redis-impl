#include <assert.h>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
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

static int32_t query(int fd, const char *msg) {
  uint32_t len = (uint32_t)strlen(msg);
  char wbuf[4 + len];
  char rbuf[4 + k_max_msg + 1];

  // 1. Write to socket fd
  // TODO: add big-endianness support
  memcpy(wbuf, &len, 4);
  memcpy(wbuf + 4, msg, len);
  if (auto err = write_all(fd, wbuf, 4 + len)) {
    fprintf(stderr, "write_all()\n");
    return err;
  }

  // 2a. Read message length (4 bytes) from socket fd
  errno = 0;
  auto err = read_full(fd, rbuf, 4);
  if (err) {
    if (!errno) {
      info("EOF");
    } else {
      info("read_full(): message length");
    }
    return err;
  }

  memcpy(&len, rbuf, 4);
  if (len > k_max_msg) {
    fprintf(stderr, "message too long: %d\n", len);
    return -1;
  }
  // 2b. Read actual message, given message length info
  if (auto err = read_full(fd, rbuf + 4, len)) {
    fprintf(stderr, "read_full(): message\n");
    return err;
  }
  rbuf[4 + len] = '\0';
  std::cout << "Got message: " << rbuf + 4 << std::endl;
  return 0;
}

int main() {
  struct sockaddr_in server_addr;

  int fd{socket(AF_INET, SOCK_STREAM, 0)};
  if (fd < 0) die("socket()");

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(3030);
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (connect(fd, (const struct sockaddr *)&server_addr, sizeof(server_addr))) {
    die("connect()");
  }

  if (query(fd, "Hello, World!"))
    goto L_DONE;
  if (query(fd, "Bye, World!"))
    perror("query()");
  goto L_DONE;

L_DONE:
  close(fd);
  return 0;
}
