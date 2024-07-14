# CarKeyControlEdgent_ESP32

## Overview
CarKeyControlEdgent_ESP32 is a project designed to enable users to control their car without a traditional key using an ESP32 and Blynk IoT platform. Developed specifically with the BYD Atto 3, this system provides functionalities for locking and unlocking the car, starting and stopping the engine, toggling the trunk, and viewing the car's status including engine state, door lock status, and state of charge (SoC) remaining.

## To-Do
- [ ] **Refactoring Code**
- [ ] **Moving to Local Blynk Server**

## Features
- [x] **Lock/Unlock:** Securely lock and unlock your car remotely.
- [x] **Start/Stop Engine:** Start or stop the car engine with a simple command.
- [x] **Toggle Trunk:** Open and close the trunk at your convenience.
- [x] **Car Status Monitoring:**
  - [x] **Engine Status:** Check if the engine is on or off.
  - [x] **Door Lock Status:** Verify whether the doors are locked.
  - [x] **Trunk Status:** Verify trunk close or oprn
  - [x] **State of Charge (SoC):** Monitor the remaining charge of the car's battery.

## Hardware Components

### Main Components
- **OBD2 Dongle:** Used for interfacing with the car's OBD-II port.
- **ESP32:** Microcontroller with Wi-Fi and Bluetooth capabilities for handling the main logic and communication.
- **LM2596:** DC-DC buck converter for voltage regulation.
- **SN65HVD230:** CAN bus transceiver for communication with the car's CAN network.
- **Original Keyfob:** Used for signal capture and emulation.

<!-- ### Additional Components -->

## Software Components
- **Blynk IoT App:** Provides the user interface for controlling the car and monitoring its status.
- **Arduino IDE:** Used to write and upload code to the ESP32.


## Installation

1. **Clone the repository:**
   ```bash
   git clone https://github.com/kangrio/CarKeyControlEdgent_ESP32.git
   cd CarKeyControlEdgent_ESP32

## Usage
1. **Connect OBD2 Dongle to OBD2 Port:**
2. **Control the car:**
    - Use the Blynk app to lock/unlock the car, start/stop the engine, and toggle the trunk.
    - View the car's status in real-time through the Blynk app.
    

## Circuit Diagram

![Circuit Diagram](Diagram/Circuit%20Diagram_schem.png)