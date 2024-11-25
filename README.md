# Puara Module template (module manager)

This repository contains several templates to be used as a base to create devices that can be controlled over the network the Puara Framework.

## How to use

- Install [PlatformIO](https://platformio.org/) ([VSC](https://code.visualstudio.com/) recommended as the code editor)
- [Clone this repository](https://docs.github.com/en/repositories/creating-and-managing-repositories/cloning-a-repository) locally
- Open the template folder that interests you as a PlatformIO project using the **Add Existing** button at the Project manager tab.
- You are ready to edit the template.
- Once finished, remember to change the board before uploading your firmware to the ESP32 board. This template is configured to the [TinyPico](https://www.tinypico.com/), but the firmware is compatible with any ESP32-based board.

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

[http://www.sat.qc.ca](http://www.sat.qc.ca/)

[http://www.idmil.org](http://www.idmil.org/)

[https://www.edumeneses.com](https://www.edumeneses.com)

## Licensing

The code in this project is licensed under the MIT license, unless otherwise specified within the file.
