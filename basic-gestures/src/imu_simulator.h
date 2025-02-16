#pragma once

#include "Arduino.h"

/*
 * Simulate moving an IMU
 * Clockwise movements for each axis at rotationTimeMs speed
 */
class IMUSimulator {
private:
  float accelX, accelY, accelZ; // Acceleration (m/s^2)
  float gyroX, gyroY, gyroZ;    // Angular velocity (rad/s)
  float magX, magY, magZ;       // Magnetic field (unitless, normalized) or (microteslas, adjust as needed)
  float rotationTime;           // Rotation time (milliseconds)
  unsigned long startTimeX, startTimeY, startTimeZ;
  float angleX, angleY;

public:
  IMUSimulator(float rotationTimeMs = 2000.0) : rotationTime(rotationTimeMs) {}

  void begin() {
    randomSeed(analogRead(0)); // Seed the random number generator
    startTimeX = millis();
    startTimeY = millis();
    startTimeZ = millis();
  }

  void update() {
    simulateRotationX();
    simulateRotationY();
    simulateRotationZ();
    generateRandomMagnetometer();
  }

  float getAccelX() { return accelX; }
  float getAccelY() { return accelY; }
  float getAccelZ() { return accelZ; }
  float getGyroX() { return gyroX; }
  float getGyroY() { return gyroY; }
  float getGyroZ() { return gyroZ; }
  float getMagX() { return magX; }
  float getMagY() { return magY; }
  float getMagZ() { return magZ; }

private:
  void simulateRotationX() {
    float elapsedTimeX = millis() - startTimeX;
    angleX = (elapsedTimeX / rotationTime) * 2 * PI;
    if (elapsedTimeX >= rotationTime) {
      startTimeX = millis();
    }
    gyroX = 2 * PI / rotationTime;
    accelX = 9.81 * sin(angleX);
    gyroY = 0;
    accelY = 0;
    gyroZ = 0;
    accelZ = 9.81 * cos(angleX);
  }

  void simulateRotationY() {
    float elapsedTimeY = millis() - startTimeY;
    angleY = (elapsedTimeY / rotationTime) * 2 * PI;
    if (elapsedTimeY >= rotationTime) {
      startTimeY = millis();
    }
    gyroY = 2 * PI / rotationTime;
    float tempAccelY = 9.81 * sin(angleY);
    float tempAccelZ = 9.81 * cos(angleY);

    // Combine X and Y rotations in accelerations
    accelY = tempAccelY * cos(angleX);
    accelZ = tempAccelZ * cos(angleX);
    accelY += 9.81 * sin(angleX) * sin(angleY);
    accelZ -= 9.81 * sin(angleX) * sin(angleY);
  }

  void simulateRotationZ() {
    float elapsedTimeZ = millis() - startTimeZ;
    float angleZ = (elapsedTimeZ / rotationTime) * 2 * PI;
    if (elapsedTimeZ >= rotationTime) {
      startTimeZ = millis();
    }
    gyroZ = 2 * PI / rotationTime;
  }

  void generateRandomMagnetometer() {
    magX = random(-100, 100);
    magY = random(-100, 100);
    magZ = random(-100, 100);

    float magMagnitude = sqrt(magX * magX + magY * magY + magZ * magZ);
    magX /= magMagnitude;
    magY /= magMagnitude;
    magZ /= magMagnitude;
  }
};