import tkinter as tk
from datetime import datetime
from tkinter import ttk
from io import BytesIO
from pathlib import Path
from queue import Empty, Queue

from PIL import Image, ImageTk

from connection_to_rover import RoverConnection


class RoverInterface(tk.Tk):
    VIDEO_WIDTH = 780
    VIDEO_HEIGHT = 440
    SNAPSHOT_DIR = Path(__file__).resolve().parent / "saved_frames"
    MANUAL_KEY_COMMANDS = {
        "w": b"mF",
        "a": b"mL",
        "s": b"mB",
        "d": b"mR",
        "Up": b"lU",
        "Left": b"lL",
        "Right": b"lR",
        "Down": b"lD",
        "space": b"f",
        "q": b"s"
    }

    def __init__(self) -> None:
        super().__init__()
        self.title("Rover Interface")
        self.geometry("800x600")
        self.resizable(False, False)

        self.ip_address = tk.StringVar()
        self.status_text = tk.StringVar(value="STATUS:DISCONNECTED")
        self.manual_status_text = tk.StringVar(value="MANUAL:DISCONNECTED")
        self.frame_queue: Queue[bytes] = Queue(maxsize=1)
        self.connection: RoverConnection | None = None
        self.current_frame = None
        self.latest_frame_bytes: bytes | None = None
        self.manual_keys_bound = False

        self._build_ui()
        self._bind_app_keys()
        self.after(30, self._render_video_frame)
        self.protocol("WM_DELETE_WINDOW", self._on_close)

    def _build_ui(self) -> None:
        container = ttk.Frame(self, padding=10)
        container.pack(fill="both", expand=True)

        self.video_container = tk.Frame(
            container,
            width=self.VIDEO_WIDTH,
            height=self.VIDEO_HEIGHT,
            bg="black",
            relief="sunken",
            bd=1,
        )
        self.video_container.pack(fill="x")
        self.video_container.pack_propagate(False)

        self.video_feed = tk.Label(
            self.video_container,
            text="Video Feed",
            anchor="center",
            bg="black",
            fg="white",
        )
        self.video_feed.pack(fill="both", expand=True)

        ip_row = ttk.Frame(container, padding=(0, 18, 0, 0))
        ip_row.pack(anchor="w")

        ttk.Label(ip_row, text="IP Address:").pack(side="left")

        self.ip_entry = ttk.Entry(ip_row, textvariable=self.ip_address, width=18)
        self.ip_entry.pack(side="left", padx=(8, 0))

        self.connect_button = ttk.Button(
            ip_row,
            text="Connect Video",
            command=self._toggle_video_connection,
        )
        self.connect_button.pack(side="left", padx=(12, 0))

        self.manual_button = ttk.Button(
            ip_row,
            text="Connect Manual",
            command=self._toggle_manual_connection,
        )
        self.manual_button.pack(side="left", padx=(12, 0))

        self.status_label = ttk.Label(container, textvariable=self.status_text)
        self.status_label.pack(anchor="w", pady=(10, 0))

        self.manual_status_label = ttk.Label(container, textvariable=self.manual_status_text)
        self.manual_status_label.pack(anchor="w", pady=(4, 0))

    def _ensure_connection(self) -> RoverConnection | None:
        ip = self.ip_address.get().strip()
        if not ip:
            self.status_text.set("STATUS:MISSING_IP")
            return None

        if self.connection is not None and self.connection.ip == ip:
            return self.connection

        if self.connection is not None:
            self.connection.disconnect()
            self._set_manual_disconnected()
            self._set_video_disconnected()

        self.connection = RoverConnection(ip=ip)
        return self.connection

    def _toggle_video_connection(self) -> None:
        connection = self._ensure_connection()
        if connection is None:
            return

        if connection.video_thread is not None and connection.video_thread.is_alive():
            connection.stop_video_stream()
            self._set_video_disconnected()
            self._clear_video_frame()
            self.video_feed.configure(image="", text="Video Feed")
            self.current_frame = None
            self.latest_frame_bytes = None
            return

        self.status_text.set("STATUS:VIDEO_STARTING")
        connection.start_video_stream(
            on_frame=self._queue_video_frame,
            on_status=self._update_status_from_thread,
        )

    def _toggle_manual_connection(self) -> None:
        connection = self._ensure_connection()
        if connection is None:
            return

        if connection.manual_socket is not None:
            connection.disconnect_manual()
            self._set_manual_disconnected()
            return

        try:
            connection.connect_manual()
        except Exception as exc:
            self.manual_status_text.set(f"MANUAL:ERROR:{exc}")
            return

        self._set_manual_connected()
        self.focus_set()

    def _queue_video_frame(self, frame: bytes) -> None:
        self.latest_frame_bytes = frame

        while not self.frame_queue.empty():
            try:
                self.frame_queue.get_nowait()
            except Empty:
                break

        self.frame_queue.put_nowait(frame)

    def _update_status_from_thread(self, status: str) -> None:
        self.after(0, self._handle_video_status, status)

    def _handle_video_status(self, status: str) -> None:
        self.status_text.set(status)
        if status == "STATUS:VIDEO_CONNECTED":
            self.connect_button.configure(text="Disconnect Video")
            return

        if status.startswith("STATUS:VIDEO_"):
            if status in {"STATUS:VIDEO_DISCONNECTED"} or status.startswith("STATUS:VIDEO_ERROR:"):
                self._clear_video_frame()
                self.video_feed.configure(image="", text="Video Feed")
                self.current_frame = None
                self.latest_frame_bytes = None
            self._set_video_disconnected()

    def _set_video_disconnected(self) -> None:
        self.connect_button.configure(text="Connect Video")

    def _set_manual_connected(self) -> None:
        self.manual_button.configure(text="Disconnect Manual")
        self.manual_status_text.set("MANUAL:CONNECTED")
        self._bind_manual_keys()

    def _set_manual_disconnected(self) -> None:
        self.manual_button.configure(text="Connect Manual")
        self.manual_status_text.set("MANUAL:DISCONNECTED")
        self._unbind_manual_keys()

    def _bind_manual_keys(self) -> None:
        if self.manual_keys_bound:
            return

        self.manual_keys_bound = True

    def _unbind_manual_keys(self) -> None:
        if not self.manual_keys_bound:
            return

        self.manual_keys_bound = False

    def _bind_app_keys(self) -> None:
        self.bind_all("<KeyPress>", self._handle_keypress)

    def _handle_keypress(self, event: tk.Event) -> None:
        key = event.keysym.lower()
        if key == "p":
            self._save_latest_frame()
            return

        self._handle_manual_keypress(event)

    def _handle_manual_keypress(self, event: tk.Event) -> None:
        if (
            not self.manual_keys_bound
            or self.connection is None
            or self.connection.manual_socket is None
        ):
            return

        command = self.MANUAL_KEY_COMMANDS.get(event.keysym)
        if command is None:
            command = self.MANUAL_KEY_COMMANDS.get(event.keysym.lower())
        if command is None:
            return

        try:
            self.connection.send_manual_command(command)
        except Exception as exc:
            self.manual_status_text.set(f"MANUAL:ERROR:{exc}")
            self._set_manual_disconnected()

    def _save_latest_frame(self) -> None:
        if self.latest_frame_bytes is None:
            self.status_text.set("STATUS:NO_FRAME_TO_SAVE")
            return

        self.SNAPSHOT_DIR.mkdir(parents=True, exist_ok=True)
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
        output_path = self.SNAPSHOT_DIR / f"frame_{timestamp}.jpg"

        try:
            output_path.write_bytes(self.latest_frame_bytes)
        except Exception as exc:
            self.status_text.set(f"STATUS:SAVE_ERROR:{exc}")
            return

        self.status_text.set(f"STATUS:FRAME_SAVED:{output_path.name}")

    def _clear_video_frame(self) -> None:
        while not self.frame_queue.empty():
            try:
                self.frame_queue.get_nowait()
            except Empty:
                break

    def _render_video_frame(self) -> None:
        try:
            frame = self.frame_queue.get_nowait()
        except Empty:
            self.after(30, self._render_video_frame)
            return

        try:
            image = Image.open(BytesIO(frame))
            width = max(self.video_container.winfo_width(), self.VIDEO_WIDTH)
            height = max(self.video_container.winfo_height(), self.VIDEO_HEIGHT)
            image.thumbnail((width, height))
            self.current_frame = ImageTk.PhotoImage(image)
            self.video_feed.configure(image=self.current_frame, text="")
        except Exception as exc:
            self.status_text.set(f"STATUS:FRAME_ERROR:{exc}")

        self.after(30, self._render_video_frame)

    def _on_close(self) -> None:
        if self.connection is not None:
            self.connection.disconnect()
        self._unbind_manual_keys()
        self.destroy()


if __name__ == "__main__":
    app = RoverInterface()
    app.mainloop()
