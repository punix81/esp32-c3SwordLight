#ifndef CARD_WEATHER_H
#define CARD_WEATHER_H

#include "CardBase.h"
#include <String.h>

class CardWeather : public CardBase {
public:
  CardWeather(Adafruit_SSD1306 &d, const String &l1, const String &l2) : CardBase(d), line1(l1), line2(l2) {}
  void setLines(const String &l1, const String &l2) { line1 = l1; line2 = l2; }
  void draw() override {
    display->clearDisplay();
    // headerBar("METEO FRIBOURG");
    display->setTextSize(1);
    display->setTextColor(WHITE);
    display->setCursor(2, 4);
    display->println("METEO FRIBOURG");
    display->drawLine(0, 15, 127, 15, WHITE);

    display->setCursor(2, 22);
    display->setTextSize(2);
    display->println(line1);

    display->setCursor(2, 52-12);
    display->setTextSize(1);
    display->println(line2);

    display->display();
  }
private:
  String line1;
  String line2;
};

#endif
