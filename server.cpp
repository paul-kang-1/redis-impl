#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

int main() {
  int fd, conn_fd;
  struct sockaddr_in addr, client_addr;

  // obtain socket file descriptor
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1)
	perror("socket allocation failed");
  // set address assigned to socket to be reusable
  const int val{1};
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1)
	perror("setsockopt SO_REUSEADDR failed");
  // set address and port (localhost:3030) and bind
  addr.sin_family = AF_INET;
  addr.sin_port = htons(3030);
  // INADDR_LOOPBACK = 0x7f000001 = 127.0.0.1
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	perror("bind failed");
  // listen for connections
  if (listen(fd, SOMAXCONN) == -1)
	perror("listen failed");
  while (true) {
	socklen_t len{sizeof(client_addr)};
	// await connection on socket fd,
	conn_fd = accept(fd, (struct sockaddr *)&client_addr, &len);
	if (conn_fd == -1) {
	  perror("accept failed");
	  continue;
	};
	// send hello world, close after sending
	std::cout << "Enter message:\n";
	std::string msg;
	std::getline(std::cin, msg);
	if (send(conn_fd, msg.data(), msg.size(), 0) == -1)
	  perror("send error");
	close(conn_fd);
  }
}
