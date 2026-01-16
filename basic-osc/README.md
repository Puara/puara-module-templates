# Basic OSC

This is a template that sends and receives OSC messages.

## Known issue.

With arduino-esp32 2.X (the version currently available in mainline platformio), in conjunction to the xtensa toolchain 12.X, the receive function crashes.

Please use pioarduino to be able to use arduino-esp32 3.X.
