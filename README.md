# CarKeyControlEdgent_ESP32

## Overview
CarKeyControlEdgent_ESP32 is a project designed to enable users to control their car without a traditional key using an ESP32 and Blynk IoT platform. Developed specifically with the BYD Atto 3, this system provides functionalities for locking and unlocking the car, starting and stopping the engine, toggling the trunk, and viewing the car's status including engine state, door lock status, and state of charge (SoC) remaining.

## Blynk App
<br/>

Dashboard             |  Notification
:-------------------------:|:-------------------------:
<img src="assets/Blynk%20App%20Dashboard.png" />  |  <img src="assets/Blynk%20App%20Notifications.png" />
Charging             |  
<img src="assets/Blynk%20App%20Charging.gif" />  |  

## To-Do
- [ ] **Refactoring Code**
- [ ] **Switch to using a local Blynk server because there are no limitations.**

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

## Library
- [**Blynk** : 1.3.2](https://github.com/blynkkk/blynk-library)
- [**RemoteSerial** : 0.0.1](https://github.com/kangrio/RemoteSerial)
- [**ESP32-TWAI-CAN** : 1.0](https://github.com/handmade0octopus/ESP32-TWAI-CAN)
- [**AntiDelay version** : 1.1.3](https://github.com/martinvichnal/AntiDelay)
- [**pubsubclient** : 2.8](https://github.com/knolleary/pubsubclient)
- [**sinricpro** : 3.3.1](https://github.com/sinricpro/esp8266-esp32-sdk)

## Usage
1. **Connect OBD2 Dongle to OBD2 Port:**
2. **Control the car:**
    - Use the Blynk app to lock/unlock the car, start/stop the engine, and toggle the trunk.
    - View the car's status in real-time through the Blynk app.

## Blynk Setup
- Virtual Pin
  - V1:String = Car information
  - V4:Integer = Push buttons
    - Start/Stop button value { press: 1, release: 0 }
    - Lock button value { press: 3, release: 2 }
    - Unlock button value { press: 5, release: 4 }
    - Trunk button value { press: 7, release: 6 }
    

## Circuit Diagram
<br/>
<img src="Diagram/Circuit%20Diagram_schem.png"/>
<!-- ![Circuit Diagram](Diagram/Circuit%20Diagram_schem.png) -->

