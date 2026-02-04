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

## How to Use Puara Module Templates with PlatformIO

1. **Install VSCode and PlatformIO**: Use [Visual Studio Code](https://code.visualstudio.com/) as code editor with the [PlatformIO](https://platformio.org/install/ide?install=vscode) IDE extension.

2. **Clone the `puara-module-templates` repository locally**: `git clone https://github.com/Puara/puara-module-templates.git`

3. **Open a puara template in VS Code**: Open VS Code, and select the platformIO extension on the left side. This will open the `PLATFORMIO` panel. From there select "Pick a folder", and navigate to one of the `puara-module-templates` subfolders, e.g., `puara-module-templates/basic/` (see below for a list of available templates). Click the "Select Folder" button. Wait for a bit while PlatformIO configures your project.
<p align="center">
  <img width="450" src="https://github.com/user-attachments/assets/1d87273a-c3e3-4d5b-890f-6fca498f09b5">
</p>

4. **Configure the board**: Ensure the `board` variable in the `platformio.ini` file matches your board's name. If needed you can find valid board IDs in [Boards](https://docs.platformio.org/en/latest/boards/index.html#boards) catalog, [Boards Explorer](https://registry.platformio.org/search?t=platform) or by using the [pio boards](https://docs.platformio.org/en/latest/core/userguide/cmd_boards.html#cmd-boards) terminal command.

5. **Edit the template**: You are now ready to edit the template according to your board/needs.

6. **Build and upload the filesystem and firmware**: Once ready, you can use PlatformIO to build and upload the filesystem and firmware to your board. You can access the `PLATFORMIO` Project Tasks by clicking on the extension button on the left. Make sure you upload both the filesystem (`Build`/`Upload Filesystem Image` under the `Platform` icon) and the firmware (`Build`/`Upload` under the `General` icon).
<p align="center">
  <img width="150" src="https://github.com/user-attachments/assets/d0254aa6-c1f2-400f-97c6-873a5597637b">
</p>



> #### ⚠️ **Important detail for users**
> Most embedded projects only upload the **code** that runs on the device. However, in this project, the device also needs a **filesystem** to store important data, such as configuration files, templates, or other resources that the code relies on. These two parts—**code** and **filesystem**—serve different purposes and must be built and uploaded separately.
> The **executable code** tells the device what to do, includes the logic, instructions, and behavior of the device such as how to read a sensor, process data, or send information over Wi-Fi.
> The **filesystem** is like a "hard drive" for the device, where additional files are stored and can include configuration files, templates, or other resources that the code needs to function properly. In our approach, the filesystem stores a JSON file with user settings for the network configurations and some global variables that can be modified through the browser without needing to reflash the whole system.
>


---

## How It Works


**Every template related to Puara Module has a different set of options but they all generally respect the following explanation.**
The following sections are detailed more thoroughly in the [Puara Module](https://github.com/Puara/puara-module) doumentation.

### 1. Establishing WiFi

When initiating the program, the module manager will try to connect to the WiFi Network (SSID) defined in `config.json`.
The `puara-module` supports three modes of operation:

1. **Station - Access Point (STA-AP) Mode** (Default):
   - The device connects to an existing WiFi network (station).
   - The device creates its own WiFi network (access point).
   - User has two ways to communicate with the board.

2. **Access Point (AP) Mode**:
   - The device does not connect to an existing WiFi network.
   - The device creates its own WiFi network (access point).

3. **Station (STA) Mode**:
   - The device connects to an existing WiFi network.
   - The Access Point is turned off with `persistent_AP=0`
   - Useful to limit Wifi pollution and securing device.


### 2. Making the Web Server Accessible

Browser-accessible pages available for configuring, scanning, and managing settings on your device are made availabe through Puara Module.

Once the web server is running, you can access it in two ways:

1. **Via IP Address**: Navigate to the device's IP address in your web browser (e.g., `http://192.168.4.1` for AP mode, or the assigned IP in STA/STA-AP modes which can be retrieved using `puara.staIP()`).

2. **Via mDNS Hostname**: If mDNS is enabled, you can access the device using its hostname (e.g., `http://your-device-name.local`). Default `config.json`values enable mDNS with the Puara_001, and browser is accessible with `puara_001.local`.

Using the web browser, users can modify variables that keep their value after reboot/shutdown of device without needing to rebuild/upload their program.
Access these values in the program by using:

```cpp
// For text fields
std::string my_string = puara.getVarText("variable_name");

// For number fields (all numbers are `doubles` -- see JSON documentation for explanation)
double my_number = puara.getVarNumber("variable_name");
```

#### Making of Custom Variables in `settings.json`

The `/data/settings.json` file stores custom application settings as an array of name-value pairs:

```json
{
    "settings": [
        {
            "name": "user_defined_text",
            "value": "user defined value"
        },
        {
            "name": "variable3",
            "value": 12.345
        }
    ]
}
```
User may add/modify fields in this file and then upload the new filesystem in order to have a more custom device.

---

For more detailed documentation, please refer to the mdBook in the puara-module's github repository.

---


## Available Templates

The [puara-module-templates](https://github.com/Puara/puara-module-templates) repository includes multiple examples demonstrating different use cases and functionalities.
Each example includes a `data/` folder containing configuration files (`config.json`, `settings.json`) and web interface files (HTML and CSS).

 ⚠️ **After building and uploading the firmware to your board, you must also upload the filesystem**.

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

Extends the basic template with gesture recognition (Puara-Gestures) capabilities using a pseudo-IMU (Inertial Measurement Unit). This template:
- Demonstrates how to read and process IMU sensor data
- Includes gesture detection logic
- Shows integration with the Puara module system

---

### 7. Button OSC Example

Demonstrates how to use button inputs with puara gestures and OSC messaging. This template:
- Reads digital button inputs (simulated button in template cas be replaced a real button)
- Shows event-driven communication patterns
- Evaluates the user interaction with button to determine if button is being held, pressed once, twice, or three times in a row, and such...
- Sends button state changes as OSC messages

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
