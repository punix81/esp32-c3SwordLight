#ifndef CARD_UPTIME_H
#define CARD_UPTIME_H

#include "CardBase.h"

class CardUptime : public CardBase {
public:
  CardUptime(Adafruit_SSD1306 &d) : CardBase(d) {}
  void draw() override {
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(WHITE);
    display->setCursor(2, 4);
    display->println("UPTIME");
    display->drawLine(0, 15, 127, 15, WHITE);

    unsigned long s = millis() / 1000;
    unsigned long m = s / 60; s %= 60;
    unsigned long h = m / 60; m %= 60;

    display->setCursor(2, 22);
    display->setTextSize(2);
    display->println(String(h) + "h " + String(m) + "m");

    display->setCursor(2, 46);
    display->setTextSize(1);
    display->println("Wemos running");

    display->display();
  }
};

#endif
