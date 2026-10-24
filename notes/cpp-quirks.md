# C++ and Systems Quirks

Things learned while building Milestone 1 (Perfect Links).

## C++ language

| Quirk | Takeaway |
|---|---|
| `explicit` | A one-argument constructor otherwise doubles as a silent conversion (`archive("x.txt")` would build a `TextFile`). Mark them `explicit` |
| `= delete` on copy | `T(const T&) = delete;` and `T& operator=(const T&) = delete;` forbid copying. Essential for classes owning an fd or `FILE*` (avoids double close) |
| `T b = a;` is construction | Despite the `=`, it calls the copy *constructor*. `b = a;` on an existing `b` calls `operator=` |
| Throwing constructor skips the destructor | If the constructor throws after acquiring a resource, clean up manually before the `throw` (`close(fd_)` on `bind` failure) |
| Initializer list | `: fd_{socket(...)}` sets members before the constructor body runs |
| `const` methods | `bool isOpen() const;` — the `const` must be repeated in the `.cpp` definition |
| Definition repeats the return type | `ssize_t UdpSocket::send(...)`; omitting it gives "forbids declaration with no type" |
| `::` scope resolution | `Parser::Host` (type nested in a class), `std::string` (namespace), `UdpSocket::send` (method definition) |
| Semicolon after `class {...};` | Forgetting it produces confusing errors further down |
| Anonymous namespace | `namespace { ... }` makes helpers (`toSockaddr`) private to one `.cpp` |
| `constexpr` vs `#define` | Typed, scoped, visible in errors and the debugger; usable as an array size: `uint8_t buf[packet::MAX_PACKET_SIZE + 1]` |
| `T arr[12] = {};` | In a declaration `[12]` is the size and `{}` zeroes every element; in an expression `arr[12]` is out of bounds (valid indices 0–11) |
| Comma operator | `buff[7, 6, 5, 4] = 1;` is just `buff[4] = 1;` — compiles silently |
| Array decay: `buff` vs `&buff` | `buff` becomes `uint8_t*` (first byte); `&buff` is `uint8_t (*)[40]` (the whole array) |
| `memcpy` instead of pointer casts | Casting `char*` to `uint32_t*` is undefined behaviour; `memcpy` the bytes into a variable |
| `memcpy` length bug | The index goes in the offset (`base + i * 4`), never in the length, or you overflow the destination |
| `size_t` vs `ssize_t` | −1 stored in `size_t` becomes 18,446,744,073,709,551,615; mixing signed and unsigned turns negatives into huge positives |
| `>>` into unsigned | `-5` silently wraps to 4294967291; read into a wider signed type, then range-check |
| `std::to_string(unsigned short)` | No exact overload, promotes to `int`, rejected by `-Wsign-promo`; use `static_cast<unsigned>(...)` |
| Printing a `uint8_t` | `cout << id` prints a character; `cout << +id` prints the number |
| `LL` suffix | `2147483647LL` is a `long long` literal; `std::numeric_limits<int>::max()` is clearer |
| Names starting with `_` | Some are reserved for the compiler and library; use a trailing underscore (`fd_`) |
| Include what you use | Don't rely on another header happening to include `<cstdint>`, `<cerrno>`, `<cstdio>`, ... |
| `<cstring>` vs `<string.h>` | The C++ header guarantees `std::memcpy`; the C one works by accident |
| Header-only classes | Methods defined inside the class body are implicitly `inline`, so including the header many times is fine (`parser.hpp`) |

## System calls and sockets

| Quirk | Takeaway |
|---|---|
| `errno` is fragile | Any later call (even `close()`) may overwrite it — save it or call `perror` immediately |
| `perror("x")` | Prints `x: <errno text>` to stderr |
| EINTR | A blocking call interrupted by a signal returns −1 with `errno == EINTR`; retry unless stopping |
| `signal()` on glibc | BSD semantics (`SA_RESTART`): interrupted syscalls restart automatically, except with receive timeouts after SIGSTOP/SIGCONT |
| SIGSTOP / SIGCONT | Cannot be caught; the process just freezes and resumes |
| Signal handlers interrupt anything | Even single-threaded code mid-`push_back`; only a few operations are safe inside a handler |
| Family / type / protocol | `AF_INET` = IPv4 addressing, `SOCK_DGRAM` = whole messages with no guarantees; protocol 0 then means UDP |
| `sockaddr*` casting | Old C API: pass `reinterpret_cast<sockaddr*>(&addr_in)` |
| Zero `sockaddr_in` first | Leftover garbage breaks `bind` |
| Network byte order | Big-endian on the wire; `htonl`/`ntohl` (32-bit), `htons`/`ntohs` (16-bit). A missing `htonl` turns seq 1 into 16,777,216 |
| Parser's ip and port | Already in network order; calling `htons` again swaps them back |
| `recvfrom` address arguments | Outputs (who sent); pass `nullptr, nullptr` if not needed |
| `sendto` success ≠ delivery | Only means "my kernel accepted it" |
| Truncation trap | A datagram larger than the buffer is silently cut; use a `+1` byte buffer and reject `n > MAX` |
| Kernel buffers | Full receive buffer means silent drops; full send buffer means `sendto` blocks; each tiny datagram costs hundreds of bytes of buffer |
| Sending to an unbound port | The packet is simply discarded |

## Build and tooling

| Quirk | Takeaway |
|---|---|
| Template builds with `-Werror` | Every warning is a build failure |
| Never build with `sudo` | Root-owned `target/` and `bin/` break later builds; `mv` silently prompts and keeps the old binary |
| `SOURCES` | Lists `.cpp` files only; headers are found through `include_directories(include)` |
| "undefined reference" | Usually a `.cpp` missing from `SOURCES`, or a method defined without the `Class::` prefix |
| "Address already in use" | An old process still holds the port: `pkill da_proc` |
