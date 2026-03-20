import asyncio
from io import BytesIO

import websockets
from PIL import Image

from sharedMem import unix_client

class connection_to_user:
    def __init__(self, ip, port):
        self.ip = ip
        self.port = port
        self.server = None
        self.python_conn = None

    def _connect_to_video_capture(self):
        if self.python_conn is None:
            self.python_conn = unix_client(ipc_use="PYTHONWEBSOCKET")

    def _read_frame(self):
        self._connect_to_video_capture()
        self.python_conn.send_ready()
        return self.python_conn.recv_image()

    @staticmethod
    def _encode_frame(frame):
        buffer = BytesIO()
        Image.fromarray(frame).save(buffer, format="JPEG", quality=80)
        return buffer.getvalue()

    async def handler(self, websocket):
        while True:
            try:
                frame = await asyncio.to_thread(self._read_frame)
                encoded_frame = await asyncio.to_thread(self._encode_frame, frame)
                await websocket.send(encoded_frame)
                await asyncio.sleep(0.05)
            except websockets.ConnectionClosed:
                break
            except Exception as exc:
                await websocket.send(f"stream-error:{exc}")
                break

    async def connect_to_user(self):
        self.server = await websockets.serve(self.handler, self.ip, self.port)
        await self.server.wait_closed()

    def close(self):
        if self.python_conn is not None:
            self.python_conn.close()
            self.python_conn = None


async def main():
    server = connection_to_user("0.0.0.0", 8765)
    try:
        await server.connect_to_user()
    finally:
        server.close()


if __name__ == "__main__":
    asyncio.run(main())
