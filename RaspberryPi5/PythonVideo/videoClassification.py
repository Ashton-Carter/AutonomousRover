from sharedMem import unix_client
from ultralytics import YOLO
from pathlib import Path
import struct

pythonConn = unix_client(ipc_use="PYTHONCLASSIFICATION")
cConn = unix_client(ipc_use="C")

MODEL_PATH = Path(__file__).resolve().parent / "best_finetuned3.onnx"
model = YOLO(str(MODEL_PATH))

FMT = "<ffI"
SIZE = struct.calcsize(FMT)

detection_streak = 0

while True:
    try:
        pythonConn.send_ready()
        image = pythonConn.recv_image()
    except Exception as e:
        print(f"videoCapture closed or recv failed: {e}")
        break

    result = model.predict(
        image,
        imgsz=416,
        conf=0.5,
        verbose=False
    )

    boxes = result[0].boxes

    best_box = None
    best_conf = -1.0

    for box in boxes:
        conf = float(box.conf[0])
        if conf <= 0.7:
            continue

        cx_n, cy_n, w_n, h_n = box.xywhn[0].tolist()

        # temporary large-box filter
        if w_n > 0.7 or h_n > 0.7:
            continue

        if conf > best_conf:
            best_conf = conf
            best_box = (cx_n, cy_n)

    if best_box is None:
        detection_streak = 0
        continue

    detection_streak += 1
    cx_n, cy_n = best_box

    data = struct.pack(FMT, cx_n, cy_n, detection_streak)
    cConn.send_struct(data)
    print("Sending detection to C, ID:", detection_streak)