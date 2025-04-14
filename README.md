This repository contains the ROS 2 nodes and utilities for controlling the HexBot: a trash-collecting, line-following robot powered by an ESP32 and YOLO color detection.

---

## Getting Started

To initialize the environment and workspace, run the following in order:

```bash
cd systems_ws/
source ./install/setup.bash
source /opt/ros/jazzy/setup.bash
```

---

## Launching ROS 2 Nodes (Open Each in a New Terminal Tab)

### 1. Camera Node
Launch the camera interface:

```bash
ros2 run line_following camera
```

### 2. YOLO Color Detection Node
Start YOLO-based object detection:

```bash
ros2 run yolo_color_detector yolo_detect_ros
```

### 3. ESP32 Controller Node
Run the ESP32 motor control node:

```bash
ros2 run line_following esp32_controller
```

---

## ESP32 Control Commands

### Send START Command
```bash
ros2 service call /send_start_command std_srvs/srv/Trigger
```

### Send STOP Command
```bash
ros2 service call /send_stop_command std_srvs/srv/Trigger
```

---

## View Camera Feed Without ROS Nodes

If you want to see the camera output directly, deactivate the ROS YOLO and camera nodes, then run:

```bash
cd 
cd yolo/
source ~/ros_env/bin/activate
python yolo_detect.py --model=my_model_ncnn_model --source=usb0 --resolution=1280x720
```

---

## View the status of the Bot

```bash
screen /dev/ttyACM0 115200
```
---

## Notes

- Make sure the USB camera is connected and accessible as `usb0`.
- Replace `my_model_ncnn_model` with the actual path to your model if needed.
- You may need to setup virtual environment before running the camera node to see live feed

---

