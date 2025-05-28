import cv2
import socket
import struct

# Setup socket connection to receiver (change IP and port)
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('RECEIVER_IP', 9999))

cap = cv2.VideoCapture(0)  # USB cam
+
while True:
    ret, frame = cap.read()
    if not ret:
        break

    # Encode frame as JPEG
    encoded, buffer = cv2.imencode('.jpg', frame, [int(cv2.IMWRITE_JPEG_QUALITY), 80])
    data = buffer.tobytes()

    # Send frame size first
    sock.sendall(struct.pack(">L", len(data)) + data)

cap.release()
sock.close()
