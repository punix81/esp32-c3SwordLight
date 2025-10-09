#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ezButton.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// 🔹 Broches I2C (ESP32-C3 Mini)
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

unsigned long lastCount = 0;
int ledBrightness = 0;       // niveau actuel de luminosité
int fadeAmount = 5;          // vitesse du fade
unsigned long lastFadeTime = 0;

void setup() {
  Serial.begin(115200);

  // Initialise I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialise OLED
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

  // Configure le bouton
  button.setDebounceTime(50);
  button.setCountMode(COUNT_FALLING);

  // Configure la LED
  pinMode(LED_PIN, OUTPUT);

  Serial.println("✅ Setup done !");
}

void loop() {
  // 🔹 Gestion du bouton
  button.loop();
  unsigned long count = button.getCount();

  if (lastCount != count) {
    Serial.println(count);
    oled.clearDisplay();
    oled.setTextSize(3);
    oled.setCursor(40, 20);
    oled.println(count);
    oled.display();

    lastCount = count;
  }

  // 🔹 Effet fade LED ("breathing")
  unsigned long currentTime = millis();
  if (currentTime - lastFadeTime >= 30) {  // ajuste la vitesse ici
    lastFadeTime = currentTime;
    ledBrightness += fadeAmount;

    // inverse la direction du fade aux limites
    if (ledBrightness <= 0 || ledBrightness >= 255) {
      fadeAmount = -fadeAmount;
    }

    analogWrite(LED_PIN, ledBrightness); // crée le fade
  }
}
