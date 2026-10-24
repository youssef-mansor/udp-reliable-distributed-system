#include "udp_socket.hpp"
#include <sys/socket.h>
#include <netinet/in.h> // sockaddr_in
#include <unistd.h> // close
#include <cstring> // strerror
#include <cerrno>
#include <stdexcept>
#include <string>


namespace {
sockaddr_in toSockaddr(const Parser::Host &host) {
  
  sockaddr_in addr{};

  addr.sin_family = AF_INET;
  addr.sin_port = host.port;
  addr.sin_addr.s_addr = host.ip;
  
  return addr;
}
}

UdpSocket::UdpSocket(const Parser::Host &self)
    : fd_{socket(AF_INET, SOCK_DGRAM, 0)}
{
  if (fd_ < 0) {
    throw std::runtime_error(std::string("socket() failed: ") + std::strerror(errno));
  }

  sockaddr_in addr = toSockaddr(self);

  if (bind(fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
    int err = errno;
    close(fd_);
    throw std::runtime_error("bind() to port " + std::to_string(static_cast<unsigned>(self.portReadable())) +
                             " failed: " + std::strerror(err));
  }
  
}

UdpSocket::~UdpSocket(){
  close(fd_);
}

ssize_t UdpSocket::send(const Parser::Host &dest, const uint8_t * buf, size_t len){
  sockaddr_in addr = toSockaddr(dest);
  return sendto(fd_, buf, len, 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
}

ssize_t UdpSocket::receive(uint8_t * buf, size_t cap){
  return recvfrom(fd_, buf, cap, 0, nullptr, nullptr);
}