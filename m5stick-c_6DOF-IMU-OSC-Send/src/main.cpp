//****************************************************************************//
// Société des Arts Technologiques (SAT)                                      //
// Input Devices and Music Interaction Laboratory (IDMIL), McGill University  //
// Puara Module : OSC-Send template                                           //
//                                                                            //
// This template demonstrates how to use the M5StickC, set up all the IMU     //
// data processing and send the data as a bundled OSC message to a defined    //
// IP : port. Please refer to CNMAT's OSC repository on Github for more       //
// details on OSC.                                                            //
//                                                                            //
// Puara Module Manager facilitates embedded sytem development by providing   //
// a set of pre-defined modules that manage filesystem, webserver and network //
// connections so the user can focus on easier prototyping of the system.     //
//                                                                            //
// Puara Module will configure the filesystem with information found in       //
// config.json and settings.json files. The process will then try to connect  //
// to the WiFi Network (SSID) defined in config.json (STA mode).              //
// If the user wants the board to connect to a specific WiFi network, they    //
// may modify the "wifiSSID" and "wifiPSK" values in config.json file with    //
// the name of the desired WiFi network and it's password respectively.       //
//                                                                            //
// If the "persistentAP" flag in config.json is left to 1 (True), the process //
// will create a WiFi Access Point (AP mode) even if it successfully connects //
// to a WiFi network. This allows users to connect to the board's own network //
// and access the webserver to change settings even when the board is         //
// connected to another WiFi network.                                         //
//                                                                            //
// The process will then start a webserver where users may modify             //
// configurations and settings of their board/program without having to       //
// rebuild/reflash their program. Users may configure IP address and port in  //
// settings.json.                                                             // 
// Access these webpages by connecting to the board's WiFi network, open a    //
// browser and type the network name followed by ".local" in the address bar. //
// For example, if the board's network name is the default "Puara_001", type  //
// "puara_001.local" in the browser's address bar.                            //
// If in STA mode, type the board's IP address in the browser's address bar.  //
//                                                                            //
// **REMEMBER : USER MUST BUILD+UPLOAD PROGRAM AND FILESYSTEM IN TWO STEPS**  //
//****************************************************************************//

#include "Arduino.h"
#include <iostream>
#include <M5StickC.h>
#include "OSCBundle.h"
#include "OSCMessage.h"
#include "OSCTiming.h"
#include "puara.h"
#include <WiFiUdp.h>

Puara puara;
WiFiUDP Udp;

std::string oscIP{};
int oscPort{};
std::string osc_prefix{};
std::string ip_addr;

float gyroX, gyroY, gyroZ;
float accX, accY, accZ;
float pitch, roll, yaw;

uint8_t batteryPercentage();
void drawImuTriangle();
void initM5_IMU();
void initOSCvariables();
void sendOSCBundle();
void updateIMUData();
void onSettingsChanged();


void setup() {

#ifdef Arduino_h
  Serial.begin(115200);
#endif

  // Initialize M5StickC, the IMU sensor and display boot message.
  initM5_IMU();

  // Initialize Puara module and start the process. 
  // This will set up the filesystem, connect to WiFi and start the webserver.
  puara.start();

  // Start the UDP connection for OSC transmission. 
  // The port number is defined in settings.json file and can be changed in the web interface.
  initOSCvariables();

}

void loop() {

// Update IMU data by reading from the sensor. 
// The data is stored in global variables for later use in drawing and OSC transmission.
  updateIMUData();

// Draw a triangle on the LCD screen to visualize the orientation of the board. 
// The triangle's position, size and color changes according to the IMU data and battery level.
  drawImuTriangle();

// Bundle and send OSC messages
  sendOSCBundle();

}

// This function calculates the battery percentage based on the voltage reading
// from the M5StickC's battery.
uint8_t batteryPercentage() {
  float voltage = M5.Axp.GetBatVoltage();
  const float minV = 3.20f;
  const float maxV = 4.20f;
  float pct = (voltage - minV) / (maxV - minV) * 100.0f;
  if (pct < 0.0f) {
    pct = 0.0f;
  } else if (pct > 100.0f) {
    pct = 100.0f;
  }
  return (uint8_t)pct;
}

