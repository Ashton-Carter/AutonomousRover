import socket
import os
import json
import numpy as np
import struct
from sys import exit
PYTHON_CLASSIFIER_COMMUNICATION_PATH = "/tmp/python_shared_mem"
C_DRIVER_COMMUNICATION_PATH = "/tmp/rover.sock"
PYTHON_WEBSOCKET_PATH = "/tmp/websocket.sock"
class unix_socket_server:
    def __init__(self, ipc_use):
        self.conn = None
        if ipc_use == "PYTHONCLASSIFICATION":
            self.path = PYTHON_CLASSIFIER_COMMUNICATION_PATH
        elif ipc_use == "C":
            self.path = C_DRIVER_COMMUNICATION_PATH
        elif ipc_use == "PYTHONWEBSOCKET":
            self.path = PYTHON_WEBSOCKET_PATH
        else:
            print("INVALID PATH IN SHARED MEM:", ipc_use)
            exit(-1)

        if os.path.exists(self.path):
            os.remove(self.path)

        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.sock.bind(self.path)
        self.sock.listen(1)

    def block_accept(self):
        self.conn, _ = self.sock.accept()

    def _recvall(self, n):
        data = b""
        while len(data) < n:
            chunk = self.conn.recv(n - len(data))
            if not chunk:
                return None
            data += chunk
        return data

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
        raw_len = self._recvall(4)
        if not raw_len:
            return None

        msg_len = struct.unpack("!I", raw_len)[0]
        msg_bytes = self._recvall(msg_len)
        if not msg_bytes:
            return None

        return json.loads(msg_bytes.decode())


    def close(self):
        if self.conn:
            self.conn.close()
        if self.sock:
            self.sock.close()
        if os.path.exists(self.path):
            os.remove(self.path)
    
    def close_and_rebind(self):
        self.close()
        if os.path.exists(self.path):
            os.remove(self.path)
        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.conn = None
        self.sock.bind(self.path)
        self.sock.listen(1)


class unix_client:
    def __init__(self, ipc_use):
        if ipc_use == "PYTHONCLASSIFICATION":
            self.path = PYTHON_CLASSIFIER_COMMUNICATION_PATH
        elif ipc_use == "C":
            self.path = C_DRIVER_COMMUNICATION_PATH
        elif ipc_use == "PYTHONWEBSOCKET":
            self.path = PYTHON_WEBSOCKET_PATH
        else:
            print("INVALID PATH IN SHARED MEM:", ipc_use)
            exit(-1)

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
        if self.sock:
            self.sock.close()
