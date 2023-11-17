#include "Adafruit_VL53L0X.h"
#include <MPU6050_tockn.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define L2 0.05 // length of arm joint in meters
#define f 0.01  // focal length of camera in meters
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
int sda_pin=21, scl_pin=22;


void setup() {
  Serial.begin(9600);

  Wire.begin(sda_pin, scl_pin);
  Serial.println("Adafruit VL53L0X test");
  // Wire.begin(sda_pin, scl_pin);

  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
  while(1);
  }
  // Adafruit_VL53L0X lox = Adafruit_VL53L0X();
  // Initialize sensor (if needed)
  // sensor.init();
}

void calculate_laser_coordinates(float arm_angle, float &relative_laser_x, float &relative_laser_y) {
  float arm_angle_rad = arm_angle * (PI / 180.0); // Convert to radians
  relative_laser_x = L2 * cos(arm_angle_rad);
  relative_laser_y = L2 * sin(arm_angle_rad);
}

// Function to find scene coordinates from image coordinates, Z distance, and focal length f
void find_scene_coordinates(float image_x, float image_y, float Z, float f, float *X, float *Y) {
    *X = (image_x * Z) / f;
    *Y = (image_y * Z) / f;
}

// Function to calculate inverse kinematics
void calculate_inverse_kinematics(float player_x, float player_y, float shoulder_x, float shoulder_y, float laser_x, float laser_y, float Z, float *base_angle_deg, float *laser_angle_deg) {
    float absolute_laser_x = shoulder_x + laser_x;
    float absolute_laser_y = shoulder_y + laser_y;

    // Calculate the angles in radians
    float angle_base = atan2(player_x - absolute_laser_x, Z);
    float angle_laser = atan2(player_y - absolute_laser_y, Z);

    // Convert angles to degrees
    *base_angle_deg = angle_base * (180.0 / M_PI);
    *laser_angle_deg = angle_laser * (180.0 / M_PI);
}

// Function to rotate base
float rotate_base(float current_angle, float target_angle) {
    float rotation_angle = current_angle - target_angle;
    return target_angle;
}

// Function to rotate arm
float rotate_arm(float current_angle, float target_angle) {
    float rotation_angle = current_angle - target_angle;
    return target_angle;
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure;
  // receive image coordinates
  float arm_angle = 0;
  float base_angle = 0;
  float shoulder_x = 0; // in meters (likely will actually be 0)
  float shoulder_y = 0; // in meters

  float Z = measure.RangeMilliMeter * 1000; // lidar output is in mm so need to convert to meters
  float laser_x, laser_y;
  calculate_laser_coordinates(arm_angle, laser_x, laser_y);
  float image_x, image_y;
  find_coordinates_from_image(image);
  float X, Y;
  find_scene_coordinates(image_x, image_y, Z, f);
  float angle_base, angle_laser;
  calculate_inverse_kinematics(X, Y, shoulder_x, shoulder_y, laser_x, laser_y, Z);

  base_angle = rotate_base(base_angle);
  arm_angle = rotate_arm(arm_angle);
}
