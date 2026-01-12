from RemoteOverride import remote_override

modeToString = ["manual", "remote"]
current_mode = 0
ip = "0.0"
port = 9000

if current_mode == 0:
    remote_override.connect_to_rover(ip, port, 20)