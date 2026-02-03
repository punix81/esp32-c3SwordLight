#ifndef CARD_WIFI_H
#define CARD_WIFI_H

#include "CardBase.h"

class CardWifi : public CardBase {
public:
  CardWifi(Adafruit_SSD1306 &d) : CardBase(d) {}
  void setInfo(const String &ssid, const String &ip, int rssi) { this->ssid = ssid; this->ip = ip; this->rssi = rssi; }
  void draw() override {
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(WHITE);
    display->setCursor(2, 4);
    display->println("WIFI");
    display->drawLine(0, 15, 127, 15, WHITE);

    display->setCursor(2, 22);
    display->println("SSID: " + ssid);
    display->setCursor(2, 36);
    display->println("IP: " + ip);
    display->setCursor(2, 50);
    display->println("RSSI: " + String(rssi) + " dBm");

    display->display();
  }
private:
  String ssid = "-";
  String ip = "-";
  int rssi = 0;
};

#endif
