# Puara Module Templates for PlatformIO

--- 

**Société des Arts Technologiques (SAT)**  
**Input Devices and Music Interaction Laboratory (IDMIL)**

--- 

This repository contains several platformIO templates to be used as a base to create devices that can be controlled over the network.

---

## Overview

Puara Module facilitates embedded system development by providing a set of pre-defined modules that manage filesystem, web server, and network connections so users can focus on prototyping the rest of their system.

### Why Use This?

This project is designed for artists and creators interested in:
- Developing distributed systems
- Creating interactive installations
- Building network-based projects
- Innovating digital instruments
- Designing new forms of creative interfaces

---

## How It Works

When initiating the program, the module manager will try to connect to the WiFi Network (SSID) defined in `config.json`. 

If you want the board to connect to a specific WiFi network, modify the `wifiSSID` and `wifiPSK` values in `config.json` with your network name and password respectively and then build/upload the filesystem. 

After the board connects to an external SSID, it will also create its own WiFi Access Point **(STA-AP mode)**. 

If the process cannot connect to a valid SSID, it will still create its own WiFi Access Point **(AP mode)** to which users may connect and communicate with the board.

User may modify/add custom values in `settings.json` and access them in their program at any moment by using the **puara.getVarText("name")** and/or **puara.getVarNumber("name")** for text or number fields respectively; make sure to respect the JSON *name/value* pairing. 

User may modify said values via the web server settings page and the defined values will persist even after shutting down/rebooting the system. 
This is very useful if you wish to have easily configurable variables without having to rebuild/reflash your entire system.

To access the web server, connect to the same network/SSID as the board is connected to, or connect to the board's WiFi access point, and enter the board's IP address in any web browser. 

User may also type the network name followed by `.local` in the browser's address bar. Default network name is `device`_`id` (see `config.json file`) : **Puara_001**. Hence type `puara_001.local` in the browser's address bar to access web server pages.

## How to Use

