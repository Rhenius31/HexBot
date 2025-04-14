import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from std_msgs.msg import String
from cv_bridge import CvBridge
import cv2
import numpy as np
from ament_index_python.packages import get_package_share_directory
import os
from ultralytics import YOLO

class YoloColorDetectorNode(Node):
    def __init__(self):
        super().__init__('yolo_detect_ros')

        # Load model
        package_path = get_package_share_directory('yolo_color_detector')
        model_path = os.path.join(package_path, 'my_model.pt')

        self.model = YOLO(model_path)
        self.labels = self.model.names
        self.get_logger().info("YOLO model loaded.")

        self.bridge = CvBridge()
        self.subscription = self.create_subscription(Image, '/camera/image_raw', self.image_callback, 10)
        self.result_pub = self.create_publisher(String, '/detected_objects', 10)

        self.last_detected_color = None  # 🆕 Track previous color

    def image_callback(self, msg):
        frame = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')
        results = self.model(frame, verbose=False)
        detections = results[0].boxes
        detected_color = None

        for i in range(len(detections)):
            classidx = int(detections[i].cls.item())
            classname = self.labels[classidx]  # e.g. 'red_bottle'
            conf = detections[i].conf.item()

            if conf > 0.5:
                color = classname.split('_')[0].upper()
                if color in ['RED', 'BLUE', 'YELLOW', 'BLACK', 'WHITE']:
                    detected_color = color
                    break

        # Only publish if it's a new color
        if detected_color and detected_color != self.last_detected_color:
            msg = String()
            msg.data = detected_color
            self.result_pub.publish(msg)
            self.get_logger().info(f"Published NEW color: {detected_color}")
            self.last_detected_color = detected_color

        # If nothing is detected, reset last_detected_color (optional)
        elif detected_color is None:
            self.last_detected_color = None


def main(args=None):
    rclpy.init(args=args)
    node = YoloColorDetectorNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()
