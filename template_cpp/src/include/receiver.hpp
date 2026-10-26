#pragma once

#include <map>
#include <cstdint>
#include <cstddef>

struct SenderState{
    uint32_t contiguous = 0; // delivered everything up to this point
    std::map<uint32_t, uint32_t> ranges; // contains ranges with gaps all delivered but not acked. all > contiguous + 1
};

// records [first, first + n - 1] for this sender and reports which seqs were new
size_t recordRange(SenderState& st, uint32_t first, size_t n, uint32_t* new_seqs);
