#ifndef CARD_EYES_H
#define CARD_EYES_H

#include "CardBase.h"
#include "../FluxGarage_RoboEyes.h"

class CardEyes : public CardBase {
public:
  CardEyes(Adafruit_SSD1306 &d, RoboEyes<Adafruit_SSD1306> &r) : CardBase(d), eyes(r) {}
  void draw() override {
    // On delegate l'animation à roboEyes; ici on s'assure que l'eyes est ouvert et qu'on dessine une frame
    eyes.open();
    eyes.update();
  }
private:
  RoboEyes<Adafruit_SSD1306> &eyes;
};

#endif
