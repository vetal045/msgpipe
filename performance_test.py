# tools/perf_test.py
import socket
import time
import struct
import threading

UDP_IP = "127.0.0.1"
UDP_PORT = 9001
TCP_PORT = 9003

MESSAGE_STRUCT = struct.Struct("<H B Q Q")  # MessageSize, Type, Id, Data
MESSAGE_SIZE = MESSAGE_STRUCT.size

NUM_MESSAGES = 100000

def start_tcp_server(received_ids):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server.bind((UDP_IP, TCP_PORT))
        server.listen(1)
        conn, _ = server.accept()
        with conn:
            while True:
                data = conn.recv(MESSAGE_SIZE)
                if not data:
                    break
                unpacked = MESSAGE_STRUCT.unpack(data)
                received_ids.append(unpacked[2])  # MessageId

if __name__ == "__main__":
    received_ids = []
    server_thread = threading.Thread(target=start_tcp_server, args=(received_ids,), daemon=True)
    server_thread.start()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    start = time.perf_counter()

    for i in range(NUM_MESSAGES):
        msg = MESSAGE_STRUCT.pack(MESSAGE_SIZE, 1, i, 10)
        sock.sendto(msg, (UDP_IP, UDP_PORT))

    sock.close()
    print("[perf] UDP messages sent")

    # Wait for TCP responses
    time.sleep(2)
    duration = time.perf_counter() - start

    print(f"[perf] Total time: {duration:.2f}s")
    print(f"[perf] Received {len(received_ids)} TCP messages")
    print(f"[perf] Throughput: {len(received_ids) / duration:.2f} msg/sec")
