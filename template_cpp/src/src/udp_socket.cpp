#include "udp_socket.hpp"
#include <sys/socket.h>
#include <netinet/in.h> // sockaddr_in
#include <unistd.h> // close
#include <cstring> // strerror
#include <cerrno>
#include <stdexcept>


UdpSocket::UdpSocket(const Parser::Host &self)
    : fd_{socket(AF_INET, SOCK_DGRAM, 0)}
{
  if (fd_ < 0) {
    throw std::runtime_error(std::string("socket() failed: ") + std::strerror(errno));
  }

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = self.port;
  addr.sin_addr.s_addr = self.ip;

  if (bind(fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
    int err = errno;
    close(fd_);
    throw std::runtime_error("bind() to port " + std::to_string(self.portReadable()) +
                             " failed: " + std::strerror(err));
  }
  
}