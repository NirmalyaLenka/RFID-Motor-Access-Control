/*
 * RFID Motor Access Control
 * -------------------------
 * Authorized card → Motor spins CW + Green LED ON
 * Unauthorized card → Motor stops  + Red LED ON
 *
 * Hardware:
 *   RC522 RFID  → SPI pins (see wiring below)
 *   L298N       → IN1=7, IN2=8, ENA=9 (PWM)
 *   Green LED   → Pin 4 (via 220Ω resistor)
 *   Red LED     → Pin 5 (via 220Ω resistor)
 *
 * Wiring (RC522 → Arduino Uno/Nano):
 *   SDA  → D10
 *   SCK  → D13
 *   MOSI → D11
 *   MISO → D12
 *   RST  → D9  ← change RST_PIN if clash with ENA
 *   3.3V → 3.3V
 *   GND  → GND
 *
 * NOTE: RC522 RST moved to pin 3 to free pin 9 for PWM.
 *
 * Library required: MFRC522 by GithubCommunity
 */

#include <SPI.h>
#include <MFRC522.h>

// ── Pin definitions ──────────────────────────────────────────────────────────
#define SS_PIN    10   // RC522 SDA/SS
#define RST_PIN    3   // RC522 RST (NOT pin 9 – that's ENA)

#define IN1        7   // L298N direction pin A
#define IN2        8   // L298N direction pin B
#define ENA        9   // L298N enable / speed (PWM)

#define LED_GREEN  4   // Green LED anode
#define LED_RED    5   // Red LED anode

// ── Motor speed (0–255) ──────────────────────────────────────────────────────
#define MOTOR_SPEED   200   // ~78% duty cycle; adjust as needed

// ── Motor run duration after card tap (ms) ──────────────────────────────────
#define MOTOR_RUN_TIME  5000   // 5 seconds

// ── Authorized card UIDs ─────────────────────────────────────────────────────
// Add/remove UIDs as needed. Each UID is 4 bytes (standard MIFARE 1K/4K).
// To find your card's UID: upload a UID-dump sketch or read Serial output.
const byte AUTHORIZED_UIDS[][4] = {
  { 0xDE, 0xAD, 0xBE, 0xEF },   // ← replace with your card UID
  { 0xAA, 0xBB, 0xCC, 0xDD },   // ← add more cards here
};
const uint8_t NUM_AUTHORIZED = sizeof(AUTHORIZED_UIDS) / sizeof(AUTHORIZED_UIDS[0]);

// ── MFRC522 instance ─────────────────────────────────────────────────────────
MFRC522 rfid(SS_PIN, RST_PIN);

// ── State tracking ───────────────────────────────────────────────────────────
bool      motorRunning    = false;
unsigned long motorStartTime = 0;

// ── Function prototypes ──────────────────────────────────────────────────────
bool isAuthorized(byte *uid, byte uidSize);
void motorClockwise();
void motorStop();
void setLEDs(bool greenOn, bool redOn);

// ────────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  motorStop();
  setLEDs(false, false);

  Serial.println(F("RFID Motor Access Control Ready"));
  Serial.println(F("Tap a card..."));
}

// ────────────────────────────────────────────────────────────────────────────
void loop() {
  // ── Non-blocking motor timeout ───────────────────────────────────────────
  if (motorRunning && (millis() - motorStartTime >= MOTOR_RUN_TIME)) {
    motorStop();
    setLEDs(false, false);
    motorRunning = false;
    Serial.println(F("Motor stopped. Tap card again."));
  }

  // ── Wait for new card ────────────────────────────────────────────────────
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  // ── Print UID ────────────────────────────────────────────────────────────
  Serial.print(F("Card UID: "));
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();

  // ── Access decision ──────────────────────────────────────────────────────
  if (isAuthorized(rfid.uid.uidByte, rfid.uid.size)) {
    Serial.println(F("ACCESS GRANTED – Motor CW, Green LED ON"));
    setLEDs(true, false);
    motorClockwise();
    motorRunning   = true;
    motorStartTime = millis();
  } else {
    Serial.println(F("ACCESS DENIED  – Red LED ON"));
    setLEDs(false, true);
    motorStop();
    motorRunning = false;
    delay(2000);          // Show red LED for 2 s
    setLEDs(false, false);
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// ────────────────────────────────────────────────────────────────────────────
// Returns true if uid matches any entry in AUTHORIZED_UIDS
bool isAuthorized(byte *uid, byte uidSize) {
  if (uidSize != 4) return false;   // Only support 4-byte UIDs here
  for (uint8_t i = 0; i < NUM_AUTHORIZED; i++) {
    if (memcmp(uid, AUTHORIZED_UIDS[i], 4) == 0) {
      return true;
    }
  }
  return false;
}

// ────────────────────────────────────────────────────────────────────────────
// Spin motor clockwise: IN1=HIGH, IN2=LOW, ENA=PWM
void motorClockwise() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, MOTOR_SPEED);
}

// ────────────────────────────────────────────────────────────────────────────
// Stop motor: ENA=0 (fast stop)
void motorStop() {
  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

// ────────────────────────────────────────────────────────────────────────────
void setLEDs(bool greenOn, bool redOn) {
  digitalWrite(LED_GREEN, greenOn ? HIGH : LOW);
  digitalWrite(LED_RED,   redOn   ? HIGH : LOW);
}
