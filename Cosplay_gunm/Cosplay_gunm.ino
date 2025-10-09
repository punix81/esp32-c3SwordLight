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

// 🔹 Objet écran
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// 🔹 Objet bouton
ezButton button(BUTTON_PIN);

// 🔹 Variables
int value = 10;               // valeur de départ
int ledBrightness = 0;        // luminosité LED
int fadeAmount = 5;           // vitesse du fade
unsigned long lastFadeTime = 0;

void setup() {
  Serial.begin(115200);

  // 🔹 Initialise le bus I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // 🔹 Initialise l’écran OLED
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

  // 🔹 Configure le bouton
  button.setDebounceTime(50);
  button.setCountMode(COUNT_FALLING);

  // 🔹 Configure la LED
  pinMode(LED_PIN, OUTPUT);

  Serial.println("✅ Setup done !");
}

void loop() {
  // --- Gestion du bouton ---
  button.loop();

  if (button.isPressed()) {
    if (value > 0) {
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
    delay(200); // petite pause pour éviter les doubles pressions
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
