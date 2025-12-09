# Puara Module Basic-OSC Template

This template facilitates the making of OSC ready microcontrollers where the integrated Puara Module manages all network related considerations. After selecting your board, building and uploading the firmware, all you have to do is connect to it's network to recieve it's OSC messages. The main.cpp is only a template and more complex control systems may be developped from this base. 
The current cycle sends OSC messages once every second. You may change the send speed by modifying the following line :     `vTaskDelay(1000 / portTICK_PERIOD_MS);`


## Features

- **ESP32 Support**: Uses the `espressif32` platform version `6.7.0`.
- **Arduino Framework**: Leverages the Arduino framework for development.
- **Filesystem Options**: Currently supports only SPIFFS
- **OSC Library**: Includes the CNMAT OSC library for Open Sound Control functionality.
- **Customizable Board Configuration**: Easily switch between different ESP32 boards.

## Requirements

- [PlatformIO](https://platformio.org/) installed in your development environment.
- A supported ESP32 board (e.g., Adafruit Feather ESP32-S3 TFT, TinyPICO, ESP32-C3 DevKitC-02, Seeed Xiao ESP32-C3, ... ).

## Getting Started

1. **Clone the Repository**:
   ```bash
   git clone <repository-url>
   cd <repository-folder>

   ```markdown
# Puara Module Basic-OSC Template

This repository contains a PlatformIO project template for developing firmware for Puara modules using the ESP32 microcontroller and the Arduino framework. It includes support for basic OSC (Open Sound Control) functionality and is designed to work with older Puara modules.

## Features

- **ESP32 Support**: Uses the `espressif32` platform version `6.7.0`.
- **Arduino Framework**: Leverages the Arduino framework for development.
- **Filesystem Options**: Supports both SPIFFS and LittleFS (default is commented out for compatibility with older modules).
- **OSC Library**: Includes the CNMAT OSC library for Open Sound Control functionality.
- **Customizable Board Configuration**: Easily switch between different ESP32 boards.

## Requirements

- [PlatformIO](https://platformio.org/) installed in your development environment.
- A supported ESP32 board (e.g., Adafruit Feather ESP32-S3 TFT, TinyPICO, ESP32-C3 DevKitC-02, Seeed Xiao ESP32-C3).

## Getting Started

1. **Clone the Repository**:
   ```bash
   git clone <repository-url>
   cd <repository-folder>
   ```

2. **Install Dependencies**:
   PlatformIO will automatically install the required libraries and dependencies when you build the project.

3. **Select Your Board**:
   Edit the `platformio.ini` file and update the `board` field under `[env:template]` to match your ESP32 board. For example:
   ```ini
   board = tinypico
   ```

4. **Build and Upload**:
   Use the following commands to build and upload the firmware:
   ```bash
   pio run
   pio run --target upload
   ```
5. **Build filesystem and upload filesystem**
    ```bash
    add steps here
    ```

6. **Monitor Serial Output**:
   Open the serial monitor to view debug output:
   ```bash
   pio device monitor
   ```

### Libraries
The following libraries are included:
- [Puara Module Library](https://github.com/Puara/puara-module.git) (specific commit: `6297c8e5b00302843ca7539bc246226ff03e6ae0`)
- [CNMAT OSC Library](https://github.com/cnmat/OSC) (version `3.5.8`)


## License

This project is licensed under the terms of the [MIT License](LICENSE).

## Contributing

Contributions are welcome! Please submit a pull request or open an issue for any improvements or bug fixes.
