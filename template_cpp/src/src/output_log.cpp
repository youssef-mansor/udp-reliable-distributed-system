#include "output_log.hpp"

#include <cerrno>
#include <charconv>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>

OutputLog::OutputLog(const std::string & path_to_out_file)
    : fd_{open(path_to_out_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644)}
{
    if (fd_ < 0) {
        throw std::runtime_error("cannot open output file " + path_to_out_file + ": " + std::strerror(errno));
    }
    buff_.reserve(FLUSH_BYTES + 64); // room for one more line past the threshold
}

OutputLog::~OutputLog(){
    flush();
    close(fd_);
}

void OutputLog::delivered(uint8_t sender_id, uint32_t seq){
    // the whole line is appended at once, so the file never holds half a line
    char line[32];
    char* p = line;
    *p++ = 'd';
    *p++ = ' ';
    p = std::to_chars(p, line + sizeof(line), sender_id).ptr;
    *p++ = ' ';
    p = std::to_chars(p, line + sizeof(line), seq).ptr;
    *p++ = '\n';
    appendLine(line, static_cast<size_t>(p - line));
}

void OutputLog::broadcasted(uint32_t seq){
    char line[32];
    char* p = line;
    *p++ = 'b';
    *p++ = ' ';
    p = std::to_chars(p, line + sizeof(line), seq).ptr;
    *p++ = '\n';
    appendLine(line, static_cast<size_t>(p - line));
}

void OutputLog::appendLine(const char* line, size_t len){
    buff_.append(line, len);
    if (buff_.size() >= FLUSH_BYTES) {
        flush();
    }
}

bool OutputLog::flush(){
    // write() may write fewer bytes than asked, so loop until everything is out
    size_t done = 0;
    while (done < buff_.size()) {
        ssize_t n = write(fd_, buff_.data() + done, buff_.size() - done);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            buff_.erase(0, done); // keep what wasn't written
            return false;
        }
        done += static_cast<size_t>(n);
    }
    buff_.clear();
    return true;
}
