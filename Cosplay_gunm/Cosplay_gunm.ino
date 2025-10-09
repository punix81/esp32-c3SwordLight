#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ezButton.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// 🔹 Broches I2C pour ESP32-C3
#define SDA_PIN 20
#define SCL_PIN 21

// 🔹 Broche du bouton
#define BUTTON_PIN 6

// 🔹 Broche de la LED
#define LED_PIN 5

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
ezButton button(BUTTON_PIN);

int value = 10;               // Valeur de départ
int ledBrightness = 0;        // Luminosité actuelle de la LED
int fadeAmount = 5;           // Vitesse du fade
unsigned long lastFadeTime = 0;

unsigned long pressStartTime = 0;  // Moment où on commence à appuyer
bool buttonHeld = false;           // Indique si le bouton est maintenu

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("❌ SSD1306 allocation failed"));
    while (true);
  }

  delay(500);
  oled.clearDisplay();
  oled.setTextSize(3);
  oled.setTextColor(WHITE);
  oled.setCursor(40, 20);
  oled.println(value);
  oled.display();

  button.setDebounceTime(50);
  button.setCountMode(COUNT_FALLING);

  pinMode(LED_PIN, OUTPUT);

  Serial.println("✅ Setup done !");
}

void loop() {
  button.loop();

  // --- Gestion de l'appui court ---
  if (button.isPressed()) {
    pressStartTime = millis();
    buttonHeld = true;
  }

  if (button.isReleased()) {
    unsigned long pressDuration = millis() - pressStartTime;

    // Si appui court (moins de 2 secondes)
    if (pressDuration < 2000 && value > 0) {
      value--;
      Serial.print("Valeur actuelle: ");
      Serial.println(value);

      oled.clearDisplay();
      oled.setTextSize(3);
      oled.setTextColor(WHITE);

      if (value > 0) {
        oled.setCursor(50, 20);
        oled.println(value);
      } else {
        oled.setTextSize(2);
        oled.setCursor(10, 25);
        oled.println(F("Recharge"));
      }
      oled.display();
    }
    buttonHeld = false;
  }

  // --- Gestion de l'appui long (>= 5 secondes) ---
  if (buttonHeld && (millis() - pressStartTime >= 2000)) {
    Serial.println("🔄 Reset à 10 après appui long !");
    value = 10;
    buttonHeld = false;

    oled.clearDisplay();
    oled.setTextSize(3);
    oled.setTextColor(WHITE);
    oled.setCursor(40, 20);
    oled.println(value);
    oled.display();
  }

  // --- Effet fade LED ("breathing") ---
  unsigned long currentTime = millis();
  if (currentTime - lastFadeTime >= 30) {
    lastFadeTime = currentTime;
    ledBrightness += fadeAmount;

    if (ledBrightness <= 0 || ledBrightness >= 255) {
      fadeAmount = -fadeAmount;
    }

    analogWrite(LED_PIN, ledBrightness);
  }
}
