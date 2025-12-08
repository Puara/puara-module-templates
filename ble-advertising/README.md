# Puara BLE Advertising Template

This template demonstrates how to use BLE advertising to broadcast device information in CBOR format without requiring a connection to the BLE device. It is designed to work with the Puara module manager and the [BLE-CBOR-to-OSC script](https://gitlab.com/sat-mtl/collaborations/2024-iot/ble-cbor-to-osc), enabling seamless integration with OSC-based environments.

N.B : This is a hack on BLE advertisement which allows the BLE-CBOR-to_OSC script to get broadcast information without ever needing to connect to the device itself, allowing the use of hundreds of simultaneous BLE devices.

## Overview

The template uses the Puara module manager to initialize the ESP32 board, configure BLE advertising, and broadcast dummy sensor data sensor1 and sensor2 as CBOR-encoded payloads. These advertisements can be received and decoded by the BLE-CBOR-to-OSC script, which forwards the data as OSC messages to a specified IP address and port.
This dummy sensor data can be replaced by data from the pins on the board you are using to make it a useful data broadcast. Just initialize your pins in setup as you would do in setup() and get their data every loop in main(). 

### Key Features:
- BLE advertising of data at 50Hz.
- CBOR encoding of sensor data for efficient transmission.
- Integration with the Puara module manager for streamlined setup and configuration.
- Compatibility with the BLE-CBOR-to-OSC script for decoding and forwarding data as OSC messages.

## Prerequisites

- **Hardware**: ESP32 board (e.g., TinyPICO) with BLE support.
- **Software**:
  - [PlatformIO](https://platformio.org/) for building and uploading the firmware.
  - Python 3.8+ for running the BLE-CBOR-to-OSC script.

## Getting Started

### 1. Build and Upload the Firmware

1. Clone this repository and open it in your preferred IDE (e.g., VS Code with PlatformIO extension or Arduino IDE 2.0).
2. Build and upload the firmware to your ESP32 board.

### 2. Run the BLE-CBOR-to-OSC Script

The BLE-CBOR-to-OSC script listens for BLE advertisements, decodes the CBOR data, and forwards it as OSC messages. Follow these steps to set it up:

1. Clone the [BLE-CBOR-to-OSC repository](https://gitlab.com/sat-mtl/collaborations/2024-iot/ble-cbor-to-osc).
2. Navigate to the repository folder:
    ```bash
    cd ble-cbor-to-osc/
    ```
3. Create and activate a Python virtual environment:
    ```bash
    python -m venv venv
    source venv/bin/activate
    ```
4. Install the required dependencies:
    ```bash
    pip install -r requirements.txt
    ```
5. Run the script:
    ```bash
    python ble-cbor-to-osc.py
    ```

### 3. Configure OSC Address and Port (Optional)

By default, the script sends OSC messages to `127.0.0.1:9001`. You can modify these settings using the following arguments:

- **IP Address**: `-a` or `--osc-address` (default: [`127.0.0.1`])
- **Port**: `-o` or `--osc-port` (default: [`9001`]) 

Example:
```bash
python ble-cbor-to-osc.py -a 192.168.1.100 -o 8000
```

### 4. Verify Data Reception

Once the firmware is running on the ESP32 and the BLE-CBOR-to-OSC script is active, you should start receiving OSC messages containing the dummy sensor data `sensor1` and`sensor2` at the specified IP address and port.

## Notes

- The BLE advertising payload is limited to 27 bytes. Ensure the CBOR-encoded data does not exceed this limit.
- The template uses the Puara module manager for initialization and configuration. Refer to the [Puara documentation](https://github.com/Puara) for more details.

## License

This project is licensed under the MIT License. See the LICENSE file for details.

## Acknowledgments

- Developed by the [Société des Arts Technologiques (SAT)](https://sat.qc.ca/).
- Contributions from the Input Devices and Music Interaction Laboratory (IDMIL), McGill University.
- Special thanks to Edu Meneses for the original implementation.