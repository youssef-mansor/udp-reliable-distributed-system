#pragma once

#include <string>
#include <cstddef>
#include <cstdint>

class OutputLog{
public:
    explicit OutputLog(const std::string & path_to_out_file);
    ~OutputLog(); // flushes, then closes the file.
    OutputLog(const OutputLog &) = delete;
    OutputLog & operator=(const OutputLog &) = delete;
    void delivered(uint8_t sender_id, uint32_t seq);
    void broadcasted(uint32_t seq);
    bool flush();

private:
    void appendLine(const char* line, size_t len); // appends, flushing at FLUSH_BYTES
    std::string buff_;
    static constexpr size_t FLUSH_BYTES = 1 << 20; // write to the file once it reaches 1 MiB
    int fd_;
};
