/*
 * ============================================================
 *  RFID Access Control System
 *  Hardware : Arduino Uno + MFRC522 + SSD1306 OLED (128x64)
 *  Author   : NirmalyaLenka
 *  Repo     : https://github.com/NirmalyaLenka/rfid-access-control
 * ============================================================
 *
 *  Wiring Summary
 *  ──────────────
 *  MFRC522  →  Arduino Uno
 *    SDA    →  D10  (SS)
 *    SCK    →  D13
 *    MOSI   →  D11
 *    MISO   →  D12
 *    IRQ    →  (not connected)
 *    GND    →  GND
 *    RST    →  D9
 *    3.3V   →  3.3V
 *
 *  SSD1306 OLED (I2C)  →  Arduino Uno
 *    VCC    →  3.3V / 5V
 *    GND    →  GND
 *    SCL    →  A5
 *    SDA    →  A4
 *
 *  Buzzer  →  D8
 *  Green LED → D7   (Access Granted)
 *  Red LED   → D6   (Access Denied)
 *
 *  Libraries required (install via Library Manager):
 *    - MFRC522  by GithubCommunity
 *    - Adafruit SSD1306
 *    - Adafruit GFX Library
 * ============================================================
 */

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ── Pin Definitions ─────────────────────────────────────────
#define SS_PIN      10
#define RST_PIN      9
#define BUZZER_PIN   8
#define LED_GREEN    7
#define LED_RED      6

// ── OLED Config ──────────────────────────────────────────────
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1   // Reset pin not used
#define OLED_ADDRESS  0x3C

// ── Objects ──────────────────────────────────────────────────
MFRC522 rfid(SS_PIN, RST_PIN);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ── Authorised UIDs ──────────────────────────────────────────
//  Each entry: 4 bytes.  Add more rows as needed.
const byte AUTHORIZED_UIDS[][4] = {
  { 0x13, 0xC0, 0x92, 0xF5 }   // Card 1  – ANDAR AJAO
};
const byte DENIED_UIDS[][4] = {
  { 0xC0, 0x8B, 0x16, 0x25 }   // Card 2  – ACCESS DENIED
};

const int NUM_AUTHORIZED = sizeof(AUTHORIZED_UIDS) / sizeof(AUTHORIZED_UIDS[0]);
const int NUM_DENIED     = sizeof(DENIED_UIDS)     / sizeof(DENIED_UIDS[0]);

// ── Timing ───────────────────────────────────────────────────
#define DISPLAY_HOLD_MS   2500   // How long to show result
#define BEEP_SHORT_MS      100
#define BEEP_LONG_MS       600

// ── Helpers ──────────────────────────────────────────────────

void beep(int duration_ms) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration_ms);
  digitalWrite(BUZZER_PIN, LOW);
}

void doubleBeep() {
  beep(BEEP_SHORT_MS); delay(80); beep(BEEP_SHORT_MS);
}

bool uidMatches(const byte* scanned, const byte* reference, byte len = 4) {
  for (byte i = 0; i < len; i++) {
    if (scanned[i] != reference[i]) return false;
  }
  return true;
}

// ── Display Helpers ──────────────────────────────────────────

void showIdle() {
  display.clearDisplay();

  // Top bar
  display.fillRect(0, 0, SCREEN_WIDTH, 14, WHITE);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.setCursor(18, 3);
  display.print("RFID ACCESS SYSTEM");

  // Prompt
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(12, 24);
  display.print("Scan your card...");

  // Bottom icon hint
  display.drawRect(54, 42, 20, 16, WHITE);
  display.drawRect(56, 38, 16, 6, WHITE);
  display.setCursor(22, 48);
  display.print("<< RFID >>");

  display.display();
}

void showGranted() {
  display.clearDisplay();

  display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
  display.setTextColor(BLACK);

  display.setTextSize(2);
  display.setCursor(28, 4);
  display.print("ACCESS");
  display.setCursor(25, 24);
  display.print("GRANTED");

  display.drawLine(0, 44, SCREEN_WIDTH, 44, BLACK);

  display.setTextSize(1);
  display.setCursor(4, 50);
  display.print("ANDAR AJAO");

  display.display();
}

void showDenied() {
  display.clearDisplay();

  // Red-style – inverted block
  display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
  display.setTextColor(BLACK);

  display.setTextSize(2);
  display.setCursor(22, 4);
  display.print("ACCESS");
  display.setCursor(22, 24);
  display.print("DENIED");

  display.drawLine(0, 44, SCREEN_WIDTH, 44, BLACK);

  display.setTextSize(1);
  // Message split across two lines to fit 128px width
  display.setCursor(0, 50);
  display.print("HAT JAO LODE KE SAMNE SE");

  display.display();
}

void showUnknown() {
  display.clearDisplay();
  display.setTextColor(WHITE);

  display.setTextSize(1);
  display.setCursor(25, 10);
  display.print("UNKNOWN CARD");

  display.setTextSize(1);
  display.setCursor(10, 30);
  display.print("Not registered.");
  display.setCursor(10, 44);
  display.print("Contact admin.");

  display.display();
}

// ── Setup ────────────────────────────────────────────────────

void setup() {
  Serial.begin(9600);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_GREEN,  OUTPUT);
  pinMode(LED_RED,    OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_GREEN,  LOW);
  digitalWrite(LED_RED,    LOW);

  SPI.begin();
  rfid.PCD_Init();

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("SSD1306 init failed"));
    while (true);
  }

  display.clearDisplay();
  display.display();

  // Boot splash
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(20, 18);
  display.print("Initialising...");
  display.setCursor(8, 36);
  display.print("RFID Access Control");
  display.display();
  delay(1500);

  showIdle();
  Serial.println(F("System ready. Waiting for card..."));
}

// ── Main Loop ────────────────────────────────────────────────

void loop() {
  // Wait for a card
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  byte* uid = rfid.uid.uidByte;
  byte  len = rfid.uid.size;

  // Print UID to Serial for debugging / enrolling new cards
  Serial.print(F("Card UID: "));
  for (byte i = 0; i < len; i++) {
    if (uid[i] < 0x10) Serial.print('0');
    Serial.print(uid[i], HEX);
    if (i < len - 1) Serial.print(' ');
  }
  Serial.println();

  // ── Check denied list first ──────────────────────────────
  bool isDenied = false;
  for (int i = 0; i < NUM_DENIED; i++) {
    if (uidMatches(uid, DENIED_UIDS[i])) {
      isDenied = true;
      break;
    }
  }

  if (isDenied) {
    Serial.println(F(">> ACCESS DENIED"));
    showDenied();
    digitalWrite(LED_RED, HIGH);
    beep(BEEP_LONG_MS);
    delay(DISPLAY_HOLD_MS);
    digitalWrite(LED_RED, LOW);
    showIdle();
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return;
  }

  // ── Check authorised list ────────────────────────────────
  bool isAuthorized = false;
  for (int i = 0; i < NUM_AUTHORIZED; i++) {
    if (uidMatches(uid, AUTHORIZED_UIDS[i])) {
      isAuthorized = true;
      break;
    }
  }

  if (isAuthorized) {
    Serial.println(F(">> ACCESS GRANTED - ANDAR AJAO"));
    showGranted();
    digitalWrite(LED_GREEN, HIGH);
    doubleBeep();
    delay(DISPLAY_HOLD_MS);
    digitalWrite(LED_GREEN, LOW);
    showIdle();
  } else {
    // Unknown card
    Serial.println(F(">> UNKNOWN CARD"));
    showUnknown();
    beep(BEEP_SHORT_MS);
    delay(DISPLAY_HOLD_MS);
    showIdle();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
