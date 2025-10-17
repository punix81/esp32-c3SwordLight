#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include "DHT.h"

// --- Configuration OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SDA_PIN 20
#define SCL_PIN 21
#define OLED_RESET -1


Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Capteur DHT ---
#define DHTPIN 5       // Broche du DHT11
#define DHTTYPE DHT11  // ou DHT22 selon ton capteur
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);

  // Initialisation I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialisation de l’écran OLED
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("❌ Écran SSD1306 non détecté !"));
    for (;;); // Boucle infinie si erreur
  }

  // Initialisation du DHT
  dht.begin();

  // Message de démarrage
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);
  oled.println("DHT11 + OLED Ready!");
  oled.display();
  delay(2000);
}

void loop() {
  // Lecture des valeurs du DHT11
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Vérification lecture
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Erreur lecture DHT11 !");
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setCursor(0, 20);
    oled.println("Erreur capteur DHT!");
    oled.display();
    delay(2000);
    return;
  }

  // Affichage série
  Serial.print("🌡️ Temp: ");
  Serial.print(temperature);
  Serial.print(" °C | 💧 Hum: ");
  Serial.print(humidity);
  Serial.println(" %");

  // Affichage sur OLED
  oled.clearDisplay();

  oled.setTextSize(1);
  oled.setCursor(0, 7);
  oled.print("Temperature:");
  oled.setTextSize(2);
  oled.setCursor(50, 16);
  oled.print(temperature, 1);
  oled.write(247); // symbole °
  oled.print("C");

  oled.setTextSize(1);
  oled.setCursor(0, 35);
  oled.print("Humidity:");
  oled.setTextSize(2);
  oled.setCursor(50, 45);
  oled.print(humidity, 0);
  oled.print(" %");

  oled.display();

  delay(2000);
}
