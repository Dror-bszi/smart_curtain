#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "secrets.h"

// Motor Control Pins (L298N)
#define ENA 25
#define IN1 14
#define IN2 27

// Current Sensor (ACS712-05A)
#define CURRENT_SENSOR_PIN 34
const float ACS712_SENSITIVITY = 0.185;
const float ZERO_CURRENT_VOLTAGE = 2.42;
const int NUM_SAMPLES = 100;

// Curtain control
const int MOTOR_SPEED = 200;
const unsigned long ROLL_TIME_MS = 8000;
const float JAM_CURRENT_THRESHOLD = 2.0;  // Amps

WiFiClient espClient;
PubSubClient client(espClient);

enum CurtainState { STOPPED, OPENING, CLOSING };
CurtainState currentState = STOPPED;
unsigned long actionStartTime = 0;

void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected");
}

void connectToMQTT() {
  client.setServer(MQTT_BROKER, MQTT_PORT);
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32Curtain", MQTT_USER, MQTT_PASSWD)) {
      Serial.println("connected");
      client.subscribe(MQTT_COMMAND_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(1000);
    }
  }
}

float readCurrent() {
  float sum = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += analogRead(CURRENT_SENSOR_PIN);
    delay(2);
  }
  float avg = sum / NUM_SAMPLES;
  float voltage = (avg / 4095.0) * 3.3;
  float current = (voltage - ZERO_CURRENT_VOLTAGE) / ACS712_SENSITIVITY;
  return abs(current);
}

void stopMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
  currentState = STOPPED;
  client.publish(MQTT_STATUS_TOPIC, "stopped", true);
  Serial.println("Motor stopped");
}

void runMotor(CurtainState direction) {
  if (direction == OPENING) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    client.publish(MQTT_STATUS_TOPIC, "opening", true);
  } else if (direction == CLOSING) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    client.publish(MQTT_STATUS_TOPIC, "closing", true);
  }
  analogWrite(ENA, MOTOR_SPEED);
  currentState = direction;
  actionStartTime = millis();
  Serial.println("Motor running...");
}

void callback(char* topic, byte* payload, unsigned int length) {
  String command;
  for (int i = 0; i < length; i++) {
    command += (char)payload[i];
  }
  command.trim();

  if (command == "open") runMotor(OPENING);
  else if (command == "close") runMotor(CLOSING);
  else if (command == "stop") stopMotor();
}

void setup() {
  Serial.begin(115200);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(CURRENT_SENSOR_PIN, INPUT);

  connectToWiFi();
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) connectToMQTT();
  client.loop();

  if (currentState != STOPPED) {
    if (millis() - actionStartTime > ROLL_TIME_MS) {
      Serial.println("Timed stop");
      stopMotor();
    }

    float current = readCurrent();
    Serial.print("Current: ");
    Serial.print(current);
    Serial.println(" A");

    if (current > JAM_CURRENT_THRESHOLD) {
      Serial.println("Jam detected!");
      client.publish(MQTT_STATUS_TOPIC, "jammed", true);
      stopMotor();
    }
  }
}
