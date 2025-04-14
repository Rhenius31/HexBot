import rclpy
from rclpy.node import Node
import serial
import threading

class ESP32Logger(Node):
    def __init__(self):
        super().__init__('esp32_logger')
        self.get_logger().info('Starting ESP32 logger...')

        # ✅ Adjust your serial port if needed (e.g. /dev/ttyUSB0)
        try:
            self.ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
            self.get_logger().info('[ESP32]: Connected to /dev/ttyACM0')
        except serial.SerialException:
            self.get_logger().error('ESP32 not found on /dev/ttyACM0')
            return

        # ✅ Send the START command
        self.ser.write(b'START\n')
        self.get_logger().info('[ESP32]: START command sent')

        # ✅ Start thread to read ESP32 serial logs
        self.read_thread = threading.Thread(target=self.read_serial, daemon=True)
        self.read_thread.start()

    def read_serial(self):
        while rclpy.ok():
            try:
                line = self.ser.readline().decode(errors='ignore').strip()

                if line:
                    self.get_logger().info(f'[ESP32]: {line}')
            except Exception as e:
                self.get_logger().error(f'[ESP32 Serial Error]: {e}')
                break

def main(args=None):
    rclpy.init(args=args)
    node = ESP32Logger()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
