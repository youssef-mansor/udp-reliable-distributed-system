# Milestone 1 — Perfect Links: Design Decisions

## Approach
Perfect link = fair-loss UDP + retransmission (PL1) + duplicate elimination (PL2).
ACKs are a performance optimization: they let the sender stop retransmitting.

## Message identity
- A message is identified by `(sender_id, seq)`; seq is per-sender, starting at 1.
- Sender id is carried in the packet (not taken from `recvfrom` source address,
  since the OS may pick the outgoing port).

## Packet format (byte-aligned, multi-byte fields in network order)
```
Header (8 B): type u8 | sender_id u8 | reserved u16 | seq u32
DATA: header + 1..8 payloads (u32 each); seq = first message's seq,
      messages are seq..seq+k-1, k = (len - 8) / 4
ACK:  header only; seq = cumulative "delivered everything <= seq from you"
```
- Batching: up to 8 messages per packet (spec limit) to cut syscalls.
- No `count` field: derived from packet length.

- Seq numbers count DATA messages only; an ACK's seq refers to the peer's messages.

## Concurrency
Single-threaded event loop per process: non-blocking socket + `poll()`.
No locks, no thread switches (all processes share 2 cores).
- Each iteration reads at most `MAX_DRAIN` packets (e.g. 64), so ACKs and timers never starve under constant traffic; leftovers stay in the buffer and the next `poll()` returns immediately.

## Sender
- Retransmits unACKed messages on timeout until ACKed.
- Window: at most W unACKed packets in flight, so receivers never overflow while active.

## Window size
`senders × W ≤ C`, counted in packets (not bytes):
```
C = SO_RCVBUF (read at runtime) / per-packet charge (measured offline)
e.g. 212992 B / ~960 B ≈ 222 packets; 2 senders → W ≤ 111, 127 senders → W = 1
```
- The kernel charges ~960 B per datagram for payloads 1–325 B, so 8 messages per packet cost the same as 1.
- Per-packet charge depends on the kernel: measure it on the VM with `experiments/rcvbuf-capacity`.
- Use a safety margin (e.g. half of C); final W is tuned by the throughput experiment.

## Receiver
- Duplicate detection: highest contiguous delivered seq per sender
  + set of out-of-order seqs above it (keeps memory bounded).
- After draining the socket, one cumulative ACK per sender heard from.

## Deferred / open
- NACKs for gaps (wait until loss is confirmed; reordering ≠ loss) — add only if measured useful.
- Adaptive window (grow on ACKs, shrink on timeouts) — only if time allows.
- Timeout value (must exceed queueing delay at a busy receiver, not just network latency).
