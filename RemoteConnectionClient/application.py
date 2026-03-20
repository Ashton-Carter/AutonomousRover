import tkinter as tk
from tkinter import ttk


class RoverInterface(tk.Tk):
    def __init__(self) -> None:
        super().__init__()
        self.title("Rover Interface")
        self.geometry("800x600")
        self.resizable(False, False)

        self.ip_address = tk.StringVar()
        self.status_text = tk.StringVar(value="STATUS:DISCONNECTED")

        self._build_ui()

    def _build_ui(self) -> None:
        container = ttk.Frame(self, padding=10)
        container.pack(fill="both", expand=True)

        self.video_feed = tk.Label(
            container,
            text="Video Feed",
            anchor="center",
            bg="black",
            fg="white",
            relief="sunken",
            width=97,
            height=20,
        )
        self.video_feed.pack(fill="x")

        ip_row = ttk.Frame(container, padding=(0, 18, 0, 0))
        ip_row.pack(anchor="w")

        ttk.Label(ip_row, text="IP Address:").pack(side="left")

        self.ip_entry = ttk.Entry(ip_row, textvariable=self.ip_address, width=18)
        self.ip_entry.pack(side="left", padx=(8, 0))

        self.status_label = ttk.Label(container, textvariable=self.status_text)
        self.status_label.pack(anchor="w", pady=(10, 0))


if __name__ == "__main__":
    app = RoverInterface()
    app.mainloop()
