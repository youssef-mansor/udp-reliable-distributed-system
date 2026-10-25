#!/usr/bin/env python3
"""Measure how many UDP datagrams fit in a socket's receive buffer, per payload size."""

import os
import socket
import sys

SIZES = list(range(1, 1001)) + [1500, 2000, 4000, 8000]
SENDS_PER_SIZE = 3000  # must exceed what the buffer can hold


def fit(size):
    # the receiver never reads while the sender floods it, then counts the survivors
    rcv = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    rcv.bind(("127.0.0.1", 0))
    snd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    for _ in range(SENDS_PER_SIZE):
        snd.sendto(b"x" * size, rcv.getsockname())
    rcv.setblocking(False)
    count = 0
    try:
        while True:
            rcv.recv(70000)
            count += 1
    except BlockingIOError:
        pass
    rcvbuf = rcv.getsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF)
    rcv.close()
    snd.close()
    return count, rcvbuf


def main():
    rows = []
    prev, start, last, rcvbuf = None, SIZES[0], SIZES[0], 0
    for size in SIZES:
        count, rcvbuf = fit(size)
        if count != prev:
            if prev is not None:
                rows.append((start, last, prev, rcvbuf // prev))
            prev, start = count, size
        last = size
    rows.append((start, last, prev, rcvbuf // prev))

    lines = [f"host {socket.gethostname()}, SO_RCVBUF = {rcvbuf} B"]
    lines += [f"payload {a:5d}-{b:5d} B: {n:4d} packets  (~{c} B charged each)" for a, b, n, c in rows]
    report = "\n".join(lines) + "\n"

    out_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "results")
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, f"{socket.gethostname()}.txt")
    with open(out_path, "w") as f:
        f.write(report)
    sys.stdout.write(report + f"saved to {out_path}\n")


if __name__ == "__main__":
    main()
