#!/bin/bash -eu

# Variables
TEMPLATE=$1
BOARD=$2
EXTRA_FLAGS=$3

# We'll put the temporary platformio.ini file in the current template folder
OUTPUT_FILE="${TEMPLATE}/platformioTemp.ini"

# Determine if -DPUARA_SPIFFS should be added based on the template
SPIFFS_FLAG=""
case "${TEMPLATE}" in
  basic-spiffs|basic-gestures-spiffs|ble-advertising-spiffs)
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
build_flags = -std=gnu++2a ${EXTRA_FLAGS} ${SPIFFS_FLAG}
build_unflags = -std=gnu++11 -std=gnu++14 -std=gnu++17
lib_deps =
EOL


# Add dependencies based on the template
case "${TEMPLATE}" in
  basic-littlefs)
    echo "    https://github.com/Puara/puara-module.git#d1b9505ba398312a9863e904d40f0a6255342582" >> "${OUTPUT_FILE}"
    echo "board_build.filesystem = littlefs" >> "${OUTPUT_FILE}"
    ;;
  basic-spiffs)
    echo "    https://github.com/Puara/puara-module.git#d1b9505ba398312a9863e904d40f0a6255342582" >> "${OUTPUT_FILE}"
    echo "board_build.filesystem = spiffs" >> "${OUTPUT_FILE}"
    ;;
  basic-gestures-littlefs)
    echo "    https://github.com/Puara/puara-module.git#d1b9505ba398312a9863e904d40f0a6255342582" >> "${OUTPUT_FILE}"
    echo "    https://github.com/Puara/puara-gestures.git" >> "${OUTPUT_FILE}"    
    echo "board_build.filesystem = littlefs" >> "${OUTPUT_FILE}"
    ;;
  basic-gestures-spiffs)
    echo "    https://github.com/Puara/puara-module.git#d1b9505ba398312a9863e904d40f0a6255342582" >> "${OUTPUT_FILE}"
    echo "    https://github.com/Puara/puara-gestures.git" >> "${OUTPUT_FILE}"        
    echo "board_build.filesystem = spiffs" >> "${OUTPUT_FILE}"
    ;;
  basic-osc-nofs)
    echo "    https://github.com/Puara/puara-module.git#6297c8e5b00302843ca7539bc246226ff03e6ae0" >> "${OUTPUT_FILE}"
    echo "    https://github.com/cnmat/OSC#3.5.8" >> "${OUTPUT_FILE}"
    ;;
  ble-advertising-littlefs)
    echo "    https://github.com/Puara/puara-gestures.git" >> "${OUTPUT_FILE}"
    echo "    https://github.com/Puara/puara-module.git#d1b9505ba398312a9863e904d40f0a6255342582" >> "${OUTPUT_FILE}"
    echo "    arduino-libraries/ArduinoBLE" >> "${OUTPUT_FILE}"
    echo "    johboh/nlohmann-json@3.11.3" >> "${OUTPUT_FILE}"
    echo "board_build.filesystem = littlefs" >> "${OUTPUT_FILE}"
    ;;
  ble-advertising-spiffs)
    echo "    https://github.com/Puara/puara-gestures.git" >> "${OUTPUT_FILE}"
    echo "    https://github.com/Puara/puara-module.git#d1b9505ba398312a9863e904d40f0a6255342582" >> "${OUTPUT_FILE}"
    echo "    arduino-libraries/ArduinoBLE" >> "${OUTPUT_FILE}"
    echo "    johboh/nlohmann-json@3.11.3" >> "${OUTPUT_FILE}"
    echo "board_build.filesystem = spiffs" >> "${OUTPUT_FILE}"
    ;;
  button-osc-nofs)
    echo "    https://github.com/Puara/puara-module.git#6297c8e5b00302843ca7539bc246226ff03e6ae0" >> "${OUTPUT_FILE}"
    echo "    https://github.com/cnmat/OSC#3.5.8" >> "${OUTPUT_FILE}"
    echo "    https://github.com/Puara/puara-gestures.git" >> "${OUTPUT_FILE}"    
    ;;
  libmapper-osc-nofs)
    echo "    https://github.com/Puara/puara-gestures.git" >> "${OUTPUT_FILE}"
    echo "    https://github.com/mathiasbredholt/libmapper-arduino.git#v0.3" >> "${OUTPUT_FILE}"
    echo "    https://github.com/Puara/puara-module.git#6297c8e5b00302843ca7539bc246226ff03e6ae0" >> "${OUTPUT_FILE}"
    ;;
  *)
    echo "Unknown template: ${TEMPLATE}"
    exit 1
    ;;
esac

