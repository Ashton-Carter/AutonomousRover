import socket
from pynput import keyboard
import time

pressed_keys = set()

def on_press(key):
    try:
        pressed_keys.add(key.char)
    except AttributeError:
        pass

def on_release(key):
    try:
        pressed_keys.discard(key.char)
    except AttributeError:
        pass

def connect_to_rover(ip, port, Hz):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, port))
    secondsSleep = 1/Hz
    print("Connected to server")


    listener = keyboard.Listener(
        on_press=on_press,
        on_release=on_release
    )
    listener.start()

    try:
        while True:
            command = None

            if 'w' in pressed_keys:
                command = "mF"
            elif 's' in pressed_keys:
                command = "mB"
            elif 'a' in pressed_keys:
                command = "mL"
            elif 'd' in pressed_keys:
                command = "mR"
            elif 'q' in pressed_keys:
                command = "s"

            if command:
                s.sendall((command + "\n").encode())

            if command == 's':
                break
            time.sleep(secondsSleep)

    except KeyboardInterrupt:
        print("Interrupted")

    s.close()
    print("Disconnected")

