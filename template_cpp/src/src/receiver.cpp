#include "receiver.hpp"

#include <algorithm>
#include <iterator>

namespace{
// true if seq lies inside one of the stored ranges
bool inRanges(const std::map<uint32_t, uint32_t>& ranges, uint32_t seq){
    auto it = ranges.upper_bound(seq); // first range starting after seq
    if (it == ranges.begin()){
        return false; // no range starts at or before seq
    }
    --it; // last range starting at or before seq
    return seq <= it->second;
}
}

size_t recordRange(SenderState& st, uint32_t first, size_t n, uint32_t* new_seqs){
    // n is already validated by packet::decode (1..MAX_MSGS_PER_PACKET)
    const uint32_t last = first + static_cast<uint32_t>(n) - 1;

    // all old: everything up to contiguous was delivered already
    if (last <= st.contiguous) {
        return 0;
    }

    // clip: drop the part that is <= contiguous
    const uint32_t start = std::max(first, st.contiguous + 1);

    // fast path: continues contiguous and touches no stored range
    if (start == st.contiguous + 1 && (st.ranges.empty() || last < st.ranges.begin()->first - 1 )) {
        size_t k = 0;
        for (uint32_t seq = start; seq <= last; seq++) {
            new_seqs[k++] = seq;
        }
        st.contiguous = last;
        return k;
    }

    // general case: lands beyond a gap or overlaps stored ranges
    size_t k = 0;
    for (uint32_t seq = start; seq <= last; seq++) {
        if (!inRanges(st.ranges, seq)) {
            new_seqs[k++] = seq;
        }
    }

    // merge [start, last] with every stored range it overlaps or touches
    uint32_t lo = start;
    uint32_t hi = last;
    auto it = st.ranges.upper_bound(start); // first range starting after start
    if (it != st.ranges.begin()) {
        auto prev = std::prev(it); // last range starting at or before start
        if (prev->second + 1 >= start) {
            lo = prev->first;
            hi = std::max(hi, prev->second);
            st.ranges.erase(prev); // it stays valid: it points at a different entry
        }
    }
    while (it != st.ranges.end() && it->first <= hi + 1) {
        hi = std::max(hi, it->second);
        it = st.ranges.erase(it);
    }
    st.ranges.emplace_hint(it, lo, hi); // it is the position right after [lo, hi]

    // absorb: the gap after contiguous is closed
    auto smallest = st.ranges.begin();
    if (smallest->first == st.contiguous + 1) {
        st.contiguous = smallest->second;
        st.ranges.erase(smallest);
    }

    return k;
}