// This function draws a triangle on the LCD screen to visualize the orientation of the board.
// The triangle's position is determined by the roll and pitch values, its size is determined by
// the acceleration magnitude, and its color changes according to the battery level.
void drawImuTriangle() {

  float ax = accX * M5.IMU.aRes;
  float ay = accY * M5.IMU.aRes;
  float az = accZ * M5.IMU.aRes;
  float accelG = sqrtf(ax * ax + ay * ay + az * az);

  int16_t centerX = 40 + (int16_t)constrain(roll * 0.5f, -24, 24);
  int16_t centerY = 80 + (int16_t)constrain(pitch * 0.5f, -24, 24);
  int16_t size = 18 + (int16_t)constrain((accelG - 1.0f) * 8.0f, -10, 20);

  float angle = yaw * DEG_TO_RAD;
  float s = sinf(angle);
  float c = cosf(angle);

  int16_t vx0 = 0;
  int16_t vy0 = -size;
  int16_t vx1 = -size;
  int16_t vy1 = size;
  int16_t vx2 = size;
  int16_t vy2 = size;

  int16_t x0 = centerX + (int16_t)(vx0 * c - vy0 * s);
  int16_t y0 = centerY + (int16_t)(vx0 * s + vy0 * c);
  int16_t x1 = centerX + (int16_t)(vx1 * c - vy1 * s);
  int16_t y1 = centerY + (int16_t)(vx1 * s + vy1 * c);
  int16_t x2 = centerX + (int16_t)(vx2 * c - vy2 * s);
  int16_t y2 = centerY + (int16_t)(vx2 * s + vy2 * c);

  uint8_t battPct = batteryPercentage();
  float battVoltage = M5.Axp.GetBatVoltage();
  uint16_t triColor = battPct > 50 ? GREEN : (battPct > 25 ? YELLOW : RED);

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.fillTriangle(x0, y0, x1, y1, x2, y2, triColor);
  M5.Lcd.drawTriangle(x0, y0, x1, y1, x2, y2, WHITE);

  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("Bat: %u%%\nV: %.2fV\nIP: %s", battPct, battVoltage, ip_addr.c_str());
}

// This function initializes the M5StickC, sets up the IMU sensor and displays 
// a boot message on the LCD screen.
void initM5_IMU(){

  M5.begin();
  M5.IMU.Init();
  M5.IMU.getAres();
  M5.IMU.getGres();

  M5.Lcd.fillScreen(CYAN);
  M5.Lcd.setCursor(5, 10);
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.printf("Booting up...\nConnecting to WiFi...");
}

// This function initializes the UDP connection for OSC transmission and sets 
// up the settings changed handler for the web interface.
void initOSCvariables(){

  Udp.begin(puara.getVarNumber("localPORT"));
  puara.set_settings_changed_handler(onSettingsChanged);
  oscIP = puara.getVarText("oscIP");
  oscPort = puara.getVarNumber("oscPORT");
  osc_prefix = "/" + puara.dmi_name();
  ip_addr = puara.staIP();

}

// This function bundles the IMU data into an OSC message and sends it to the 
// defined IP address and port using UDP.
void sendOSCBundle(){
  if (!oscIP.empty() && oscIP != "0.0.0.0") {
    OSCBundle bundle;
    osctime_t timetag;

    bundle.add((osc_prefix + "/IMU/accl/x").c_str()).add(((float)accX) * M5.IMU.aRes);
    bundle.add((osc_prefix + "/IMU/accl/y").c_str()).add(((float)accY) * M5.IMU.aRes);
    bundle.add((osc_prefix + "/IMU/accl/z").c_str()).add(((float)accZ) * M5.IMU.aRes);

    bundle.add((osc_prefix + "/IMU/gyro/x").c_str()).add(((float)gyroX) * M5.IMU.gRes);
    bundle.add((osc_prefix + "/IMU/gyro/y").c_str()).add(((float)gyroY) * M5.IMU.gRes);
    bundle.add((osc_prefix + "/IMU/gyro/z").c_str()).add(((float)gyroZ) * M5.IMU.gRes);

    bundle.add((osc_prefix + "/IMU/YPR/Roll").c_str()).add(roll);
    bundle.add((osc_prefix + "/IMU/YPR/Pitch").c_str()).add(pitch);
    bundle.add((osc_prefix + "/IMU/YPR/Yaw").c_str()).add(yaw);

    Udp.beginPacket(oscIP.c_str(), oscPort);
    bundle.setTimetag(oscTime());
    bundle.send(Udp);
    Udp.endPacket();
    bundle.empty();
  }
}

// This function updates the global variables for gyro, accel and YPR data by reading
// from the M5StickC's IMU sensor. The gyro and accel data are multiplied by their 
// respective resolution values to convert them to physical units. 
// The YPR data is already in degrees.
void updateIMUData(){

  M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);
  M5.IMU.getAccelData(&accX, &accY, &accZ);
  M5.IMU.getAhrsData(&pitch, &roll, &yaw);

}

// The onSettingsChanged() function is called when settings are saved in the web
// interface (click on "Save" button). This allows user to change variables on
// their board without needing to go through the code build/flash process again.
// Here we use it to change the UDP port for OSC reception and transmission.
void onSettingsChanged() {
  Udp.begin(puara.getVarNumber("localPORT"));
  oscIP = puara.getVarText("oscIP");
  oscPort = puara.getVarNumber("oscPORT");
}


 // The Arduino header defines app_main and conflicts with having an app_main
 // function in code. This ifndef makes the code valid in case we remove the
 // Arduino header in the future.
 #ifndef Arduino_h
extern "C" {
void app_main(void);
}

void app_main() {
  setup();
  while (1) {
    loop();
  }
}
#endif
