#!/bin/bash -eu

# Variables
TEMPLATE=$1
BOARD=$2
EXTRA_FLAGS=$3

if [[ -z "$PUARA_MODULE_PATH" ]]; then
# If no hash given, points to main head
  PUARA_MODULE_COMMIT_HASH=""
  PUARA_MODULE_PATH="https://github.com/Puara/puara-module.git$PUARA_MODULE_COMMIT_HASH"
fi

if [[ -z "$PUARA_GESTURES_PATH" ]]; then
  PUARA_GESTURES_PATH="https://github.com/Puara/puara-gestures.git"
fi

# We'll put the temporary platformio.ini file in the current template folder
OUTPUT_FILE="${TEMPLATE}/platformioTemp.ini"

# Determine if -DPUARA_SPIFFS should be added based on the template
SPIFFS_FLAG=""
case "${TEMPLATE}" in
  basic-spiffs|basic-gestures-spiffs|ble-advertising-spiffs|button-osc-spiffs|libmapper-osc-spiffs|basic-osc-spiffs)
    SPIFFS_FLAG="-DPUARA_SPIFFS"
    ;;
esac


# Write the file
cat <<EOL > "${OUTPUT_FILE}"
[platformio]
[env:template]
platform = espressif32
board = ${BOARD}
framework = arduino
board_build.partitions = min_spiffs_no_OTA.csv
monitor_speed = 115200
monitor_echo = yes
monitor_filters = default,esp32_exception_decoder
builg_type = release
build_flags = -std=gnu++2a ${EXTRA_FLAGS} ${SPIFFS_FLAG}
build_unflags = -std=gnu++11 -std=gnu++14 -std=gnu++17
lib_deps =
EOL


# Add dependencies based on the template
case "${TEMPLATE}" in
  basic-littlefs)
    echo "    $PUARA_MODULE_PATH" >> "${OUTPUT_FILE}"
    echo "board_build.filesystem = littlefs" >> "${OUTPUT_FILE}"
    ;;
  basic-spiffs)
    echo "    $PUARA_MODULE_PATH" >> "${OUTPUT_FILE}"
    echo "board_build.filesystem = spiffs" >> "${OUTPUT_FILE}"
    ;;
  basic-gestures-littlefs)
    echo "    $PUARA_MODULE_PATH" >> "${OUTPUT_FILE}"
    echo "    $PUARA_GESTURES_PATH" >> "${OUTPUT_FILE}"
    echo "board_build.filesystem = littlefs" >> "${OUTPUT_FILE}"
    ;;
  basic-gestures-spiffs)
    echo "    $PUARA_MODULE_PATH" >> "${OUTPUT_FILE}"
    echo "    $PUARA_GESTURES_PATH" >> "${OUTPUT_FILE}"
    echo "board_build.filesystem = spiffs" >> "${OUTPUT_FILE}"
    ;;
  basic-osc-littlefs)
    echo "    $PUARA_MODULE_PATH" >> "${OUTPUT_FILE}"
    echo "    https://github.com/cnmat/OSC#3.5.8" >> "${OUTPUT_FILE}"
    ;;
  basic-osc-spiffs)
    echo "    $PUARA_MODULE_PATH" >> "${OUTPUT_FILE}"
    echo "    https://github.com/cnmat/OSC#3.5.8" >> "${OUTPUT_FILE}"
    ;;
  ble-advertising-littlefs)
    echo "    $PUARA_GESTURES_PATH" >> "${OUTPUT_FILE}"
    echo "    $PUARA_MODULE_PATH" >> "${OUTPUT_FILE}"
    echo "    arduino-libraries/ArduinoBLE" >> "${OUTPUT_FILE}"
    echo "    johboh/nlohmann-json@3.11.3" >> "${OUTPUT_FILE}"
    echo "board_build.filesystem = littlefs" >> "${OUTPUT_FILE}"
    ;;
  ble-advertising-spiffs)
    echo "    $PUARA_GESTURES_PATH" >> "${OUTPUT_FILE}"
    echo "    $PUARA_MODULE_PATH" >> "${OUTPUT_FILE}"
    echo "    arduino-libraries/ArduinoBLE" >> "${OUTPUT_FILE}"
    echo "    johboh/nlohmann-json@3.11.3" >> "${OUTPUT_FILE}"
    echo "board_build.filesystem = spiffs" >> "${OUTPUT_FILE}"
    ;;
  button-osc-littlefs)
    echo "    $PUARA_MODULE_PATH" >> "${OUTPUT_FILE}"
    echo "    https://github.com/cnmat/OSC#3.5.8" >> "${OUTPUT_FILE}"
    echo "    $PUARA_GESTURES_PATH" >> "${OUTPUT_FILE}"
    ;;
  button-osc-spiffs)
    echo "    $PUARA_MODULE_PATH" >> "${OUTPUT_FILE}"
    echo "    https://github.com/cnmat/OSC#3.5.8" >> "${OUTPUT_FILE}"
    echo "    $PUARA_GESTURES_PATH" >> "${OUTPUT_FILE}"
    ;;
  libmapper-osc-littlefs)
    echo "    $PUARA_GESTURES_PATH" >> "${OUTPUT_FILE}"
    echo "    https://github.com/mathiasbredholt/libmapper-arduino.git#v0.3" >> "${OUTPUT_FILE}"
    echo "    $PUARA_MODULE_PATH" >> "${OUTPUT_FILE}"
    ;;
  libmapper-osc-spiffs)
    echo "    $PUARA_GESTURES_PATH" >> "${OUTPUT_FILE}"
    echo "    https://github.com/mathiasbredholt/libmapper-arduino.git#v0.3" >> "${OUTPUT_FILE}"
    echo "    $PUARA_MODULE_PATH" >> "${OUTPUT_FILE}"
    ;;
  *)
    echo "Unknown template: ${TEMPLATE}"
    exit 1
    ;;
esac

