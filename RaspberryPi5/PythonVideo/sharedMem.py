import socket
import os
import json
import numpy as np
import struct

PYTHON_COMMUNICATION_PATH = "/tmp/python_shared_mem"
C_DRIVER_COMMUNICATION_PATH = "../tmp/rover.sock"
class unix_socket_server:
    def __init__(self, ipc_use):
        if ipc_use == "PYTHON":
            self.path = PYTHON_COMMUNICATION_PATH
        elif ipc_use == "C":
            self.path = C_DRIVER_COMMUNICATION_PATH

        if os.path.exists(self.path):
            os.remove(self.path)

        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.sock.bind(self.path)
        self.sock.listen(1)

        self.conn, _ = self.sock.accept()

    def send_image(self, array: np.ndarray):
        header = {
            "dtype": str(array.dtype),
            "shape": array.shape,
            "data_len": array.nbytes,
        }
        header_bytes = json.dumps(header).encode()

        self.conn.sendall(struct.pack("!I", len(header_bytes)))
        self.conn.sendall(header_bytes)
        self.conn.sendall(array.tobytes())

    def recv_message(self):
        raw_len = self.conn.recv(4)
        if not raw_len:
            return None

        msg_len = struct.unpack("!I", raw_len)[0]
        msg_bytes = self.conn.recv(msg_len)

        return json.loads(msg_bytes.decode())


    def close(self):
        self.conn.close()
        self.sock.close()
        os.remove(self.path)


class unix_client:
    def __init__(self, ipc_use):
        if ipc_use == "PYTHON":
            self.path = PYTHON_COMMUNICATION_PATH
        elif ipc_use == "C":
            self.path = C_DRIVER_COMMUNICATION_PATH

        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.sock.connect(self.path)

    def _recvall(self, n):
        data = b""
        while len(data) < n:
            chunk = self.sock.recv(n - len(data))
            if not chunk:
                raise ConnectionError("Socket closed")
            data += chunk
        return data

    def recv_image(self):
        header_len = struct.unpack("!I", self._recvall(4))[0]
        header = json.loads(self._recvall(header_len).decode())

        img_bytes = self._recvall(header["data_len"])
        img = np.frombuffer(img_bytes, dtype=header["dtype"])
        return img.reshape(header["shape"])

    def send_ready(self):
        msg = {
            "type": "READY"
        }
        msg_bytes = json.dumps(msg).encode()

        self.sock.sendall(struct.pack("!I", len(msg_bytes)))
        self.sock.sendall(msg_bytes)

    def send_struct(self, data):
        self.sock.sendall(data)


    def close(self):
        self.sock.close()
