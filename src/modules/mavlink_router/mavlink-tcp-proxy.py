#!/usr/bin/env python3
"""
mavlink-tcp-proxy.py - Reliable TCP server for MAVLink GCS connections.
Listens on TCP 5760 and bridges bidirectionally to mavlink-router's Internal_Companion (UDP 127.0.0.1:14540).
Bypasses Windows Firewall UDP restrictions on client laptops.
"""

import socket
import threading
import sys
import os

TCP_PORT = 5760
UDP_HOST = "127.0.0.1"
UDP_PORT = 14540

def handle_client(client_sock, client_addr):
    print(f"[tcp-proxy] Client connected from {client_addr}", flush=True)
    try:
        udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        udp_sock.connect((UDP_HOST, UDP_PORT))
        # Initial ping to register client with mavlink-router
        udp_sock.send(b"\x00")
    except Exception as e:
        print(f"[tcp-proxy] Failed to connect to UDP {UDP_HOST}:{UDP_PORT}: {e}", flush=True)
        client_sock.close()
        return

    stop = threading.Event()

    def tcp_to_udp():
        while not stop.is_set():
            try:
                data = client_sock.recv(4096)
                if not data:
                    break
                udp_sock.send(data)
            except Exception:
                break
        stop.set()

    def udp_to_tcp():
        while not stop.is_set():
            try:
                data = udp_sock.recv(4096)
                if not data:
                    break
                client_sock.sendall(data)
            except Exception:
                break
        stop.set()

    t1 = threading.Thread(target=tcp_to_udp, daemon=True)
    t2 = threading.Thread(target=udp_to_tcp, daemon=True)
    t1.start()
    t2.start()

    stop.wait()
    try:
        client_sock.close()
    except Exception:
        pass
    try:
        udp_sock.close()
    except Exception:
        pass
    print(f"[tcp-proxy] Client {client_addr} disconnected", flush=True)

def main():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("0.0.0.0", TCP_PORT))
    server.listen(5)
    print(f"[tcp-proxy] Listening on 0.0.0.0:{TCP_PORT}, forwarding to {UDP_HOST}:{UDP_PORT}", flush=True)

    while True:
        try:
            client_sock, client_addr = server.accept()
            client_sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            t = threading.Thread(target=handle_client, args=(client_sock, client_addr), daemon=True)
            t.start()
        except Exception as e:
            print(f"[tcp-proxy] Accept error: {e}", flush=True)

if __name__ == "__main__":
    main()