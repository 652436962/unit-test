
NPU 不支持动态输入,需要输入固定并简化onnx模型

https://github.com/ultralytics/yolov5/releases/download/v6.0/yolov5s.onnx

sudo apt install python3-pip
pip install onnx-simplifier


python3 -m onnxsim yolov5s.onnx yolov5s-sim.onnx --overwrite-input-shape 1,3,640,640

