#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
  char buf[512];
  socklen_t addr_len;
  ssize_t data_bytes;
  struct sockaddr_in server_addr;

  int fd{socket(AF_INET, SOCK_STREAM, 0)};

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(3030);
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr_len = sizeof(server_addr);
  if (connect(fd, (const struct sockaddr *)&server_addr, addr_len))
	perror("connect()");
  data_bytes = recv(fd, buf, sizeof(buf), 0);
  printf("[RECV] %.*s\n", int(data_bytes), buf);
}
