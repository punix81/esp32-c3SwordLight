#ifndef CARD_TIME_H
#define CARD_TIME_H

#include "CardBase.h"

class CardTime : public CardBase {
public:
  CardTime(Adafruit_SSD1306 &d) : CardBase(d) {}
  void draw() override {
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(WHITE);
    display->setCursor(2, 4);
    display->println("HEURE");
    display->drawLine(0, 15, 127, 15, WHITE);

    time_t nowT = time(nullptr);
    struct tm* t = localtime(&nowT);

    char buf1[16];
    snprintf(buf1, sizeof(buf1), "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);

    char buf2[16];
    snprintf(buf2, sizeof(buf2), "%02d.%02d.%04d", t->tm_mday, t->tm_mon + 1, t->tm_year + 1900);

    display->setCursor(2, 22);
    display->setTextSize(2);
    display->println(String(buf1));

    display->setCursor(2, 46);
    display->setTextSize(1);
    display->println(String(buf2));

    display->display();
  }
};

#endif
