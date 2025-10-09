#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ezButton.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// 🔹 Broches I2C pour l’ESP32-C3 Mini (tu peux changer selon ton câblage)
#define SDA_PIN 20
#define SCL_PIN 21

// 🔹 Broche du bouton
#define BUTTON_PIN 6  // ⚠️ choisis un pin libre (GPIO2 ou 9 selon ton modèle)

// 🔹 Objet écran
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// 🔹 Objet bouton
ezButton button(BUTTON_PIN);

unsigned long lastCount = 0;

void setup() {
  Serial.begin(115200);

  // Initialise l’I2C sur les bons pins
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialise l’écran OLED (adresse souvent 0x3C)
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("❌ SSD1306 allocation failed"));
    while (true);
  }

  delay(500);
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(WHITE);
  oled.setCursor(0, 10);
  oled.println(F("Ready"));
  oled.display();

  // Configuration du bouton
  button.setDebounceTime(50);
  button.setCountMode(COUNT_FALLING);

  Serial.println("✅ Setup done !");
}

void loop() {
  button.loop();
  unsigned long count = button.getCount();

  if (lastCount != count) {
    Serial.println(count);
    oled.clearDisplay();
    oled.setTextSize(3);
    oled.setCursor(40, 20);
    oled.println(count);
    oled.display();

    lastCount = count;  // ✅ ici tu avais une erreur : “!=” au lieu de “=”
  }
}
