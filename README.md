# ESP32 Smart Curtain System

This project automates curtain movement using an **ESP32**, **L298N motor driver**, and **ACS712 current sensor** for jam protection. It integrates with **Home Assistant** via **MQTT**, allowing remote control, automation, and feedback.

---

## Features

* Timed curtain open/close
* Jam detection via current sensing
* MQTT integration with Home Assistant
* Powered by 12V power supply with buck converter for ESP32

---

## Hardware Required

| Component            | Description                                 |
| -------------------- | ------------------------------------------- |
| ESP32 Dev Board      | Main controller                             |
| L298N Motor Driver   | Motor control (H-Bridge)                    |
| ACS712 5A            | Current sensor for jam detection            |
| 12V DC Motor         | Curtain motor                               |
| 12V Power Supply     | Powers motor and ESP32 (via buck converter) |
| Buck Converter       | 12V → 5V for ESP32                          |
| Wires, Curtain Strip | Mechanical setup                            |

---

## Wiring Diagram

| ESP32 GPIO | Connected To              | Description                 |
| ---------- | ------------------------- | --------------------------- |
| 25         | ENA (L298N)               | PWM motor speed             |
| 14         | IN1 (L298N)               | Motor direction control     |
| 27         | IN2 (L298N)               | Motor direction control     |
| GND        | GND (L298N, ACS712, Buck) | Common ground               |
| VIN        | 5V Output from Buck       | ESP32 power input           |
| 34         | ACS712 OUT                | Analog current sensor input |

### Pin Summary

* **ENA (GPIO 25):** PWM signal to control motor speed
* **IN1 (GPIO 14):** Motor direction control pin 1
* **IN2 (GPIO 27):** Motor direction control pin 2
* **CURRENT\_SENSOR\_PIN (GPIO 34):** Analog input from ACS712 for current measurement
* **VIN:** Connect to 5V output of buck converter (converted from 12V power supply)
* **GND:** Shared ground between ESP32, motor driver, sensor, and power source

---

## Home Assistant Configuration

Add the following to your `configuration.yaml`:

```yaml
cover:
  - platform: mqtt
    name: "Smart Curtain"
    command_topic: "smartcurtain/command"
    state_topic: "smartcurtain/status"
    payload_open: "open"
    payload_close: "close"
    payload_stop: "stop"
    state_open: "opening"
    state_closed: "closed"
    state_stopped: "stopped"
    optimistic: false
    retain: true
```

Example automation or dashboard button can send MQTT payloads like `open`, `close`, or `stop` to topic `smartcurtain/command`.

---

## MQTT Topics

| Topic                  | Description                              |
| ---------------------- | ---------------------------------------- |
| `smartcurtain/command` | `open`, `close`, `stop`                  |
| `smartcurtain/status`  | `opening`, `closed`, `jammed`, `stopped` |

---

## Safety

This project uses an ACS712 current sensor to detect motor jams. If current exceeds the configured threshold (e.g., 2A), the system stops the motor and sends a "jammed" status.

---

## Installation

1. Flash the ESP32 with the provided PlatformIO code.
2. Configure `secrets.h` with your Wi-Fi and MQTT broker.
3. Wire components as listed above.
4. Add MQTT integration in Home Assistant.

---

## Status & Expansion Ideas

* Add limit switches for position feedback
* Enable bidirectional control (open/close)
* Use ESPHome for easier integration
* OTA firmware update support
