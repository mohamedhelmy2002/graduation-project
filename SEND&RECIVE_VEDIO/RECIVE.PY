import cv2
import socket
import struct
import numpy as np

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(('0.0.0.0', 9999))
sock.listen(1)
conn, addr = sock.accept()

data = b''
payload_size = struct.calcsize(">L")

while True:
    # Receive message size
    while len(data) < payload_size:
        data += conn.recv(4096)

    packed_msg_size = data[:payload_size]
    data = data[payload_size:]
    msg_size = struct.unpack(">L", packed_msg_size)[0]

    # Receive frame data
    while len(data) < msg_size:
        data += conn.recv(4096)

    frame_data = data[:msg_size]
    data = data[msg_size:]

    # Decode and show frame
    frame = cv2.imdecode(np.frombuffer(frame_data, dtype=np.uint8), cv2.IMREAD_COLOR)
    cv2.imshow('Received', frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

conn.close()
sock.close()
cv2.destroyAllWindows()
