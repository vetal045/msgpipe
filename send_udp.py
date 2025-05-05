import socket
import struct

msg = struct.pack('<H B Q Q', 26, 1, 123, 10)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(msg, ("127.0.0.1", 5001))
print("[UDP SEND] Sent Message with id=123, data=10")
