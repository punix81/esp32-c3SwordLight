#include <Adafruit_NeoPixel.h>

// Pin Definitions
#define BUTTON_PIN 21       // Pin connected to the button
#define LED_PIN 6          // Pin connected to the Neopixel data input
#define NUM_PIXELS 200      // Total number of LEDs on the Neopixel strip
#define ACTIVE_PIXELS 60   // Number of LEDs to glow

// Neopixel object
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Variables for color and button state
int currentColorIndex = 0;  // Index of the current color
bool buttonPressed = false;
bool lastButtonState = HIGH;

// Function to generate a random color
uint32_t randomColor() {
  return strip.Color(random(0, 256), random(0, 256), random(0, 256));
}

// Array of 12 predefined colors (RGB values)
uint32_t colors[] = {
  strip.Color(255, 0, 0),    // Red
  strip.Color(0, 255, 0),    // Green
  strip.Color(0, 0, 255),    // Blue
  strip.Color(255, 255, 0),  // Yellow
  strip.Color(0, 255, 255),  // Cyan
  strip.Color(255, 0, 255),  // Magenta
  strip.Color(255, 165, 0),  // Orange
  strip.Color(128, 0, 128),  // Purple
  strip.Color(75, 0, 130),   // Indigo
  0,                         // Placeholder for random colors (set later)
  strip.Color(255, 105, 180),// Pink
  strip.Color(255, 255, 255) // White
};

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Set button pin as input with internal pull-up resistor
  strip.begin();                      // Initialize the Neopixel strip
  strip.show();                       // Initialize all pixels to 'off'
  randomSeed(analogRead(0));          // Seed the random number generator
}

void loop() {
  // Read the state of the button
  bool buttonState = digitalRead(BUTTON_PIN);
  
  // Detect button press (transition from HIGH to LOW)
  if (buttonState == LOW && lastButtonState == HIGH) {
    buttonPressed = true;
  } else {
    buttonPressed = false;
  }
  
  // When the button is pressed, change the color
  if (buttonPressed) {
    currentColorIndex = (currentColorIndex + 1) % 12;  // Cycle through 12 colors
    if (currentColorIndex == 9) {
      setRandomStripColors();  // Set each LED to a random color
    } else {
      setStripColor(colors[currentColorIndex]);  // Set the new color on the strip
    }
  }
  
  // Update the last button state
  lastButtonState = buttonState;
}

// Fonction pour allumer les LEDs une par une avec une couleur donnée
void setStripColor(uint32_t color) {
  for (int i = 0; i < ACTIVE_PIXELS; i++) {
    strip.setPixelColor(i, color);
    strip.show(); // Met à jour la bande pour afficher la nouvelle LED allumée
    delay(25);    // Petite pause pour voir l'effet de progression
  }
}

// Fonction pour allumer les LEDs une par une avec des couleurs aléatoires
void setRandomStripColors() {
  for (int i = 0; i < ACTIVE_PIXELS; i++) {
    strip.setPixelColor(i, randomColor()); // Assigne une couleur aléatoire à chaque LED
    strip.show(); // Met à jour la bande pour afficher la nouvelle LED allumée
    delay(25);    // Petite pause pour voir l'effet de progression
  }
}