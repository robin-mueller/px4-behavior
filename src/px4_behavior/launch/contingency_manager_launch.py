import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    IncludeLaunchDescription,
    RegisterEventHandler,
    ExecuteProcess,
    LogInfo,
)
from launch.substitutions import FindExecutable
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.event_handlers import OnProcessStart, OnProcessIO
from launch_ros.actions import ComposableNodeContainer, Node
from launch_ros.descriptions import ComposableNode

NAMESPACE = ""


def upload_tree_action(executor_name: str, package_name: str, filename: str):
    process_name = f"{executor_name}_upload_tree"
    execute = ExecuteProcess(
        cmd=[
            [
                FindExecutable(name="ros2"),
                f" run uas_behavior upload_tree '{NAMESPACE}' '{executor_name}' '{package_name}' '{filename}'",
            ]
        ],
        name=process_name,
        shell=True,
        emulate_tty=True,
    )
    return [
        execute,
        RegisterEventHandler(
            OnProcessIO(
                target_action=execute,
                on_stdout=lambda event: LogInfo(
                    msg=f"[{process_name}] {event.text.decode().strip()}"
                ),
            )
        ),
    ]


def generate_launch_description():
    bt_executor_launch = IncludeLaunchDescription(
        launch_description_source=PythonLaunchDescriptionSource(
            [
                os.path.join(
                    get_package_share_directory("commander"),
                    "launch",
                    "maneuver_launch.py",
                ),
            ]
        ),
        launch_arguments=[("namespace", NAMESPACE)],
    )

    container = ComposableNodeContainer(
        name="ctmanager_container_node",
        exec_name="ctmanager_container",
        namespace=NAMESPACE,
        package="rclcpp_components",
        executable="component_container_mt",  # Multithreading is important due to parallel spin until operations of bt executors
        composable_node_descriptions=[
            ComposableNode(
                package="px4_behavior",
                namespace=NAMESPACE,
                plugin="px4_behavior::FlightOrchestratorExecutor",
                parameters=[{"state_change_logging": False}],
            ),
            ComposableNode(
                package="px4_behavior",
                namespace=NAMESPACE,
                plugin="px4_behavior::SafetyMonitorExecutor",
                parameters=[{"state_change_logging": False}],
            ),
            ComposableNode(
                package="px4_behavior",
                namespace=NAMESPACE,
                plugin="px4_behavior::MissionManagerExecutor",
                parameters=[{"state_change_logging": False}],
            ),
            ComposableNode(
                package="px4_behavior",
                namespace=NAMESPACE,
                plugin="px4_behavior::ContingencyManagerExecutor",
                parameters=[{"state_change_logging": False}],
            ),
        ],
        output="screen",
        emulate_tty=True,
    )

    # Register default tree for orchestrator
    upload_orchestrator_tree = upload_tree_action(
        "flight_orchestrator", "px4_behavior", "flight_orchestrator"
    )

    return LaunchDescription(
        [
            bt_executor_launch,
            container,
            RegisterEventHandler(
                OnProcessStart(
                    target_action=container,
                    on_start=upload_orchestrator_tree,
                )
            ),
        ]
    )