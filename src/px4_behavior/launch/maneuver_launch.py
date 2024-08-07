from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    namespace_launch_arg = DeclareLaunchArgument(
        "namespace", description="namespace for the maneuver nodes", default_value=""
    )

    namespace = LaunchConfiguration("namespace")

    container = ComposableNodeContainer(
        name="maneuver_container_node",
        exec_name="maneuver_container",
        namespace=namespace,
        package="rclcpp_components",
        executable="component_container",
        composable_node_descriptions=[
            ComposableNode(
                package="px4_behavior",
                namespace=namespace,
                plugin="px4_behavior::ArmDisarmManeuver",
            ),
            ComposableNode(
                package="px4_behavior",
                namespace=namespace,
                plugin="px4_behavior::EnableHoldManeuver",
            ),
            ComposableNode(
                package="px4_behavior",
                namespace=namespace,
                plugin="px4_behavior::GoToManeuver",
            ),
            ComposableNode(
                package="px4_behavior",
                namespace=namespace,
                plugin="px4_behavior::LandManeuver",
            ),
            ComposableNode(
                package="px4_behavior",
                namespace=namespace,
                plugin="px4_behavior::RTLManeuver",
            ),
            ComposableNode(
                package="px4_behavior",
                namespace=namespace,
                plugin="px4_behavior::TakeoffManeuver",
            ),
            ComposableNode(
                package="px4_behavior",
                namespace=namespace,
                plugin="px4_behavior::MissionManeuver",
            ),
        ],
        output="screen",
        emulate_tty=True,
    )

    return LaunchDescription([namespace_launch_arg, container])
