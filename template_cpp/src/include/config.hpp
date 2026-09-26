#pragma once

#include <cstddef>
#include <string>

struct PerfectLinksConfig {
  unsigned int msg_count;
  unsigned long receiver_id;
};

PerfectLinksConfig parsePerfectLinksConfig(const std::string &path, std::size_t num_hosts);
