# Puara Module Template (Module Manager)

[![Build](https://github.com/Puara/puara-module-templates/actions/workflows/build.yml/badge.svg)](https://github.com/Puara/puara-module-templates/actions/workflows/build.yml)

This repository contains several templates to be used as a base to create devices that can be controlled over the network the Puara Framework.

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
