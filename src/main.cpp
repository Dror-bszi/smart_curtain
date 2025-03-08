#include <Arduino.h>

// Motor Control Pins (L298N)
#define ENA 25   // PWM Speed Control
#define IN1 14   // Motor Direction 1
#define IN2 27   // Motor Direction 2

// Load Sensor (ACS712-05A) Pin
#define CURRENT_SENSOR_PIN 34  // Analog input for ACS712

// Current Sensor Calibration for 5A version
const float ACS712_SENSITIVITY = 0.185;  // 185mV per A for ACS712-05A
const float ZERO_CURRENT_VOLTAGE = 2.42;  // Adjust this based on your test
const int NUM_SAMPLES = 100;  // Increased samples for smoother readings

void setup() {
    Serial.begin(115200);

    // Set up motor control pins
    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);

    // Run the motor continuously
    digitalWrite(IN1, HIGH);  // Motor direction 1
    digitalWrite(IN2, LOW);   // Motor direction 2
    analogWrite(ENA, 200);    // Set motor speed (0-255)

    Serial.println("Motor is running...");
}

void loop() {
    // Measure the current using ACS712
    float sum = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        sum += analogRead(CURRENT_SENSOR_PIN);
        delay(2);  // Small delay to stabilize the reading
    }
    float avgReading = sum / NUM_SAMPLES;

    float voltage = (avgReading / 4095.0) * 3.3;  // Convert ADC to Voltage
    float current = (voltage - ZERO_CURRENT_VOLTAGE) / ACS712_SENSITIVITY;

    // Print current in Amps
    Serial.print("Current: ");
    Serial.print(abs(current));  // Take absolute value to ignore direction
    Serial.println(" A");

    delay(1000);  // Slow down the output for easier reading
}
