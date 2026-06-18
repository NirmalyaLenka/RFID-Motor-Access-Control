# RFID Motor Access Control
 
Control a DC motor and LED indicators based on RFID card authorization.
 
- ✅ **Authorized card** → Motor spins **clockwise** + 🟢 Green LED ON
- ❌ **Unauthorized card** → Motor stops + 🔴 Red LED ON
---
 
## Hardware Required
 
| Component | Quantity |
|-----------|----------|
| Arduino Uno / Nano | 1 |
| RC522 RFID Module | 1 |
| MIFARE Card or Key Fob | 1+ |
| L298N Motor Driver Module | 1 |
| DC Motor (5–12 V) | 1 |
| Green LED | 1 |
| Red LED | 1 |
| 220Ω Resistor | 2 |
| Jumper Wires | several |
| 9–12 V Power Supply (for motor) | 1 |
 
---
 
## Wiring
 
### RC522 RFID → Arduino
 
| RC522 Pin | Arduino Pin |
|-----------|-------------|
| SDA       | D10         |
| SCK       | D13         |
| MOSI      | D11         |
| MISO      | D12         |
| RST       | D3          |
| 3.3V      | 3.3V        |
| GND       | GND         |
 
> ⚠️ RC522 runs on **3.3 V only**. Connecting to 5 V will damage it.
 
### L298N Motor Driver → Arduino
 
| L298N Pin | Arduino Pin | Notes |
|-----------|-------------|-------|
| IN1       | D7          | Direction control |
| IN2       | D8          | Direction control |
| ENA       | D9          | PWM speed control |
| GND       | GND         | Common ground |
| 12V       | External PSU| Motor power supply |
 
> Connect the motor to the **OUT1 / OUT2** terminals on the L298N.
 
### LEDs → Arduino
 
| Component  | Arduino Pin | Via |
|------------|-------------|-----|
| Green LED (+) | D4 | 220Ω resistor |
| Red LED (+)   | D5 | 220Ω resistor |
| Both LED (–)  | GND | direct |
 
---
 
## Wiring Diagram (ASCII)
 
```
Arduino                RC522
   D10 ──────────────── SDA
   D13 ──────────────── SCK
   D11 ──────────────── MOSI
   D12 ──────────────── MISO
   D3  ──────────────── RST
   3V3 ──────────────── 3.3V
   GND ──────────────── GND
 
Arduino                L298N
   D7  ──────────────── IN1
   D8  ──────────────── IN2
   D9  ──────────────── ENA
   GND ──────────────── GND
   
   12V PSU ──────────── 12V (motor power)
   Motor ─────────────── OUT1 / OUT2
 
Arduino                LEDs
   D4  ── 220Ω ── Green LED (+) ── GND
   D5  ── 220Ω ── Red LED (+)   ── GND
```
 
---
 
## Software Setup
 
### 1. Install Library
 
Open **Arduino IDE → Sketch → Include Library → Manage Libraries**
 
Search for: `MFRC522` by **GithubCommunity** → Install
 
### 2. Find Your Card UID
 
Before adding authorized cards, you need their UID. Open Serial Monitor at **9600 baud**, tap your card — the UID prints automatically:
 
```
Card UID:  DE AD BE EF
```
 
### 3. Add Authorized UIDs
 
Edit this section in the sketch:
 
```cpp
const byte AUTHORIZED_UIDS[][4] = {
  { 0xDE, 0xAD, 0xBE, 0xEF },   // ← your card UID here
  { 0xAA, 0xBB, 0xCC, 0xDD },   // ← add more as needed
};
```
 
Replace placeholder values with your card's actual UID bytes.
 
### 4. Adjust Settings (optional)
 
```cpp
#define MOTOR_SPEED    200   // Motor PWM speed (0–255)
#define MOTOR_RUN_TIME 5000  // How long motor runs after tap (ms)
```
 
### 5. Upload
 
Select your board (**Tools → Board → Arduino Uno**) and port, then upload.
 
---
 
## How It Works
 
```
Card Tap
   │
   ├── UID in list?
   │       │
   │      YES → Green LED ON
   │             Motor spins CW for MOTOR_RUN_TIME ms
   │             Motor auto-stops after timeout
   │
   └──     NO → Red LED ON for 2 seconds
                Motor stays OFF
```
 
The loop is **non-blocking** — the motor timeout uses `millis()`, so the RFID reader stays responsive at all times.
 
---
 
## Serial Monitor Output
 
```
RFID Motor Access Control Ready
Tap a card...
Card UID:  DE AD BE EF
ACCESS GRANTED – Motor CW, Green LED ON
Motor stopped. Tap card again.
 
Card UID:  11 22 33 44
ACCESS DENIED  – Red LED ON
```
 
---
 
## Pin Summary
 
| Pin | Function |
|-----|----------|
| D3  | RC522 RST |
| D4  | Green LED |
| D5  | Red LED |
| D7  | L298N IN1 |
| D8  | L298N IN2 |
| D9  | L298N ENA (PWM) |
| D10 | RC522 SDA/SS |
| D11 | SPI MOSI |
| D12 | SPI MISO |
| D13 | SPI SCK |
 
---
 
## Extending the Project
 
| Feature | How |
|---------|-----|
| Add buzzer | `tone(BUZZER_PIN, 1000, 200)` on grant/deny |
| OLED feedback | Wire SSD1306 on I2C (A4/A5), use `Adafruit_SSD1306` |
| Counter-clockwise mode | Set IN1=LOW, IN2=HIGH in a second function |
| 7-byte UID cards | Change array size to `[][7]` and `uidSize != 7` check |
| Multiple access levels | Map UIDs to speed/duration levels |
 
---
 
## Troubleshooting
 
| Problem | Fix |
|---------|-----|
| Card not detected | Check SPI wiring; ensure 3.3V power to RC522 |
| Motor doesn't spin | Check ENA PWM pin; verify 12V supply on L298N |
| Motor spins wrong way | Swap IN1/IN2 or swap motor wire connections |
| LED not lighting | Verify resistor and correct pin; check LED polarity |
| UID always denied | Print UID to Serial and update `AUTHORIZED_UIDS` array |
 
---
 
## License
 
MIT — free to use and modify.
