from picamera2 import Picamera2
from sharedMem import unix_socket_server
import time
import select

classification_server = None
websocket_server = None
cam = None

def start_camera():
    global cam 
    cam = Picamera2()
    config = cam.create_still_configuration({"size": (640, 640)})
    cam.configure(config)
    cam.start()

def connect_to_classification():
    global classification_server
    classification_server = unix_socket_server(ipc_use="PYTHONCLASSIFICATION")
    classification_server.block_accept()

start_camera()
connect_to_classification()

websocket_server = unix_socket_server(ipc_use="PYTHONWEBSOCKET")
possible_messages = [classification_server.conn, websocket_server.conn]
while(1):
    if not websocket_server.conn:
        possible_messages[1] = websocket_server.sock
    #this blocks until either connection is ready
    ready, _, _ = select.select(possible_messages, [], [], None)
    ready_connection = None

    #Check classificaiton server first to ensure it gets priority
    if classification_server.conn in ready:
        mes = classification_server.recv_message()
        if not mes:
            print("CLASSIFICATION CONNECTION DISCONNCTED, BLOCKING WHILE RECONNECTING")
            classification_server.close()
            connect_to_classification()
            continue
        ready_connection = classification_server

    elif websocket_server.conn in ready:
        mes = websocket_server.recv_message()
        if not mes:
            print("client disconnected")
            websocket_server.conn = None
            continue
        ready_connection = websocket_server
    elif websocket_server.sock in ready:
        websocket_server.block_accept()
        possible_messages[1] = websocket_server.conn
        continue

    if not mes or "type" not in mes:
        continue
    typ = mes["type"]
    if(typ == "READY"):
        array = cam.capture_array()
        ready_connection.send_image(array) 
    

classification_server.close()
websocket_server.close()
