from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='line_following',
            executable='esp32_controller',
            name='esp32_controller',
            output='screen'
        )
    ])
