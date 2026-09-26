#include "config.hpp"

#include <fstream>
#include <stdexcept>

PerfectLinksConfig parsePerfectLinksConfig(const std::string &path, std::size_t num_hosts) {
  std::ifstream configFile(path);
  if (!configFile.is_open()) {
    throw std::runtime_error("cannot open file " + path);
  }

  long long raw_msg_count = 0;
  long long raw_receiver_id = 0;
  if (!(configFile >> raw_msg_count >> raw_receiver_id)) {
    throw std::runtime_error("couldn't read values from configuration file " + path + ".");
  }

  if (raw_msg_count < 0 || raw_msg_count > 2147483647LL) {
    throw std::runtime_error("message count " + std::to_string(raw_msg_count) +
                             " is not within the valid range of messages: 1 - 2147483647.");
  }

  if (raw_receiver_id < 1 || static_cast<std::size_t>(raw_receiver_id) > num_hosts) {
    throw std::runtime_error("receiver process id " + std::to_string(raw_receiver_id) +
                             " is not within the value range of ids: 1 - " +
                             std::to_string(num_hosts) + ".");
  }

  PerfectLinksConfig config;
  config.msg_count = static_cast<unsigned int>(raw_msg_count);
  config.receiver_id = static_cast<unsigned long>(raw_receiver_id);
  return config;
}
