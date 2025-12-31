#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ezButton.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// 🔹 Broches I2C pour ESP32-C3
#define SDA_PIN 20
#define SCL_PIN 21

// 🔹 Boutons
#define BUTTON_MINUS_PIN 6
#define BUTTON_PLUS_PIN  7

// 🔹 LED
#define LED_PIN 5

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
ezButton buttonMinus(BUTTON_MINUS_PIN);
ezButton buttonPlus(BUTTON_PLUS_PIN);

int value = 12;
int ledBrightness = 0;
int fadeAmount = 5;
unsigned long lastFadeTime = 0;

unsigned long pressStartTime = 0;
bool buttonHeld = false;
int activeButton = 0; // -1 = minus, +1 = plus

void displayValue() {
  oled.clearDisplay();
  oled.setTextColor(WHITE);

  if (value > 0) {
    oled.setTextSize(3);
    oled.setCursor(value < 10 ? 55 : 45, 20);
    oled.println(value);
  } else {
    oled.setTextSize(2);
    oled.setCursor(10, 25);
    oled.println(F("Recharge"));
  }
  oled.display();
}

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true);
  }
  oled.setRotation(2);

  buttonMinus.setDebounceTime(50);
  buttonMinus.setCountMode(COUNT_FALLING);

  buttonPlus.setDebounceTime(50);
  buttonPlus.setCountMode(COUNT_FALLING);

  pinMode(LED_PIN, OUTPUT);

  displayValue();
}

void loop() {
  buttonMinus.loop();
  buttonPlus.loop();

  // --- Début appui ---
  if (buttonMinus.isPressed()) {
    pressStartTime = millis();
    buttonHeld = true;
    activeButton = -1;
  }

  if (buttonPlus.isPressed()) {
    pressStartTime = millis();
    buttonHeld = true;
    activeButton = +1;
  }

  // --- Relâchement bouton MINUS ---
  if (activeButton == -1 && buttonMinus.isReleased()) {
    unsigned long pressDuration = millis() - pressStartTime;

    if (pressDuration < 2000 && value > 0) {
      value--;
      Serial.println(value);
      displayValue();
    }

    buttonHeld = false;
    activeButton = 0;
  }

  // --- Relâchement bouton PLUS ---
  if (activeButton == +1 && buttonPlus.isReleased()) {
    unsigned long pressDuration = millis() - pressStartTime;

    if (pressDuration < 2000) {
      value++;
      Serial.println(value);
      displayValue();
    }

    buttonHeld = false;
    activeButton = 0;
  }

  // --- Appui long UNIQUEMENT sur bouton MINUS ---
  if (buttonHeld && activeButton == -1 && millis() - pressStartTime >= 2000) {
    Serial.println("🔄 Recharge (reset à 10)");
    value = 10;
    displayValue();

    buttonHeld = false;
    activeButton = 0;
  }

  // --- Fade LED ---
  if (millis() - lastFadeTime >= 30) {
    lastFadeTime = millis();
    ledBrightness += fadeAmount;

    if (ledBrightness <= 0 || ledBrightness >= 255) {
      fadeAmount = -fadeAmount;
    }
    analogWrite(LED_PIN, ledBrightness);
  }
}
