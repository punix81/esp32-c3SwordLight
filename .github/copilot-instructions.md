# ESP32-C3 Sword Light Project - AI Coding Guidelines

## Project Overview
This is a decorative sword light using ESP32-C3 microcontroller with a Neopixel LED strip controlled by a button. The system displays 12 preset colors plus random color patterns on the first 28 LEDs of a 120-LED strip.

## Hardware Architecture
- **ESP32-C3**: Primary microcontroller
- **LED Strip**: 120 Neopixel LEDs (WS2812B), only first 28 are active
- **Button**: Connected to pin 21 with internal pull-up resistor
- **LED Data Pin**: Pin 6 for Neopixel communication

## Key Design Patterns

### Pin Configuration
```cpp
#define BUTTON_PIN 21       // Digital input with INPUT_PULLUP
#define LED_PIN 6          // Neopixel data line
#define NUM_PIXELS 120      // Physical strip length
#define ACTIVE_PIXELS 28   // Only illuminate first 28 LEDs
```

### Button Debouncing Pattern
The project uses edge detection (HIGH→LOW transition) rather than polling:
```cpp
if (buttonState == LOW && lastButtonState == HIGH) {
    buttonPressed = true;
}
```

### Color Management
- 12-element color array with predefined RGB values
- Index 9 is special case for random color mode
- Cycling through colors using modulo: `(currentColorIndex + 1) % 12`

### LED Control Patterns
- `setStripColor()`: Uniform color across active LEDs
- `setRandomStripColors()`: Each LED gets random color
- Always call `strip.show()` after setting pixels

## Critical Implementation Details

### ESP32-C3 Specifics
- Uses `INPUT_PULLUP` for button (no external resistor needed)
- Random seed from `analogRead(0)` for color generation
- Neopixel timing: `NEO_GRB + NEO_KHZ800`

### Memory Efficiency
- Static color array to avoid repeated Color() calls
- Only updates active portion of strip (28/120 LEDs)
- Minimal global variables for state tracking

### Arduino IDE Workflow
- Single `.ino` file structure
- Requires Adafruit_NeoPixel library
- No custom build process - standard Arduino compilation

## Code Modification Guidelines

### Adding New Colors
Modify the `colors[]` array, keeping index 9 for random mode. Update the modulo value in main loop to match array size.

### Changing LED Count
Update `ACTIVE_PIXELS` define. The strip buffer size (`NUM_PIXELS`) should match physical hardware.

### Button Behavior
Current pattern is press-to-cycle. For hold/repeat behavior, implement timing with `millis()` instead of edge detection.

## Common Pitfalls
- Don't forget `strip.show()` after pixel changes
- Button logic is inverted (LOW = pressed) due to pull-up
- Random colors use `random(0, 256)` for full 8-bit range
- Only modify first `ACTIVE_PIXELS`, not the full strip