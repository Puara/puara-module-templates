# Puara Module : Basic Example

## Overview

A minimal example demonstrating core Puara Module functionality. This template:
- Initializes the Puara module manager
- Reads custom settings from the JSON configuration files
- Outputs dummy sensor data to the serial monitor
- Demonstrates how to access custom configuration variables using `getVarText()` and `getVarNumber()`

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