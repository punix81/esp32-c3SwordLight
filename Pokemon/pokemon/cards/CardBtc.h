#ifndef CARD_BTC_H
#define CARD_BTC_H

#include "CardBase.h"

class CardBtc : public CardBase {
public:
  CardBtc(Adafruit_SSD1306 &d, const String &title, const String &l1, const String &l2) : CardBase(d), title(title), line1(l1), line2(l2) {}
  void setLines(const String &l1, const String &l2) { line1 = l1; line2 = l2; }
  void setTitle(const String &t) { title = t; }
  void draw() override {
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(WHITE);
    display->setCursor(2, 4);
    display->println(title);
    display->drawLine(0, 15, 127, 15, WHITE);

    display->setCursor(2, 22);
    display->setTextSize(2);
    display->println(line1);



    display->display();
  }
private:
  String title;
  String line1;
  String line2;
};

#endif
