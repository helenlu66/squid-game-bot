#include "Adafruit_VL53L0X.h"
#include <MPU6050_tockn.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"
#include <Arduino_JSON.h>



int base_pin = 32;
int hand_pin = 33;
int green = 32;
int yellow = 34;
int red = 35;
#define L2 0.05 // length of arm joint in meters
#define f 0.01  // focal length of camera in meters
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
int sda_pin=21, scl_pin=22;
Servo servo_base;
Servo servo_hand;
int base_servo_offset = 150;
int calc_flag = 0;
float x = -100;
float y = -100;
pinMode(green, OUTPUT);
pinMode(yellow, OUTPUT);
pinMode(red, OUTPUT);

const char* ssid = "Tufts_Robot";
const char* password = "";
const char* param_servo = "servo";
const char* param_target = "target";
AsyncWebServer server(80);


unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;





void calculate_laser_coordinates(float arm_angle, float &relative_laser_x, float &relative_laser_y) {
  float arm_angle_rad = arm_angle * (PI / 180.0); // Convert to radians
  relative_laser_x = L2 * cos(arm_angle_rad);
  relative_laser_y = L2 * sin(arm_angle_rad);
}

// Function to find scene coordinates from image coordinates, Z distance, and focal length f
void find_scene_coordinates(float image_x, float image_y, float Z, float f1, float *X, float *Y) {
    *X = (image_x * Z) / f1;
    *Y = (image_y * Z) / f1;
}

// Function to calculate inverse kinematics
float calculate_inverse_kinematics(float player_x, float player_y, float shoulder_x, float shoulder_y, float laser_x, float laser_y, float Z, float base_angle_deg, float laser_angle_deg) {
    float res[2];
    float absolute_laser_x = shoulder_x + laser_x;
    float absolute_laser_y = shoulder_y + laser_y;
    Serial.print("Start inverse!");
    // Calculate the angles in radians
    float angle_base = atan2(player_x - absolute_laser_x, Z);
    Serial.print("angle_base:");
    Serial.println(angle_base);
    float angle_laser = atan2(player_y - absolute_laser_y, Z);
    Serial.print("angle_laser:");
    Serial.println(angle_laser);
    // Convert angles to degrees
    base_angle_deg = 0;
    Serial.print("check pointer:");
    Serial.println(base_angle_deg);
    base_angle_deg = float(angle_base * (180.0 / 3.14));
    float base_angle_deg1 = angle_base * (180.0 / 3.14);
  Serial.print("angle_base_deg:");
    Serial.println(base_angle_deg1);
    laser_angle_deg = angle_laser * (180.0 / 3.14);
    return base_angle_deg;
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
int angle = 0;
float angle_calc(float image_x,float image_y) {
  // if (Serial.available() > 0) {
  // String input = Serial.readStringUntil('\n');
  // angle = std::stoi(input.c_str());

  // Serial.print(angle,'\n');
  // servo_base.write(angle);
  // servo_hand.write(angle);}
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false); 
  // receive image coordinates
  float arm_angle = 0;
  // float base_angle = 0;
  float shoulder_x = 0.1; // in meters (likely will actually be 0)
  float shoulder_y = 0.1; // in meters
  
  float Z = measure.RangeMilliMeter * 1000; // lidar output is in mm so need to convert to meters
  Serial.print(Z);
  Z=0.1;
  float laser_x, laser_y;
  calculate_laser_coordinates(arm_angle, laser_x, laser_y);
  Serial.print("laser_x");
  Serial.println(laser_x);
  Serial.print("laser_y:");
  Serial.println(laser_y);
  // float image_x, image_y;
  // find_coordinates_from_image(image);
  float *X, *Y;
  float *base_angle_deg, *laser_angle_deg;
  *base_angle_deg = 0;
  find_scene_coordinates(image_x, image_y, Z, f, X, Y);
  Serial.print("*X");
  Serial.println(*X);
  float angle_base, angle_laser;
  Serial.print("*base_angle_deg");
  Serial.println(*base_angle_deg);
  float base_ag_deg = calculate_inverse_kinematics(*X+0.1, *Y+0.1, shoulder_x, shoulder_y, laser_x, laser_y, Z, 0, 0);
  // base_angle = rotate_base(base_angle);
  // arm_angle = rotate_arm(arm_angle);
  // Send an HTTP POST request every 10 minutes
  // Serial.print(res[0]);
  delete X, Y, base_angle_deg, laser_angle_deg;
  delay(100);
  Serial.println("function completed!");
  return base_ag_deg;
}

void setup() {
  Serial.begin(9600);
  servo_base.attach(base_pin);
  servo_hand.attach(hand_pin);
  Wire.begin(sda_pin, scl_pin);
  Serial.println("Adafruit VL53L0X test");
  // Wire.begin(sda_pin, scl_pin);

  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
  while(1);
  }
  // SerialBT.begin("Squid-game-bot-main");


  // Adafruit_VL53L0X lox = Adafruit_VL53L0X();
  // Initialize sensor (if needed)
  // sensor.init();

  //wifi server
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println(WiFi.localIP());
  server.on("/start", HTTP_GET, [](AsyncWebServerRequest * request) {
 
    request->send_P(200, "text/plain", "game started");

  });
    AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/post-message", [](AsyncWebServerRequest *request, JsonVariant &json) {
    StaticJsonDocument<200> data;
    if (json.is<JsonArray>())
    {
      data = json.as<JsonArray>();
    }
    else if (json.is<JsonObject>())
    {
      data = json.as<JsonObject>();
    }
    String response;
    serializeJson(data, response);
    request->send(200, "application/json", response);
    if(example.contains("light")) {
        light = data["light"];
        calc_flag = 2;
    } else {
        // Serial.println(response);
        x= data["x"];
        y = data["y"];
        // Serial.println(x);
        calc_flag = 1;
    }
  });
  
  server.addHandler(handler);
  server.begin();
}
void loop(){

  if(calc_flag==1)
  {
    calc_flag = 0;
    float servo_angle= angle_calc(x,y);
    
    Serial.println(servo_angle);
    delay(500);
    
  } else if (calc_flag==2) {
    calc_flag = 0;
    if (light == 'off') {
      digitalWrite(green, LOW);
      digitalWrite(yellow, LOW);
      digitalWrite(red, LOW);
    } else {
      digitalWrite(green, LOW);
      digitalWrite(yellow, LOW);
      digitalWrite(red, LOW);
      digitalWrite(light, HIGH);
  }
  delay(500);
}


