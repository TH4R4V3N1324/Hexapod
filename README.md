# Hexapod

This repository contains the complete design, control software and documentation for a six-legged walking robot (hexapod). The project combines robotics, kinematics, embedded systems, and motion planning to create a stable and adaptable walking platform.

---

## Table of Contents

- [About](#about)
- [Features](#features)
- [Installation](#installation)
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
- [Git](https://git-scm.com/)

#### Dependencies
- [Pimironi pico sdk](https://github.com/pimoroni/pimoroni-pico.git)

Using the Raspberry Pi Pico VSCode extension, import the Hexapod sub-folder by selecting it in the Location tab and clicking Import.

<img width="942" height="592" alt="image" src="https://github.com/user-attachments/assets/354eea97-4b16-4b11-a991-5acd650c455d" />

Once imported, you can build the project by selecting Compile Project from the Pico extension commands. This will generate a .uf2 firmware file inside the build directory.

To flash the firmware to the Pimoroni Servo 2040, put the board into BOOTSEL mode (by holding the BOOT button and pressing RESET), then copy the .uf2 file to the mounted USB storage.

Alternatively, you can enter BOOTSEL mode first, then simply use the Run Project command. This will automatically build and flash the firmware to the board—no manual file transfer required.

### Receiver & Controller

#### Prerequisites
- [Arduino IDE](https://www.arduino.cc/en/software)

#### Dependencies
- [Arduino-esp32](https://github.com/espressif/arduino-esp32.git)
- [u8g2 library](https://github.com/olikraus/u8g2.git)

Open the Arduino IDE and navigate to File>Preferences. In the Additional boards managaer URLs tab paste in the esp32 package URL:

```bash
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

The Arduino-esp32 boards package by Espressif Systems can then be installed through the boards manager. Before uploading the code to either the Receiver or Controller, the appropriate Board and COM Port needs to be selected under the Tools tab.

---

## Usage

Explain how to use the project. Provide code snippets, screenshots, or examples as helpful.
