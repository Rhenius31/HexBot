#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
import serial
from std_srvs.srv import Trigger
from std_msgs.msg import String


class ESP32SerialNode(Node):

    def __init__(self):
        super().__init__('esp32_serial_node')

        # Adjust this to match your USB port (e.g. /dev/ttyUSB0, /dev/ttyACM0, etc.)
        self.serial_port = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
        self.get_logger().info('ESP32 serial port opened on /dev/ttyACM0')

        # Create a service to trigger "START"
        self.srv = self.create_service(Trigger, 'send_start_command', self.send_start_callback)
        self.subscription = self.create_subscription(String,'/detected_objects',self.listener_callback,10)
        self.srv_stop = self.create_service(Trigger, 'send_stop_command', self.send_stop_callback)


    def listener_callback(self, msg):
       self.get_logger().info(f"Forwarding detection to ESP32: {msg.data}")
       try:
        self.serial_port.write((msg.data + '\n').encode())
       except Exception as e:
        self.get_logger().warn(f"Failed to write to ESP32: {e}")

    def send_start_callback(self, request, response):
        try:
            self.serial_port.write(b'START\n')
            self.get_logger().info('Sent START command to ESP32')
            response.success = True
            response.message = "START command sent"
        except Exception as e:
            response.success = False
            response.message = f"Failed to send START: {e}"
        return response
    def send_stop_callback(self, request, response):
        try:
            self.serial_port.write(b'STOP\n')
            self.get_logger().info('Sent STOP command to ESP32')
            response.success = True
            response.message = "STOP command sent"
        except Exception as e:
            response.success = False
            response.message = f"Failed to send STOP: {e}"
        return response

def main(args=None):
    rclpy.init(args=args)
    node = ESP32SerialNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
