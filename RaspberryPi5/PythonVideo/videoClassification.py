from sharedMem import unix_client
from PIL import Image
from ultralytics import YOLO

conn = unix_client(ipc_use="PYTHON")
model = YOLO("yolov8n.pt")
i = 0
while(1):
    try:
        image = conn.recv_image()
    except:
        print("videoCapture Closed, exiting...")
        exit()
    result = model.predict(image)
    result[0].save(filename=f"../tstImgs/img{i}.jpg")
    # img = Image.fromarray(image, 'RGB')
    # img.save(f'tstImgs/output_image{i}.png')
    i += 1    

