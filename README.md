# Puara Module Template (Module Manager)

[![Build](https://github.com/Puara/puara-module-templates/actions/workflows/build.yml/badge.svg)](https://github.com/Puara/puara-module-templates/actions/workflows/build.yml)

This repository contains several templates to be used as a base to create devices that can be controlled over the network the Puara Framework.

## How to Use

1. **Install PlatformIO**: [PlatformIO](https://platformio.org/) (Visual Studio Code ([VSC](https://code.visualstudio.com/)) is recommended as the code editor).

2. **Clone this repository**: (https://docs.github.com/en/repositories/creating-and-managing-repositories/cloning-a-repository) locally.

3. **Open the project**: Open the template folder that interests you as a PlatformIO project using the **Add Existing** button in the Project Manager tab.

4. **Configure the board**: Ensure the `board` variable in the `platformio.ini` file matches your board's name.

5. **Edit the template**: You are now ready to edit the template according to your board/needs.

6. **Upload the firmware**: Once finished, remember to change the board configuration if necessary before uploading your firmware to the ESP32 board. This template is configured for the [m5stick-c](https://shop.m5stack.com/products/stick-c), but the firmware is compatible with any ESP32-based board.

## Available Templates

### basic

Use this as a bare-minimum template to just make puara-module available in you microcontroller.

### basic-osc

Use this as a base if you want to send and receive osc messages.

### ble-advertising

Use this as a base if you want to expose sensor data as BLE advertisements.

### libmapper-osc

Use this as a base if you want to use libmapper or raw OSC messages.

## More Info on the research related with [Puara](https://github.com/Puara)

- [SAT](http://www.sat.qc.ca)
- [IDMIL](http://www.idmil.org)
- [Edu Meneses](https://www.edumeneses.com)

## Licensing

The code in this project is licensed under the MIT license, unless otherwise specified within the file.
