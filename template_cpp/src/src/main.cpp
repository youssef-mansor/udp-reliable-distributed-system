#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "hello.h"
#include "config.hpp"
#include "parser.hpp"
#include "udp_socket.hpp"
#include <signal.h>
#include "packet.hpp"
#include <cerrno>
#include <cstdio>

// Global variabls
uint32_t SEQ = 0;

static void stop(int) {
  // reset signal handlers to default
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT, SIG_DFL);

  // immediately stop network packet processing
  std::cout << "Immediately stopping network packet processing.\n";

  // write/flush output file if necessary
  std::cout << "Writing output.\n";

  // exit directly from signal handler
  exit(0);
}

int main(int argc, char **argv) {
  signal(SIGTERM, stop);
  signal(SIGINT, stop);

  // `true` means that a config file is required.
  // Call with `false` if no config file is necessary.
  bool requireConfig = true;

  Parser parser(argc, argv);
  parser.parse();

  // Check id is within range defined implicitly by hosts
  auto hosts = parser.hosts();
  if (parser.id() < 1 || parser.id() > hosts.size()) {
    std::cerr << "Error: --id " << parser.id() << " is not in the hosts file (valid: 1 - "
              << hosts.size() << ")\n";
    return EXIT_FAILURE;
  }

  hello();
  std::cout << std::endl;

  std::cout << "My PID: " << getpid() << "\n";
  std::cout << "From a new terminal type `kill -SIGINT " << getpid() << "` or `kill -SIGTERM "
            << getpid() << "` to stop processing packets\n\n";

  std::cout << "My ID: " << parser.id() << "\n\n";

  std::cout << "List of resolved hosts is:\n";
  std::cout << "==========================\n";
  for (auto &host : hosts) {
    std::cout << host.id << "\n";
    std::cout << "Human-readable IP: " << host.ipReadable() << "\n";
    std::cout << "Machine-readable IP: " << host.ip << "\n";
    std::cout << "Human-readbale Port: " << host.portReadable() << "\n";
    std::cout << "Machine-readbale Port: " << host.port << "\n";
    std::cout << "\n";
  }
  std::cout << "\n";

  std::cout << "Path to output:\n";
  std::cout << "===============\n";
  std::cout << parser.outputPath() << "\n\n";

  std::cout << "Path to config:\n";
  std::cout << "===============\n";
  std::cout << parser.configPath() << "\n\n";

  std::cout << "Doing some initialization...\n\n";

  PerfectLinksConfig config;
  try {
    config = parsePerfectLinksConfig(parser.configPath(), hosts.size());
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return EXIT_FAILURE;
  }

  const bool isReceiver = (parser.id() == config.receiver_id);

  if (isReceiver) {
    std::cout << "I am the RECEIVER.\n\n";
  } else {
    std::cout << "I am a SENDER: sending " << config.msg_count << " messages to process "
              << config.receiver_id << ".\n\n";
  }


  std::cout << "Broadcasting and delivering messages...\n\n";


  UdpSocket sock(hosts[parser.id() - 1]);
  if(isReceiver){
    uint8_t buff[packet::MAX_PACKET_SIZE + 1] = {};  // +1 spare byte detects oversized packets

    // receive; retry if a signal interrupted the wait
    ssize_t packet_length = 0;
    sock.waitReadable(-1);
    do {
      packet_length = sock.receive(buff, sizeof(buff));
    } while (packet_length < 0 && errno == EINTR);

    packet::Decoded pkt;
    if (packet_length < 0) {
      perror("receive");
    } else if (!packet::decode(buff, static_cast<size_t>(packet_length), pkt)) {
      std::cerr << "Dropped malformed packet of " << packet_length << " bytes\n";
    } else if (pkt.sender_id < 1 || pkt.sender_id > hosts.size()) {
      std::cerr << "Dropped packet from unknown sender " << +pkt.sender_id << "\n";
    } else if (pkt.type != packet::TYPE_DATA) {
      std::cout << "Ignored non-DATA packet (type " << +pkt.type << ")\n";
    } else {
      std::cout << packet_length << " bytes: DATA from " << +pkt.sender_id << ", seq " << pkt.seq
                << ", k=" << pkt.n << "\n";
      // message i has seq + i
      for (size_t i = 0; i < pkt.n; i++) {
        std::cout << "  msg seq " << pkt.seq + i << ", payload " << pkt.payloads[i] << "\n";
      }
    }
  }else{ // sender
    // test packet: messages 1..4, whose payloads are 1..4
    const uint32_t payloads[] = {1, 2, 3, 4};
    const size_t msg_count = sizeof(payloads) / sizeof(payloads[0]);
    uint8_t buf[packet::MAX_PACKET_SIZE];

    size_t len = packet::encodeData(buf, static_cast<uint8_t>(parser.id()), 1, payloads, msg_count);

    if (sock.send(hosts[config.receiver_id-1], buf, len) < 0){
      perror("send");
    }
  }

  // After a process finishes broadcasting,
  // it waits forever for the delivery of messages.
  while (true) {
    std::this_thread::sleep_for(std::chrono::hours(1));
  }

  return 0;
}
