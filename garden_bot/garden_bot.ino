#include "DHT.h"
#include <Adafruit_SSD1306.h>
#include <FluxGarage_RoboEyes.h>

// === Brochage ESP32-C3 ===
#define SDA_PIN 20
#define SCL_PIN 21
#define DHTPIN 5
#define DHTTYPE DHT11

// === Écran OLED ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// === DHT + RoboEyes ===
DHT dht(DHTPIN, DHTTYPE);
RoboEyes<Adafruit_SSD1306> roboEyes(display);

// === Timers ===
unsigned long phaseStartTime = 0;
bool showAnimation = true; // alterne entre animation et affichage des valeurs

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  dht.begin();
  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 60); // 60 FPS max
  roboEyes.setPosition(DEFAULT);
  roboEyes.setAutoblinker(ON, 3, 2);

  display.clearDisplay();
  display.display();

  phaseStartTime = millis();
}

void loop() {
  unsigned long now = millis();

  if (showAnimation) {
    // === PHASE 1 : ANIMATION ROBOEYES (10 sec) ===
    roboEyes.update();

    if (now - phaseStartTime >= 10000) { // après 10 secondes
      showAnimation = false;
      phaseStartTime = now;

      // Préparer l’écran pour afficher la température
      display.clearDisplay();
      display.display();
    }

  } else {
    // === PHASE 2 : TEMPÉRATURE + HUMIDITÉ (5 sec) ===
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      display.clearDisplay();
      display.setTextColor(SSD1306_WHITE);
      display.setTextSize(1);
      display.setCursor(0, 5);
      display.print("Temperature:");

      display.setTextSize(2);
      display.setCursor(0, 22);
      display.print(t, 1);
      display.print(" C");

      display.setTextSize(1);
      display.setCursor(0, 48);
      display.print("Humidite: ");
      display.print(h, 1);
      display.print(" %");
      display.display();
    } else {
      Serial.println("Erreur lecture DHT");
    }

    // Après 5 secondes → retour à l’animation
    if (now - phaseStartTime >= 5000) {
      showAnimation = true;
      phaseStartTime = now;
      roboEyes.setPosition(DEFAULT);
      roboEyes.setMood(DEFAULT);
    }
  }
}
