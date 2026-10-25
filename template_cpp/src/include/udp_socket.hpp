#pragma once

#include "parser.hpp"
#include <cstdint>
#include <cstddef>
#include <sys/types.h>

class UdpSocket {
  public:
    explicit UdpSocket(const Parser::Host &self);
    ~UdpSocket();
    UdpSocket(const UdpSocket &) = delete;
    UdpSocket & operator=(const UdpSocket &) = delete;
    ssize_t send(const Parser::Host &dest, const uint8_t * buf, size_t len);
    ssize_t receive(uint8_t * buf, size_t cap);
    int waitReadable(int timeout_ms);
    
  private:
    int fd_;
};