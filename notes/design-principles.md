# Design Principles Used

| Principle | Definition | Where in the project |
|---|---|---|
| High cohesion | Everything in a module belongs to one topic. | `packet.hpp`/`packet.cpp` hold only the wire format; `UdpSocket` holds only socket I/O. |
| Low coupling | Modules depend on each other's interfaces, not internals. | `main.cpp` calls `encodeData`/`decode` and never touches byte offsets. |
| Single responsibility | A unit has one reason to change. | Config parsing lives in `config.cpp`, so M2's new config format changes only that file. |
| Separation of concerns | Each layer handles only what it knows. | `decode` checks the format; the caller checks `sender_id` against the hosts count and handles `receive` returning −1. |
| DRY (don't repeat yourself) | Each fact is written once. | `PAYLOAD_OFFSET = HEADER_SIZE`; `MAX_PACKET_SIZE` is computed; `writeHeader` is shared by `encodeData` and `encodeAck`; `toSockaddr` by the constructor and `send`. |
| Named constants | No magic numbers. | `packet::SEQ_OFFSET` instead of `4`; `uint8_t buf[packet::MAX_PACKET_SIZE]` instead of `buf[40]`. |
| Encapsulation | Hide internals behind a small interface. | `fd_` is private; `waitReadable()` wraps `poll()` instead of exposing an `fd()` getter; helpers sit in anonymous namespaces. |
| RAII | A resource's lifetime is tied to an object's lifetime. | `UdpSocket` opens in its constructor, closes in its destructor, and forbids copying (no double close). |
| Explicit ownership | The caller owns memory; functions don't allocate behind its back. | `encodeData` writes into the caller's stack buffer and returns the length, instead of returning `malloc`'d memory. |
| Group related data | Data used together lives in one type, not in parallel containers. | `SenderState { contiguous, ranges }` in one `std::vector`, instead of one vector of counters and another of maps. |
| Fail fast | Validate early and stop on bad input. | Config values are range-checked at startup; `decode` rejects malformed packets before any field is used. |
