from sharedMem import unix_client
from PIL import Image
from ultralytics import YOLO

pythonConn = unix_client(ipc_use="PYTHONCLASSIFICATION")
cConn = unix_client(ipc_use="C")
model = YOLO("best.pt")
model.fuse()
results = []
id = 0

import struct

FMT = "<ffI" # two floats
SIZE = struct.calcsize(FMT)


while(1):
    try:
        pythonConn.send_ready()
        image = pythonConn.recv_image()
    except:
        print("videoCapture Closed, exiting...")
        break
    result = model.predict(
        image,
        imgsz=320,
        conf=0.5,
        verbose=False
    )
    if len(result[0].boxes) < 1:
        id = 0
    else:
        id += 1

    for box in result[0].boxes:
        if box.conf > 0.7:
            cx_n, cy_n, w_n, h_n = box.xywhn[0].tolist()

            #remove after training with negatives
            if w_n > 0.7 or h_n > 0.7:
                id = 0
                continue

            # direc = "r" if cx_n > 0.5 else "l"
            # direc += "d" if cy_n > 0.5 else "u"
            

            data = struct.pack(FMT, cx_n, cy_n, id)
            cConn.send_struct(data)
            break
            # results.append((direc, result[0]))
    # img = Image.fromarray(image, 'RGB')
    # img.save(f'tstImgs/output_image{i}.png') 

# for i, dir_result in enumerate(results):
#     dir_result[1].save(filename=f"./tstImgs/{dir_result[0]}{i}.jpg")