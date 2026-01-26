# OSC-Receive

This template demonstrates how to use Puara modules to receives OSC messages.

Please refer to CNMAT's OSC repository on Github for more details on OSC. 

## Known issue.

With arduino-esp32 2.X (the version currently available in mainline platformio), in conjunction to the xtensa toolchain 12.X, the receive function crashes.

Please use pioarduino to be able to use arduino-esp32 3.X.

This can be done by setting the platform and framework as follows in platformio.ini:

[env:your-template]
platform = https://github.com/pioarduino/platform-espressif32/releases/download/stable/platform-espressif32.zip
framework = arduino
