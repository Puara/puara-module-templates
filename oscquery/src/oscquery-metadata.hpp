#pragma once
#include <string>
inline std::string oscquery_namespace =
R"_({
  "FULL_PATH": "/",
  "CONTENTS": {
    "valve.1": {
      "FULL_PATH": "/valve.1",
      "CONTENTS": {
        "duty": {
          "FULL_PATH": "/valve.1/duty",
          "TYPE": "i",
          "RANGE": [{"MIN": 0, "MAX": 255 }],
          "ACCESS": 3,
          "VALUE": 0
        }
      }
    },
    "valve.2": {
      "FULL_PATH": "/valve.2",
      "CONTENTS": {
        "state": {
          "FULL_PATH": "/valve.2/state",
          "TYPE": "T",
          "ACCESS": 3,
          "VALUE": 0
        }
      }
    },
    "valve.3": {
      "FULL_PATH": "/valve.3",
      "CONTENTS": {
        "state": {
          "FULL_PATH": "/valve.3/state",
          "TYPE": "T",
          "RANGE": [{"MIN": 0, "MAX": 255 }],
          "ACCESS": 3,
          "VALUE": 0
        }
      }
    },
    "valve.4": {
      "FULL_PATH": "/valve.4",
      "CONTENTS": {
        "duty": {
          "FULL_PATH": "/valve.4/duty",
          "TYPE": "i",
          "RANGE": [{"MIN": 0, "MAX": 255 }],
          "ACCESS": 3,
          "VALUE": 0
        }
      }
    },
    "pixel.5": {
      "FULL_PATH": "/pixel.5",
      "CONTENTS": {
        "duty": {
          "FULL_PATH": "/pixel.5/rgb",
          "TYPE": "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii",
          "ACCESS": 3,
          "VALUE": 0
        }
      }
    },
    "valve.6": {
      "FULL_PATH": "/valve.6",
      "CONTENTS": {
        "duty": {
          "FULL_PATH": "/valve.6/duty",
          "TYPE": "i",
          "RANGE": [{"MIN": 0, "MAX": 255 }],
          "ACCESS": 3,
          "VALUE": 0
        }
      }
    },
    "valve.7": {
      "FULL_PATH": "/valve.7",
      "CONTENTS": {
        "duty": {
          "FULL_PATH": "/valve.7/duty",
          "TYPE": "i",
          "RANGE": [{"MIN": 0, "MAX": 255 }],
          "ACCESS": 3,
          "VALUE": 0
        }
      }
    },
    "valve.8": {
      "FULL_PATH": "/valve.8",
      "CONTENTS": {
        "duty": {
          "FULL_PATH": "/valve.8/duty",
          "TYPE": "i",
          "RANGE": [{"MIN": 0, "MAX": 255 }],
          "ACCESS": 3,
          "VALUE": 0
        }
      }
    }
  }
})_";

inline std::string oscquery_host_info =
  R"_({
  "NAME": "esp32-oscquery-1",
  "OSC_PORT": 2333,
  "OSC_TRANSPORT": "UDP",
  "EXTENSIONS": {
    "TYPE": true,
    "ACCESS": true,
    "VALUE": true,
    "RANGE": true,
    "TAGS": false,
    "CLIPMODE": true,
    "UNIT": false,
    "CRITICAL": false,
    "DESCRIPTION": true,
    "HTML": false,
    "OSC_STREAMING": false,
    "LISTEN": false,
    "ECHO": false,
    "PATH_CHANGED": false,
    "PATH_RENAMED": false,
    "PATH_ADDED": false,
    "PATH_REMOVED": false
  }
})_";
