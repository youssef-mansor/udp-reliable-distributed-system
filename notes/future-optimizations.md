# Future Optimizations

Only after M1 is correct, and only if the throughput experiment shows a gain.

## Bitmap instead of the ranges map
Per sender, replace `std::map` with a fixed ring of bits covering `contiguous + 1 … contiguous + W × 8` (the sender's window bounds how far ahead a seq can be).
- Gain: O(1) insert and lookup, no heap allocation, cache-friendly.
- Cost: the receiver must know the sender's W.
- Example: W = 100 → 800 bits = 100 bytes per sender.

## Adaptive window
Replace the static `W = f(n)` with one that grows while ACKs arrive and shrinks on timeouts (like TCP).
- Gain: finds the receiver's limit on its own; backs off when processes are paused or crashed.
- Cost: more state and tuning than a constant.
- Example: +1 packet per ACK, halve W on a timeout.
