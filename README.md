# Autonomous Rover
For video of the rover see my LinkedIn Post: 
## Overview

Autonomous rover to scan for drones and fire a laser at them.  Automated scanning logic and a manual override from a remote connection.

## AI Usage

The goal of thjis project was to learn robots/low level programming, so the UI logic in RemoteConnectionClient are AI written.

## Improvements

1. This was my first time using embedded communication methods, initially SPI seemed like the smart decision, but I believe SPI was not the best option here.

2. Hardware was a big limitation, it would have been cool do use more advanced hardware like motors with encoders, gyroscope, accelerometer, etc.  Additionally the 3D printing definitely could have been better.

3. Telemetry data and pausing cv inference if manual control

Lots more minor improvements, but I believe I am past the point of diminishing returns where the time put in is not worth the additional learning as I near the end.

## Repository Structure

```text
AutonomousRover/
├── StmNoHAL/                 # STM32 firmware project
├── RaspberryPi5/             # Pi-side C + Python CV 
├── RemoteConnectionClient/   # Desktop client for video + manual control
```

## Components

### STM32 Firmware

Path: `StmNoHAL/`

The firmware in `StmNoHAL` handles:

- motor direction control
- servo positioning for camera movement
- HC-SR04 distance measurement
- SPI message handling from the Raspberry Pi
- outbound sensor info such as servo position and distance

### Raspberry Pi Runtime

Path: `RaspberryPi5/`

The Pi-side system includes:

- `driver.c`: main coordinator process
- `STM32Communication/`: SPI communication with the STM32
- `ExternalConnection/ManualControl/`: manual-control server
- `PythonCIPC/`: C/Python IPC layer
- `PythonVideo/videoCapture.py`: camera capture
- `PythonVideo/videoClassification.py`: YOLO-based target detection
- `PythonVideo/externalConnection.py`: websocket video streaming server
- `start_rover.sh`: tmux-based launcher for the Pi services

### Remote Client

Path: `RemoteConnectionClient/`

- Connect to rover video stream and if desired you can take over control of the entire rover


## Controls

Based on the current client code:

- `W` move forward
- `A` turn left
- `S` move backward
- `D` turn right
- `Up` camera up
- `Left` camera left
- `Right` camera right
- `Down` camera down
- `Space` fire
- `Q` switch out of manual mode
- `P` save the latest video frame



