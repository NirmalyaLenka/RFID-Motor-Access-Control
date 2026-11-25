# RFID Motor Access Control

Arduino RFID prototype that reads an MFRC522 card UID and, in the primary sketch, runs a DC motor through an L298N driver for a fixed interval when the UID matches an allow-list. This repository contains **three different sketches**; their pin assignments and hardware features are not interchangeable.

## Choose the sketch first

| File | Implemented behavior | Important distinction |
|---|---|---|
| `rfid_motor_control.ino` | MFRC522 reader, UID allow-list, red/green LEDs, L298N motor direction and PWM, timed motor stop | **Primary sketch for the motor-driver wiring below.** No OLED or buzzer. |
| `rfid_access_control (1).ino` | MFRC522, SSD1306 OLED, buzzer, red/green LEDs, separate allowed and denied UID lists | Does not control a motor. Uses RC522 RST on D9 and OLED I²C on A4/A5. |
| `rfid_access_control (2).ino` | Same OLED/buzzer/LED arrangement plus a single motor-control output | Uses D5 as a HIGH/LOW motor-driver control signal; it does not implement the L298N direction/PWM mapping used by the primary sketch. |

Do not upload one sketch while following another sketch's pin table. The two OLED sketches also contain hard-coded example UIDs; inspect and replace them before use.

## Hardware for the primary sketch

- Arduino Uno or compatible board with the standard SPI pins
- MFRC522/RC522 RFID reader and compatible card/tag
- L298N motor driver and a DC motor
- Separate motor power supply suitable for the motor and driver
- One green LED and one red LED, each with a current-limiting resistor
- Jumper wires and a breadboard or suitable connections

The RC522 is a **3.3 V device**. Do not connect its VCC to 5 V. Check the specific breakout board's logic-level requirements as well.

## Wiring: `rfid_motor_control.ino`

### RC522 to Arduino

| RC522 | Arduino Uno |
|---|---|
| SDA / SS | D10 |
| SCK | D13 |
| MOSI | D11 |
| MISO | D12 |
| RST | D3 |
| VCC | 3.3 V |
| GND | GND |
| IRQ | Not connected |

### L298N and indicators

| Signal | Arduino / supply |
|---|---|
| IN1 | D7 |
| IN2 | D8 |
| ENA | D9 (PWM) |
| Driver GND | Arduino GND (common ground) |
| Driver motor supply | External supply appropriate for the motor/driver |
| Motor terminals | L298N OUT1 and OUT2 |
| Green LED anode | D4 through a 220 Ω resistor |
| Red LED anode | D5 through a 220 Ω resistor |
| LED cathodes | GND |

Never power a motor directly from an Arduino GPIO. Keep the motor supply disconnected while wiring, verify the driver module's power-jumper instructions, and ensure the mechanism can move safely before testing. The motor's direction depends on the actual motor and output wiring.

## Software setup and upload

1. Install the Arduino IDE.
2. In **Library Manager**, install **MFRC522** by GithubCommunity.
3. Open `rfid_motor_control.ino` from the repository root.
4. Select the actual board and serial port in **Tools**.
5. Upload the sketch and open Serial Monitor at **9600 baud**.
6. Scan a card and read its UID from the serial output.
7. Replace the placeholder values in `AUTHORIZED_UIDS` with the exact four UID bytes for cards you intend to allow, then upload again.

The primary sketch defines:

```cpp
#define MOTOR_SPEED    200   // PWM value: 0–255
#define MOTOR_RUN_TIME 5000  // milliseconds
```

Change these only after confirming the motor and driver are appropriate for the application. The current authorization function accepts **four-byte UIDs only**; seven-byte tags are not supported by this sketch as written.

## How the primary sketch behaves

1. The reader detects a card and prints its UID to Serial Monitor.
2. The UID is compared with the hard-coded allow-list.
3. A match turns on the green LED and drives the L298N in one direction.
4. The motor stops after `MOTOR_RUN_TIME`; the timeout is checked using `millis()`.
5. A non-matching UID turns on the red LED for about two seconds and does not start the motor.

This is an educational prototype, not a security-grade access-control product. RFID UIDs can be copied, the allow-list is stored in firmware, and scanned UIDs are printed to the serial console. Do not use this alone to protect property or safety-critical equipment.

## Repository contents

- `rfid_motor_control.ino` — primary L298N motor-control sketch
- `rfid_access_control (1).ino` — OLED/buzzer/LED variant without motor control
- `rfid_access_control (2).ino` — OLED/buzzer/LED variant with a single motor-control output
- `rfid-access-control.tar.gz` and `rfid-access-control-final.tar.gz` — checked-in archive files; their contents have not been extracted or audited in this review
- `README.md` — this guide

## Troubleshooting

| Symptom | Checks |
|---|---|
| Reader is not detected | Confirm 3.3 V power, SPI wiring, SS on D10, RST on D3 for the primary sketch, and library installation. |
| All cards are denied | Compare the complete UID printed in Serial Monitor with the four bytes in `AUTHORIZED_UIDS`. |
| Motor does not run | Confirm the correct sketch is uploaded, the external motor supply is present, driver wiring is correct, and grounds are common. |
| Motor runs in the opposite direction | Power down and check motor leads and IN1/IN2 wiring; direction depends on the physical wiring. |
| OLED sketch stops during startup | Check the SSD1306 I²C wiring and address (the sketch uses 0x3C); the OLED variants have a different pin map. |

## License and verification status

No conventional root-level `LICENSE` file is present in the current repository tree, so redistribution terms are not established by the tracked root files. Verify licensing before reusing or distributing the project.

Documentation was reviewed against the three visible sketches and root file list. No compilation, upload, reader, motor, or OLED hardware test was performed.

## Maintenance & Audit

- **Audit date:** 20 November 2026
- **Documentation status:** README reviewed; the three sketch variants and their different wiring requirements are now distinguished.
- **Verification boundary:** Documentation was compared with the checked-in text files. No build, security, or hardware result is claimed.
- **Known limitations:** UID-only authorization is cloneable; only the primary sketch has the documented L298N PWM/direction control and timed stop.

## Contact

Maintained up to 25 November 2026.

If you encounter any problems, please contact [carmodbhai@gmail.com](mailto:carmodbhai@gmail.com).