1. **Install VSCode and PlatformIO**: We recommend using [Visual Studio Code](https://code.visualstudio.com/) as code editor with the [PlatformIO](https://platformio.org/install/ide?install=vscode) IDE extension.

2. **Clone the `puara-module-templates` repository locally**: `git clone https://github.com/Puara/puara-module-templates.git`

3. **Open a puara template in VS Code**: Open VS Code, and select the platformIO extension on the left side. This will open the `PLATFORMIO` panel. From there select "Pick a folder", and navigate to one of the `puara-module-templates` subfolders, e.g., `puara-module-templates/basic/` (see below for a list of available templates). Click the "Select Folder" button. Wait for a bit while PlatformIO configures your project.
<p align="center">
  <img width="450" src="https://github.com/user-attachments/assets/1d87273a-c3e3-4d5b-890f-6fca498f09b5">
</p>

4. **Configure the board**: Ensure the `board` variable in the `platformio.ini` file matches your board's name. If needed you can find valid board IDs in [Boards](https://docs.platformio.org/en/latest/boards/index.html#boards) catalog, [Boards Explorer](https://registry.platformio.org/search?t=platform) or by using the [pio boards](https://docs.platformio.org/en/latest/core/userguide/cmd_boards.html#cmd-boards) terminal command.

5. **Edit the template**: You are now ready to edit the template according to your board/needs.

6. **Build and upload the file system and firmware**: Once ready, you can use PlatformIO to build and upload the file system and firmware to your board. You can access the `PLATFORMIO` Project Tasks by clicking on the extension button on the left. Make sure you upload both the filesystem (`Build`/`Upload Filesystem Image` under the `Platform` icon) and the firmware (`Build`/`Upload` under the `General` icon).
<p align="center">
  <img width="150" src="https://github.com/user-attachments/assets/d0254aa6-c1f2-400f-97c6-873a5597637b">
</p>

---

## Available Templates

This repository includes several PlatformIO templates that demonstrate different use cases and functionalities. Each template includes a `data/` folder containing configuration files (`config.json`, `settings.json`) and web interface files (HTML and CSS).

**After building and uploading the firmware to your board, you must also upload the filesystem** using PlatformIO's filesystem upload feature:

1. Access the `PLATFORMIO` Project Tasks by clicking on the extension button on the left
2. Make sure you upload both the filesystem (`Build`/`Upload Filesystem Image` under the `Platform` icon) and the firmware (`Build`/`Upload` under the `General` icon)

---

### 1. Basic Example

A minimal example demonstrating core Puara Module functionality. This template:
- Initializes the Puara module manager
- Reads custom settings from the JSON configuration files
- Outputs dummy sensor data to the serial monitor
- Demonstrates how to access custom configuration variables using `getVarText()` and `getVarNumber()`

This is the best starting point for learning how to use the Puara framework.

---

### 2. OSC-Send Example

Demonstrates how to set up a basic OSC transmitter. This template:
- Sends dummy sensor data as OSC messages to a specified IP address and port
- Configurable OSC IP/port via the web interface without rebuilding/reflashing
- Shows how to use the `onSettingsChanged()` callback to update parameters dynamically
- Includes example code for reading analog sensors and digital signals

**Note**: Please refer to [CNMAT's OSC repository](https://github.com/CNMAT/OSC) on GitHub for more details on OSC.

---

### 3. OSC-Receive Example

Demonstrates how to receive and process OSC messages. This template:
- Listens for incoming OSC messages on a configurable UDP port
- Parses OSC messages and extracts data from them
- Demonstrates example processing of float values to control device outputs (e.g., LED brightness)
- Shows how to use the `onSettingsChanged()` callback for dynamic configuration

The example expects a float between [0,1] on the OSC address `/led/brightness` with the format: `/led/brightness f 0.34`

**Note**: Please refer to [CNMAT's OSC repository](https://github.com/CNMAT/OSC) on GitHub for more details on OSC.

---

### 4. OSC-Duplex Example

Combines both OSC-Send and OSC-Receive functionality in a single sketch. This template:
- Sends dummy sensor data to a remote OSC address
- Simultaneously receives OSC messages from remote sources
- Demonstrates full duplex OSC communication patterns
- Useful for bidirectional device communication scenarios

**Note**: Please refer to [CNMAT's OSC repository](https://github.com/CNMAT/OSC) on GitHub for more details on OSC.

---

### 5. BLE Advertising Example

Demonstrates BLE (Bluetooth Low Energy) advertising without requiring device connections. This template:
- Uses BLE advertising to broadcast device information
- Encodes sensor data as CBOR payloads in BLE manufacturer data packets
- Broadcasts at configurable frequency (default 50Hz)
- Works seamlessly with the [BLE-CBOR-to-OSC script](https://gitlab.com/sat-mtl/collaborations/2024-iot/ble-cbor-to-osc)

For detailed setup instructions and configuration options, refer to the template's README file.

---

### 6. Basic Gestures Example

Extends the basic template with gesture recognition capabilities using an IMU (Inertial Measurement Unit). This template:
- Demonstrates how to read and process IMU sensor data
- Includes gesture detection logic
- Shows integration with the Puara module system

---

### 7. Button OSC Example

Demonstrates how to use button inputs with OSC messaging. This template:
- Reads digital button inputs
- Sends button state changes as OSC messages
- Shows event-driven communication patterns

---

### 8. Libmapper OSC Example

Demonstrates integration with libmapper, a distributed signal mapping system. This template:
- Uses libmapper to register signals that can be mapped remotely
- Combines libmapper with OSC messaging capabilities
- Shows how to use the Puara framework with advanced mapping scenarios

---


## References

Learn more about the research related to Puara: 

- [Puara GitHub Repository](https://github.com/Puara)
- [SAT](http://www.sat.qc.ca) // [SAT-R&D](https://sat.qc.ca/fr/recherche/)
- [IDMIL](http://www.idmil.org)

---

## Licensing

The code in this project is licensed under the MIT license, unless otherwise specified within the file.
