#ifndef CARD_BASE_H
#define CARD_BASE_H

#include <Adafruit_SSD1306.h>

class CardBase {
public:
  CardBase(Adafruit_SSD1306 &d) : display(&d) {}
  virtual ~CardBase() {}
  virtual void draw() = 0;
protected:
  Adafruit_SSD1306 *display;
};

#endif
