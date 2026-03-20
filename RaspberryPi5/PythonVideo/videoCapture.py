from picamera2 import Picamera2
from sharedMem import unix_socket_server
import time
import select

cam = Picamera2()
config = cam.create_still_configuration({"size": (640, 640)})
cam.configure(config)

cam.start()
time.sleep(1)
i = 0
classification_server = unix_socket_server(ipc_use="PYTHONCLASSIFICATION")
websocket_server = unix_socket_server(ipc_use="PYTHONWEBSOCKET")
while(1):
    #this blocks until either connection is ready
    ready, _, _ = select.select([classification_server.conn, websocket_server.conn], [], [], None)
    ready_connection = None

    #Check classificaiton server first to ensure it gets priority
    if classification_server.conn in ready:
        mes = classification_server.recv_message()
        ready_connection = classification_server
    elif websocket_server.conn in ready:
        mes = websocket_server.recv_message()
        ready_connection = websocket_server
    if not mes or "type" not in mes:
        continue
    typ = mes["type"]
    if(typ == "READY"):
        array = cam.capture_array()
        ready_connection.send_image(array) 
    

classification_server.close()
websocket_server.close()
