from __future__ import annotations

import socket
import threading
from typing import Callable, Optional

import websocket


class RoverConnection:
    def __init__(
        self,
        ip: str,
        manual_port: int = 8000,
        video_port: int = 8765,
    ) -> None:
        self.ip = ip
        self.manual_port = manual_port
        self.video_port = video_port
        self.manual_socket: Optional[socket.socket] = None
        self.video_socket = None
        self.video_thread: Optional[threading.Thread] = None
        self.stop_event = threading.Event()

    def connect_manual(self) -> None:
        if self.manual_socket is not None:
            return

        connection = socket.create_connection((self.ip, self.manual_port), timeout=5)
        self.manual_socket = connection

    def send_manual_command(self, payload: bytes) -> None:
        if self.manual_socket is None:
            raise RuntimeError("Manual control socket is not connected.")
        self.manual_socket.sendall(payload)

    def disconnect_manual(self) -> None:
        if self.manual_socket is None:
            return

        try:
            self.manual_socket.close()
        except Exception:
            pass

        self.manual_socket = None

    def start_video_stream(
        self,
        on_frame: Callable[[bytes], None],
        on_status: Optional[Callable[[str], None]] = None,
    ) -> None:
        if self.video_thread is not None and self.video_thread.is_alive():
            return

        self.stop_event.clear()

        def run_video_stream() -> None:
            status_callback = on_status or (lambda _: None)
            status_callback("STATUS:VIDEO_CONNECTING")
            had_error = False

            try:
                self.video_socket = websocket.create_connection(
                    f"ws://{self.ip}:{self.video_port}",
                    timeout=5,
                )
                status_callback("STATUS:VIDEO_CONNECTED")

                while not self.stop_event.is_set():
                    frame = self.video_socket.recv()
                    if isinstance(frame, bytes):
                        on_frame(frame)
                        continue

                    if isinstance(frame, str) and frame.startswith("stream-error:"):
                        raise RuntimeError(frame)
            except Exception as exc:
                had_error = True
                status_callback(f"STATUS:VIDEO_ERROR:{exc}")
            finally:
                if self.video_socket is not None:
                    try:
                        self.video_socket.close()
                    except Exception:
                        pass
                    self.video_socket = None

                if not self.stop_event.is_set() and not had_error:
                    status_callback("STATUS:VIDEO_DISCONNECTED")

        self.video_thread = threading.Thread(target=run_video_stream, daemon=True)
        self.video_thread.start()

    def stop_video_stream(self) -> None:
        self.stop_event.set()

        if self.video_socket is not None:
            try:
                self.video_socket.close()
            except Exception:
                pass
            self.video_socket = None

    def disconnect(self) -> None:
        self.stop_video_stream()

        if self.manual_socket is not None:
            self.disconnect_manual()
