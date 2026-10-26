// tests for recordRange (template_cpp/src/src/receiver.cpp)
#include "receiver.hpp"

#include <cstdio>
#include <random>
#include <set>
#include <vector>

namespace {

struct Case {
  const char* name;
  uint32_t contiguous;                       // state before
  std::map<uint32_t, uint32_t> ranges;
  uint32_t first;                            // packet
  size_t n;
  std::vector<uint32_t> want_new;            // expected result
  uint32_t want_contiguous;
  std::map<uint32_t, uint32_t> want_ranges;
};

int failures = 0;

void runCase(const Case& c) {
  SenderState st{c.contiguous, c.ranges};
  uint32_t got[8];
  size_t k = recordRange(st, c.first, c.n, got);
  std::vector<uint32_t> got_new(got, got + k);
  bool ok = got_new == c.want_new && st.contiguous == c.want_contiguous && st.ranges == c.want_ranges;
  printf("%s  %s\n", ok ? "PASS" : "FAIL", c.name);
  if (!ok) failures++;
}

// random packets checked against a plain std::set of every delivered seq
void runRandom(int trials, int packets) {
  std::mt19937 rng(1);
  for (int t = 0; t < trials; t++) {
    SenderState st;
    std::set<uint32_t> seen;
    for (int p = 0; p < packets; p++) {
      uint32_t first = 1 + rng() % 120;
      size_t n = 1 + rng() % 8;
      uint32_t got[8];
      size_t k = recordRange(st, first, n, got);

      std::vector<uint32_t> want;
      for (uint32_t s = first; s < first + n; s++) {
        if (!seen.count(s)) want.push_back(s);
      }
      seen.insert(want.begin(), want.end());
      uint32_t c = 0;
      while (seen.count(c + 1)) c++;

      // ranges must be sorted, disjoint and separated from contiguous and each other by a gap
      bool ranges_ok = true;
      uint32_t prev_end = st.contiguous;
      for (auto& [a, b] : st.ranges) {
        if (a <= prev_end + 1 || b < a) ranges_ok = false;
        prev_end = b;
      }

      if (std::vector<uint32_t>(got, got + k) != want || st.contiguous != c || !ranges_ok) {
        printf("FAIL  random (trial %d, packet %d: %u-%zu)\n", t, p, first, first + n - 1);
        failures++;
        return;
      }
    }
  }
  printf("PASS  random: %d trials x %d packets\n", trials, packets);
}

}  // namespace

int main() {
  const std::map<uint32_t, uint32_t> two = {{44, 49}, {51, 53}};
  const Case cases[] = {
    {"all old: 35-38",                40, two, 35, 4, {},                      40, two},
    {"fast path: 41-48",              40, {},  41, 8, {41,42,43,44,45,46,47,48}, 48, {}},
    {"gap opens: 44-49",              40, {},  44, 6, {44,45,46,47,48,49},     40, {{44, 49}}},
    {"empty map, gap: 60-62",         40, {},  60, 3, {60,61,62},              40, {{60, 62}}},
    {"overlap: 50-55",                40, two, 50, 6, {50,54,55},              40, {{44, 55}}},
    {"gap filled, absorb: 41-43",     40, two, 41, 3, {41,42,43},              49, {{51, 53}}},
    {"messy retransmission: 38-45",   40, two, 38, 8, {41,42,43},              49, {{51, 53}}},
    {"extend a range: 50-52",         40, {{44, 49}, {60, 62}}, 50, 3, {50,51,52}, 40, {{44, 52}, {60, 62}}},
    {"duplicate of a stored range",   40, two, 51, 3, {},                      40, two},
  };
  for (const Case& c : cases) runCase(c);
  runRandom(2000, 60);

  printf(failures ? "\n%d FAILED\n" : "\nall passed\n", failures);
  return failures ? 1 : 0;
}
