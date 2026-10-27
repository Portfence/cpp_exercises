from ultralytics import YOLO

model = YOLO("yolo26s.pt")  # Downloads the pretrained weights

for result in model.predict(
    source="resources/people_uhd60fps.mp4",
    classes=[0],          # Keep person detections
    imgsz=1920,
    conf=0.50,
    line_width=1,
    show_labels=False,
    show_conf=False,
    save=True,
    stream=True,
):
    pass
