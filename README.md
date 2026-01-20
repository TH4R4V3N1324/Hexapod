# Hexapod

This repository contains the complete design, control software and documentation for a six-legged walking robot (hexapod). The project combines robotics, kinematics, embedded systems, and motion planning to create a stable and adaptable walking platform.

---

## Table of Contents

- [About](#about)
- [Features](#features)
- [Installation](#installation)
  - [Hexapod](#hexapod)
  - [Receiver & Controller](#receiver--controller)
- [Usage](#usage)

---

## About

This project was developed to address the growing complexity in robotic control systems, where tightly coupled codebases often hinder scalability, debugging, and long-term maintainability. It is designed for developers, researchers, or engineers working with robotic platforms or automation systems who require a clean, extensible control framework that’s easy to understand and build upon.

The primary goal of the project was to emphasize modularity, ensuring that each component of the control software is clearly separated by function and responsibility. This modular design allows developers to quickly comprehend the structure of the system, make targeted changes without unintended side effects, and expand the system by adding new features or improving existing ones without rewriting the entire codebase.

To achieve this, the software is structured around a set of well-defined classes, each serving a distinct role:

- DataPacket Class: Acts as a container for incoming or outgoing data, ensuring that all communication between system components remains consistent, structured, and easily interpretable.

- Calculate Class: Handles all mathematical and logical computations required by the system, such as sensor data interpretation, decision-making algorithms, or movement planning.

- Move Class: Directly interfaces with the actuators or motors, translating high-level commands into specific movement instructions. It abstracts the hardware layer, allowing for easier adaptation across different platforms.

- Animation Class: Manages co-ordination of legs using the various gaits and modes to create walking and animation.

---

## Features

- 3 DOF per leg
- Inverse Kinematic engine for stable gait control
- Multiple gait patterns: tripod, ripple, wave
- Multiple control modes: normal, strafe, tilt
- Real-time control via Pimironi Servo2040
- Wireless control using custom controller and EspNOW protocol
- STL files for 3D-printing mechanical components

---

## Installation

Describe how to get your project running locally. Include prerequisites, dependencies, and step-by-step instructions.

Clone the repository into the desired location using the git bash terminal.
```bash
git clone https://github.com/TH4R4V3N1324/Hexapod.git
```

### Hexapod
Before getting started, make sure you have the following installed:

#### Prerequisites
- [Visual Studio Code](https://code.visualstudio.com/)
- [Pico SDK and cmake toolchain](https://github.com/raspberrypi/pico-sdk)
- [Raspberry Pi Pico VSCode Extension](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.raspberry-pi-pico)

#### Dependencies
- [Pimironi pico sdk](https://github.com/pimoroni/pimoroni-pico.git)

Using the Raspberry Pi Pico VSCode extension, import the Hexapod sub-folder by selecting it in the **Location** tab, enabling **CMake-Tools** and clicking **Import**. 

<img width="941" height="602" alt="image" src="https://github.com/user-attachments/assets/8c47e7c1-295a-499b-b6f6-cfa090426399" />

A selection box will appear at the top of the screen. From the list, choose the appropriate Pico compiler.

<img width="609" height="139" alt="image" src="https://github.com/user-attachments/assets/6cdaf052-d8a2-43bf-9610-47753c51e325" />

Once imported, you can build the project by selecting **Compile Project** from the Pico extension commands. This will generate a .uf2 firmware file inside the build directory.

To flash the firmware to the Pimoroni Servo 2040, put the board into BOOTSEL mode (by holding the BOOT button and pressing RESET), then copy the .uf2 file to the mounted USB storage.

Alternatively, you can enter BOOTSEL mode first, then simply use the **Run Project** command. This will automatically build and flash the firmware to the board, no manual file transfer required.

<img width="173" height="45" alt="image" src="https://github.com/user-attachments/assets/cd616e4c-6d4b-4c53-86ff-5c158eb07fa7" />

### Receiver & Controller

#### Prerequisites
- [PlatformIO VSCode Extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)

#### Dependencies
- [u8g2 library](https://github.com/olikraus/u8g2.git)

Using the PlatformIO extension in VS Code, select the **Open** option under the **Quick Access** panel to launch the PlatformIO Home page. From there, click **Open Project** and navigate to the appropriate sub-folder to import your project.

<img width="1267" height="654" alt="image" src="https://github.com/user-attachments/assets/aab1096a-94ee-453e-a0cc-c572583a5a59" />

To build or upload the project to your ESP32 board, use the **Build** or **Upload** options available under the **Project Tasks** panel in the PlatformIO extension.

<img width="391" height="490" alt="image" src="https://github.com/user-attachments/assets/9143a6cc-68e3-45dd-ba03-e871e0463958" />

---

## Usage

Explain how to use the project. Provide code snippets, screenshots, or examples as helpful.

# Ideas:
# Use ROS2 for control
<img width="475" height="585" alt="image" src="https://github.com/user-attachments/assets/5df6aec1-c747-4ea1-bb06-a8412053d7d5" />
<img width="520" height="395" alt="image" src="https://github.com/user-attachments/assets/ef33b528-e2d2-4559-9521-a9967716f75a" />

## Package Structure

### 1. hexapod_description (URDF/Configuration)

- Robot URDF model with leg geometry
- Joint limits and physical parameters
- Launch files for simulation and hardware

### 2. hexapod_hardware_interface (Hardware Layer)

#### Servo Controller Node: Interfaces with PCA9685 or direct PWM

- Subscribes to: /joint_commands (JointState)
- Publishes: /joint_states (JointState with feedback)
- Service: /emergency_stop

#### Sensor Manager Node: Aggregates all sensor data

- Publishes: /imu/data (sensor_msgs/Imu)
- Publishes: /power/status (custom PowerStatus msg)
- Publishes: /leg_contact (custom LegContactArray msg)

### 3. hexapod_kinematics (Computation Layer)

#### IK/FK Solver Node: Your Calculate class logic
- Service: /ik_solve (Vector3 → JointAngles)
- Service: /fk_solve (JointAngles → Vector3)
- Publishes: /leg_poses (geometry_msgs/PoseArray

### 4. hexapod_gait_controller (Motion Planning)

#### Gait Generator Node: Your Animation/Move logic

- Subscribes to: /cmd_vel (Twist for velocity commands)
- Subscribes to: /gait_mode (String: tripod/ripple/wave)
- Publishes: /leg_trajectories (trajectory_msgs/JointTrajectory)
- Action Server: /execute_gait for complex movements

### 5. hexapod_control (High-Level Control)

#### Behavior Manager Node: State machine (Normal/Strafe/Tilt/Config)

- Subscribes to: /mode_command (custom ModeCommand)
- Subscribes to: /joystick (sensor_msgs/Joy)
- Publishes: /cmd_vel (Twist)
- Service: /set_height, /home_stance

#### Config Manager Node: Servo offset calibration

- Reads/writes to parameter server
- Service: /save_config, /load_config

### 6. hexapod_teleop (User Interface)

#### Joystick Node: Replaces your ESP-NOW controller

- Uses joy_node from ROS2
- Publishes: /joystick (sensor_msgs/Joy)

#### Web Interface Node: Optional dashboard

- rqt plugins or custom web interface
- Monitor battery, IMU, leg contact

# Use Isaac sim for simulation
