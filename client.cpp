#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

char buf[512];

int main() {
  int fd{socket(AF_INET, SOCK_STREAM, 0)};
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(3030);
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  socklen_t len{sizeof(server_addr)};
  if (connect(fd, (const struct sockaddr *)&server_addr, len))
	  perror("connect()");
  auto data_bytes {recv(fd, buf, sizeof(buf), 0)};
  printf("[RECV] %.*s\n", int(data_bytes), buf);
}
