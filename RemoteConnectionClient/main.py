from RemoteOverride import remote_override

modeToString = ["manual", "remote"]
current_mode = 0
ip = "192.168.1.199"
port = 8000

if current_mode == 0:
    remote_override.connect_to_rover(ip, port, 80)