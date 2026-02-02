from picamera2 import Picamera2
from sharedMem import unix_socket_server
import time

cam = Picamera2()
config = cam.create_still_configuration({"size": (640, 640)})
cam.configure(config)

cam.start()
time.sleep(1)
i = 0
server = unix_socket_server(ipc_use="PYTHON")
while(1):
    mes = server.recv_message()
    if not mes or "type" not in mes:
        continue
    typ = mes["type"]
    if(typ == "READY"):
        array = cam.capture_array()
        server.send_image(array) 
    

server.close()
