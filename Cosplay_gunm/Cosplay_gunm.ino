#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ezButton.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// I2C ESP32-C3
#define SDA_PIN 20
#define SCL_PIN 21

// Boutons
#define BUTTON_MINUS_PIN  6
#define BUTTON_ENABLE_PIN 7   // bouton moteur

// LED
#define LED_PIN 5

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
ezButton buttonMinus(BUTTON_MINUS_PIN);
ezButton buttonEnable(BUTTON_ENABLE_PIN);

int value = 12;

// Fade LED
int ledBrightness = 0;
int fadeAmount = 5;
unsigned long lastFadeTime = 0;

// Appui long pour recharge (sur bouton - uniquement quand enable maintenu)
unsigned long pressStartTime = 0;
bool minusPressed = false;

// Etat affichage
bool lastUnlockedState = false;
int lastDisplayedValue = -999;

void displayLocked() {
  oled.clearDisplay();
  oled.setTextColor(WHITE);
  oled.setTextSize(2);
  oled.setCursor(20, 25);
  oled.println(F("LOCKED"));
  oled.display();
}

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

  // Boutons
  pinMode(BUTTON_MINUS_PIN, INPUT_PULLUP);
  pinMode(BUTTON_ENABLE_PIN, INPUT_PULLUP);

  buttonMinus.setDebounceTime(50);
  buttonMinus.setCountMode(COUNT_FALLING);

  buttonEnable.setDebounceTime(50);
  buttonEnable.setCountMode(COUNT_FALLING);

  pinMode(LED_PIN, OUTPUT);

  displayLocked();
}

void loop() {
  buttonMinus.loop();
  buttonEnable.loop();

  // ✅ INVERSION ICI : maintenant ça "unlock" quand le signal est HIGH
  const bool unlocked = (buttonEnable.getState() == HIGH);

  // Mise à jour écran seulement si nécessaire
  if (unlocked != lastUnlockedState) {
    if (unlocked) {
      displayValue();
    } else {
      displayLocked();
    }
    lastUnlockedState = unlocked;
    lastDisplayedValue = value;
  } else if (unlocked && value != lastDisplayedValue) {
    displayValue();
    lastDisplayedValue = value;
  }

  // Si pas unlock, on ignore le bouton -
  if (!unlocked) {
    minusPressed = false;
  } else {
    // --- Gestion bouton - ---
    if (buttonMinus.isPressed()) {
      pressStartTime = millis();
      minusPressed = true;
    }

    if (buttonMinus.isReleased()) {
      unsigned long pressDuration = millis() - pressStartTime;

      // Appui court : décrément
      if (pressDuration < 2000 && value > 0) {
        value--;
        Serial.print("Valeur: ");
        Serial.println(value);
      }
      minusPressed = false;
    }

    // Appui long (>=2s) : recharge/reset à 10
    if (minusPressed && (millis() - pressStartTime >= 2000)) {
      Serial.println("🔄 Recharge (reset à 10)");
      value = 10;
      minusPressed = false;
    }
  }

  // Fade LED
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
