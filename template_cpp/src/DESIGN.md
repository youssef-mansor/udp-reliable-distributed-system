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

## Sender
- Retransmits unACKed messages on timeout until ACKed.

## Receiver
- Duplicate detection: highest contiguous delivered seq per sender
  + set of out-of-order seqs above it (keeps memory bounded).
- Sends cumulative ACK every N packets **or** after a short delay.

## Deferred / open
- NACKs for gaps (wait until loss is confirmed; reordering ≠ loss) — add only if measured useful.
- Bounding how far a sender runs ahead of the receiver (window).
- Threads vs single-threaded event loop.
- Timeout value.
