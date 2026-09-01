#!/usr/bin/env python3
"""Fake Crowd Control desktop app for developing the SimpleTCP transport.

Usage:
    python3 tools/crowdcontrol/fake_cc_server.py
    ctr_native --crowd-control

Commands:
    test <code>                     EffectTest
    start <code> [duration] [qty]   EffectStart
    stop <code>                     EffectStop
    data                            DataRequest
    gameupdate                      GameUpdate
    keepalive                       KeepAlive (no response expected)
    raw <json>                      send a JSON object verbatim
    quit
"""

from __future__ import annotations

import argparse
import json
import socket
import sys
import threading
from itertools import count
from typing import Optional


REQUEST_TYPE = {
    "test": 0x00,
    "start": 0x01,
    "stop": 0x02,
    "data": 0x20,
    "gameupdate": 0xFD,
    "keepalive": 0xFF,
}


def recv_frames(sock: socket.socket) -> None:
    """Prints each NUL-delimited frame the game sends until the connection closes."""
    buf = b""
    while True:
        try:
            chunk = sock.recv(4096)
        except OSError:
            break
        if not chunk:
            print("\n<< [connection closed by game]")
            break
        buf += chunk
        while b"\x00" in buf:
            frame, buf = buf.split(b"\x00", 1)
            print(f"\n<< {frame.decode('utf-8', errors='replace')}")
        print("> ", end="", flush=True)


def send_frame(sock: socket.socket, payload: dict) -> None:
    data = json.dumps(payload).encode("utf-8") + b"\x00"
    sock.sendall(data)
    print(f">> {json.dumps(payload)}")


def build_request(ids: "count[int]", command: str, args: list[str]) -> Optional[dict]:
    if command == "raw":
        return json.loads(" ".join(args))

    if command not in REQUEST_TYPE:
        print(f"unknown command: {command}", file=sys.stderr)
        return None

    request: dict = {"id": str(next(ids)), "type": REQUEST_TYPE[command]}

    if command in ("test", "start", "stop"):
        if not args:
            print(f"{command} needs a code", file=sys.stderr)
            return None
        request["code"] = args[0]

    if command == "start":
        if len(args) > 1:
            request["duration"] = float(args[1])
        if len(args) > 2:
            request["quantity"] = int(args[2])

    return request


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("-p", "--port", type=int, default=58430)
    args = parser.parse_args()

    listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listener.bind((args.host, args.port))
    listener.listen(1)
    print(f"listening on {args.host}:{args.port}, waiting for ctr_native --crowd-control ...")

    conn, addr = listener.accept()
    print(f"connected: {addr[0]}:{addr[1]}")

    reader = threading.Thread(target=recv_frames, args=(conn,), daemon=True)
    reader.start()

    ids = count(1)
    print("> ", end="", flush=True)
    for line in sys.stdin:
        line = line.strip()
        if not line:
            print("> ", end="", flush=True)
            continue
        if line in ("quit", "exit"):
            break

        parts = line.split()
        request = build_request(ids, parts[0], parts[1:])
        if request is not None:
            try:
                send_frame(conn, request)
            except OSError as exc:
                print(f"send failed: {exc}", file=sys.stderr)
                break
        print("> ", end="", flush=True)

    conn.close()
    listener.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
