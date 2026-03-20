class rover_connection:
    def __init__(self, ip, port):
        self.connected = 0
        self.ip = ip
        self.port = port