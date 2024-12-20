#!/bin/bash

# Variables
TEMPLATE=$1
ENVIRONMENT_NAME=$2
BOARD=$3
EXTRA_FLAGS=$4

# Debug output
echo "Template: $TEMPLATE"
echo "Environment: $ENVIRONMENT_NAME"
echo "Board: $BOARD"
echo "Extra Flags: $EXTRA_FLAGS"

# Ensure output path is correct
OUTPUT_FILE="$(pwd)/platformioTemp.ini"
echo "Generating platformioTemp.ini at: ${OUTPUT_FILE}"

# Generate platformioTemp.ini at the project root
cat <<EOL > "${OUTPUT_FILE}"
[platformio]
[env:${ENVIRONMENT_NAME}]
platform = espressif32
board = ${BOARD}
framework = arduino
board_build.partitions = min_spiffs_no_OTA.csv
monitor_speed = 115200
monitor_echo = yes
monitor_filters = default,esp32_exception_decoder
build_flags = -std=gnu++2a ${EXTRA_FLAGS}
build_unflags = -std=gnu++11 -std=gnu++14 -std=gnu++17
lib_deps =
EOL

# Add dependencies based on the template
case "${TEMPLATE}" in
  basic)
    echo "    https://github.com/Puara/puara-module.git" >> "${OUTPUT_FILE}"
    ;;
  basic-osc)
    echo "    https://github.com/Puara/puara-module.git" >> "${OUTPUT_FILE}"
    echo "    https://github.com/cnmat/OSC#3.5.8" >> "${OUTPUT_FILE}"
    ;;
  ble-advertising)
    echo "    https://github.com/Puara/puara-gestures.git" >> "${OUTPUT_FILE}"
    echo "    https://github.com/Puara/puara-module.git" >> "${OUTPUT_FILE}"
    echo "    arduino-libraries/ArduinoBLE" >> "${OUTPUT_FILE}"
    echo "    johboh/nlohmann-json@^3.11.3" >> "${OUTPUT_FILE}"
    ;;
  libmapper-osc)
    echo "    https://github.com/Puara/puara-gestures.git" >> "${OUTPUT_FILE}"
    echo "    https://github.com/mathiasbredholt/libmapper-arduino.git#v0.3" >> "${OUTPUT_FILE}"
    echo "    https://github.com/Puara/puara-module.git" >> "${OUTPUT_FILE}"
    ;;
  *)
    echo "Unknown template: ${TEMPLATE}"
    exit 1
    ;;
esac
