# Arduino-Biometric-Keypad-Lock-System

A secure embedded access control system based on two-factor authentication using PIN input and fingerprint verification. The system is implemented on Arduino UNO with an OLED display, keypad, and R308 fingerprint sensor.

---

## Overview

This project implements a two-stage authentication mechanism for access control:

1. PIN verification via a 4x3 keypad
2. Fingerprint verification after successful PIN entry

Access is granted only if both authentication steps succeed. Real-time system feedback is displayed on an SH1106 OLED screen.

---

## Hardware Components

- Arduino UNO
- 4x3 Matrix Keypad
- SH1106 OLED Display (I2C)
- R308 Fingerprint Sensor
- Jumper wires and breadboard

---

## Key Features

- 4-digit PIN-based authentication
- Fingerprint verification using Adafruit Fingerprint library
- Two-factor authentication workflow
- OLED-based status display
- Limited fingerprint retry mechanism
- Input reset and timeout handling for security

---

## System Workflow

1. System prompts user to enter PIN
2. If PIN is correct, fingerprint verification is initiated
3. If fingerprint matches authorized ID, access is granted
4. Failed attempts trigger retry logic and system reset

---

## Security Design

- Static 4-digit PIN authentication
- Single authorized fingerprint ID
- Maximum of 3 fingerprint attempts
- Timeout-based session handling

---

## Libraries Used

- Adafruit_GFX
- Adafruit_SH110X
- Keypad
- Adafruit_Fingerprint
- SoftwareSerial
- Wire

---

## Future Improvements

- Servo-based physical locking mechanism
- EEPROM-based PIN storage
- Multi-user fingerprint support
- Alarm or buzzer for failed attempts
- Admin mode for fingerprint enrollment

