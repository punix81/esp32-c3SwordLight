#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

// I2C ESP32-C3
#define SDA_PIN 20
#define SCL_PIN 21

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------------- Moisture sensor v1.2 ----------------
#define MOISTURE_PIN 0  // AO sur ADC (GPIO0) -> OK

// ✅ Calibration à ADAPTER avec TES MESURES RÉELLES
// - AirValue  = raw en AIR (capteur sec)
// - WaterValue= raw quand il est IMMERGÉ (ou terre saturée)
const int AirValue   = 2930;
const int WaterValue = 690;

// ✅ Optionnel: si tu veux "forcer" 100% un peu avant le vrai WaterValue (pour compenser un capteur qui plafonne)
// mets une valeur > 0 (ex: 30). Sinon laisse 0.
const int WetBoost = 0;

// ---------------- LED ----------------
#define LED_PIN 5

// ---------------- DHT11 ----------------
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ---------------- Utils ----------------
int clampInt(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

int readMoistureRawAvg(int pin, int samples = 25, int delayMs = 3) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delay(delayMs);
  }
  return (int)(sum / samples);
}

// ✅ Correction: conversion robuste + saturation à 0/100
// Objectif: si raw <= WaterValue (ou proche via WetBoost) => 100%
int rawToPercent(int raw) {
  int dry = AirValue;
  int wet = WaterValue;

  // sécurité si quelqu'un inverse par erreur
  if (dry < wet) { int tmp = dry; dry = wet; wet = tmp; }

  // clamp dur => eau = 100%
  if (raw >= dry) return 0;
  if (raw <= wet) return 100;

  long num = (long)(dry - raw) * 100;
  long den = (long)(dry - wet);
  int pct = (int)(num / den);

  return clampInt(pct, 0, 100);
}


// ---------------- UI helpers ----------------
void drawPill(int x, int y, int w, int h, bool filled) {
  if (filled) display.fillRoundRect(x, y, w, h, 4, SSD1306_WHITE);
  else        display.drawRoundRect(x, y, w, h, 4, SSD1306_WHITE);
}

void drawMiniProgress(int x, int y, int w, int h, int percent) {
  percent = clampInt(percent, 0, 100);
  display.drawRoundRect(x, y, w, h, 3, SSD1306_WHITE);
  int innerW = (w - 2);
  int fillW = (percent * innerW) / 100;
  display.fillRoundRect(x + 1, y + 1, fillW, h - 2, 3, SSD1306_WHITE);
}

void drawDropletIcon(int x, int y) {
  display.fillCircle(x + 4, y + 4, 3, SSD1306_WHITE);
  display.fillTriangle(x + 4, y - 2, x + 1, y + 4, x + 7, y + 4, SSD1306_WHITE);
}

void drawThermoIcon(int x, int y) {
  display.drawRect(x + 3, y, 3, 8, SSD1306_WHITE);
  display.fillCircle(x + 4, y + 10, 3, SSD1306_WHITE);
  display.drawLine(x + 4, y + 8, x + 4, y + 10, SSD1306_WHITE);
}

void drawHumIcon(int x, int y) {
  display.drawCircle(x + 4, y + 4, 3, SSD1306_WHITE);
  display.drawTriangle(x + 4, y - 2, x + 1, y + 4, x + 7, y + 4, SSD1306_WHITE);
}

const char* soilStatus(int p) {
  if (p < 30) return "DRY";
  if (p < 60) return "OK";
  return "WET";
}

void drawScreenModern(int raw, int soilPct, float t, float h) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // ===== Header =====
  display.fillRect(0, 0, 128, 14, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(4, 3);
  display.print("PLANT DASHBOARD");
  display.setTextColor(SSD1306_WHITE);

  // ===== Big soil % =====
  display.setTextSize(3);
  display.setCursor(2, 18);
  if (soilPct < 100) display.print(" ");
  display.print(soilPct);
  display.setTextSize(2);
  display.print("%");

  // ===== Status pill =====
  const char* st = soilStatus(soilPct);
  int pillW = 44, pillH = 12;
  int pillX = 128 - pillW - 4;
  int pillY = 16;
  drawPill(pillX, pillY, pillW, pillH, false);
  display.setTextSize(1);
  display.setCursor(pillX + 10, pillY + 3);
  display.print(st);

  // ===== Progress bar =====
  drawMiniProgress(72, 32, 52, 10, soilPct);

  // ===== Bottom cards =====
  const int cardY = 46;
  const int cardH = 16;
  const int cardW = 62;

  const int iconBoxW = 14;
  const int iconPad  = 4;
  const int textX    = iconPad + iconBoxW;
  const int textY    = cardY + 5;

  display.setTextSize(1);

  // --- TEMP ---
  int tempX = 2;
  display.drawRoundRect(tempX, cardY, cardW, cardH, 4, SSD1306_WHITE);
  drawThermoIcon(tempX + iconPad + 2, cardY + 3);
  display.setCursor(tempX + textX, textY);
  if (isnan(t)) display.print("--.-");
  else display.print(t, 1);

  // --- HUM ---
  int humX = 64;
  display.drawRoundRect(humX, cardY, cardW, cardH, 4, SSD1306_WHITE);
  drawHumIcon(humX + iconPad + 2, cardY + 3);
  display.setCursor(humX + textX, textY);
  if (isnan(h)) display.print("--");
  else display.print(h, 0);

  display.display();
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED init failed (addr 0x3C?)");
    while (true) delay(1000);
  }

  analogReadResolution(12);
  dht.begin();

  display.clearDisplay();
  display.display();
}

void loop() {
  int raw = readMoistureRawAvg(MOISTURE_PIN);
  int soilPct = rawToPercent(raw);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  Serial.print("soil raw=");
  Serial.print(raw);
  Serial.print("  soil=");
  Serial.print(soilPct);
  Serial.print("%  |  T=");
  Serial.print(isnan(t) ? -999 : t);
  Serial.print("C  H=");
  Serial.print(isnan(h) ? -999 : h);
  Serial.println("%");

  digitalWrite(LED_PIN, (soilPct < 30) ? HIGH : LOW);

  drawScreenModern(raw, soilPct, t, h);

  delay(1500);
}
