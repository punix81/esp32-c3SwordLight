#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <time.h>  // ✅ NTP time

// ✅ RoboEyes (FluxGarage)
#include "FluxGarage_RoboEyes.h" // local header (template version) in the sketch folder

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
#define HEADER_H 16
#define BODY_Y HEADER_H
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ✅ RoboEyes instance (template compatible avec SSD1306)
RoboEyes<Adafruit_SSD1306> roboEyes(display);

// ---------- Buttons ----------
#define BTN_PREV D5
#define BTN_NEXT D6
#define BTN_MOOD D7 // nouveau bouton pour changer l'humeur (pression bascule)

// Nombre de moods disponibles dans la librairie (DEFAULT, TIRED, ANGRY, HAPPY)
#define MOODS_COUNT 4

// ---------- WiFi ----------
const char* WIFI_SSID = "Punix81";
const char* WIFI_PASS = "Cyb:admin04jo4yJ"; // ⚠️ change-le si tu l’as partagé publiquement

// ---------- Time (Europe/Zurich) ----------
const char* TZ_EUROPE_ZURICH = "CET-1CEST,M3.5.0/2,M10.5.0/3";
const char* NTP1 = "pool.ntp.org";
const char* NTP2 = "time.cloudflare.com";
unsigned long lastTimeRedrawMs = 0;

// ---------- Fribourg (ville) ----------
static const float FR_LAT = 46.806477f;
static const float FR_LON = 7.161972f;

// ---------- Auto refresh ----------
static const unsigned long REFRESH_INTERVAL_MS = 5UL * 60UL * 1000UL; // 5 minutes
unsigned long lastRefreshMs = 0;

// ---------- Debounce + long press ----------
static const uint16_t DEBOUNCE_MS = 25;
static const uint16_t LONGPRESS_MS = 650;

struct Btn {
  uint8_t pin;
  bool stable = HIGH;
  bool lastRead = HIGH;
  unsigned long lastChange = 0;

  bool pressed = false;     // on press
  bool longPress = false;   // on release after long hold
  bool isDown = false;
  unsigned long downAt = 0;
  bool longFired = false;
};

Btn bPrev{BTN_PREV};
Btn bNext{BTN_NEXT};
Btn bMood{BTN_MOOD}; // nouveau bouton

// index d'humeur courant
uint8_t moodIndex = DEFAULT; // start with DEFAULT

void updateBtn(Btn& b) {
  b.pressed = false;
  b.longPress = false;

  bool r = digitalRead(b.pin);
  unsigned long now = millis();

  if (r != b.lastRead) {
    b.lastRead = r;
    b.lastChange = now;
  }

  if ((now - b.lastChange) > DEBOUNCE_MS && b.stable != b.lastRead) {
    b.stable = b.lastRead;

    if (b.stable == LOW) {
      b.isDown = true;
      b.downAt = now;
      b.longFired = false;
      b.pressed = true;
    } else {
      if (b.isDown && b.longFired) b.longPress = true;
      b.isDown = false;
    }
  }

  if (b.isDown && !b.longFired && (now - b.downAt) >= LONGPRESS_MS) {
    b.longFired = true;
  }
}

// ---------- Carousel cards ----------
enum CardId {
  CARD_WEATHER,
  CARD_BTC,
  CARD_WIFI,
  CARD_UPTIME,
  CARD_TIME,   // ✅ nouvelle card heure
  CARD_EYES,   // ✅ animation
  CARD_COUNT
};
int cardIndex = 0;

// ---------- Cached data ----------
String weatherLine1 = "-";
String weatherLine2 = "-";
String btcLine1 = "-";
String btcLine2 = "-";

// ---------- UI helpers (bicolore) ----------
void headerBar(const String& t) {
  display.fillRect(0, 0, SCREEN_WIDTH, HEADER_H, BLACK);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(2, 4);
  display.println(t);
  display.drawLine(0, HEADER_H - 1, SCREEN_WIDTH - 1, HEADER_H - 1, WHITE);
}

void clearBody() {
  display.fillRect(0, BODY_Y, SCREEN_WIDTH, SCREEN_HEIGHT - BODY_Y, BLACK);
}

void bodyPrint(int x, int y, const String& s, uint8_t size = 1) {
  display.setTextColor(WHITE);
  display.setTextSize(size);
  display.setCursor(x, y);
  display.print(s);
}

String ipToString(IPAddress ip) {
  return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}

// ---------- Time helpers ----------
void setupTimeNtp() {
  configTime(0, 0, NTP1, NTP2);     // start NTP
  setenv("TZ", TZ_EUROPE_ZURICH, 1);
  tzset();
}

bool timeReady() {
  time_t now = time(nullptr);
  return now > 1700000000; // date plausible (>= 2023)
}

// ---------- HTTPS GET helper ----------
bool httpsGet(const String& url, String& out) {
  if (WiFi.status() != WL_CONNECTED) return false;

  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  client->setTimeout(6000);

  HTTPClient https;
  https.setTimeout(6000);

  if (!https.begin(*client, url)) return false;
  https.addHeader("User-Agent", "ESP8266");

  int code = https.GET();
  if (code != 200) { https.end(); return false; }

  out = https.getString();
  https.end();
  return true;
}

// ---------- Mini parsing JSON robuste (Open-Meteo) ----------
String extractObjectAfter(const String& s, const String& key) {
  int p = s.indexOf("\"" + key + "\":");
  if (p < 0) return "";
  p = s.indexOf('{', p);
  if (p < 0) return "";

  int depth = 0;
  for (int i = p; i < (int)s.length(); i++) {
    if (s[i] == '{') depth++;
    else if (s[i] == '}') {
      depth--;
      if (depth == 0) return s.substring(p, i + 1);
    }
  }
  return "";
}

String extractNumberAfter(const String& s, const String& key) {
  String pat = "\"" + key + "\":";
  int p = s.indexOf(pat);
  if (p < 0) return "";
  p += pat.length();

  while (p < (int)s.length() && (s[p] == ' ' || s[p] == '"')) p++;

  int e = p;
  while (e < (int)s.length() && (isDigit(s[e]) || s[e] == '-' || s[e] == '.')) e++;
  if (e == p) return "";
  return s.substring(p, e);
}

String extractStringAfter(const String& s, const String& key) {
  String pat = "\"" + key + "\":\"";
  int p = s.indexOf(pat);
  if (p < 0) return "";
  p += pat.length();
  int e = s.indexOf("\"", p);
  if (e < 0) return "";
  return s.substring(p, e);
}

// ---------- Fetchers ----------
bool fetchWeatherFribourg() {
  String url =
    "https://api.open-meteo.com/v1/forecast?latitude=" + String(FR_LAT, 6) +
    "&longitude=" + String(FR_LON, 6) +
    "&current=temperature_2m,wind_speed_10m" +
    "&wind_speed_unit=kmh" +
    "&timezone=Europe%2FZurich";

  String body;
  if (!httpsGet(url, body)) return false;

  String cur = extractObjectAfter(body, "current");
  if (cur.length() == 0) return false;

  String temp = extractNumberAfter(cur, "temperature_2m");
  String wind = extractNumberAfter(cur, "wind_speed_10m");
  if (temp.length() == 0) return false;

  weatherLine1 = temp + " C";
  weatherLine2 = "Vent " + (wind.length() ? wind : "?") + " km/h";
  return true;
}

bool fetchBtcChf() {
  String url = "https://api.coinbase.com/v2/prices/BTC-CHF/spot";
  String body;
  if (!httpsGet(url, body)) return false;

  String amountStr = extractStringAfter(body, "amount");
  String cur = extractStringAfter(body, "currency");
  if (amountStr.length() == 0) return false;

  float amount = amountStr.toFloat();
  char buf[24];
  dtostrf(amount, 0, 2, buf); // 2 décimales

  btcLine1 = String(buf) + " " + (cur.length() ? cur : "CHF");
  btcLine2 = "Coinbase spot";
  return true;
}

// ---------- Refresh ----------
void refreshAll(bool showUi) {
  if (showUi) {
    display.clearDisplay();
    headerBar("AUTO REFRESH");
    clearBody();
    bodyPrint(2, BODY_Y + 10, "Updating...", 1);
    display.display();
  }

  bool wOk = fetchWeatherFribourg();
  bool bOk = fetchBtcChf();

  if (!wOk) { weatherLine1 = "ERR"; weatherLine2 = "meteo"; }
  if (!bOk) { btcLine1 = "ERR"; btcLine2 = "btc"; }

  lastRefreshMs = millis();
}

// ---------- RoboEyes card entry (avoid stale screen) ----------
void enterEyesCard() {
  display.clearDisplay();
  display.display(); // clear immediately

  roboEyes.open();
  roboEyes.update(); // draw one frame immediately
}

// ---------- Cards ----------
void drawCard() {
  if (cardIndex == CARD_EYES) {
    enterEyesCard();
    return; // roboEyes.update() in loop() handles animation
  }

  display.clearDisplay();

  switch (cardIndex) {
    case CARD_WEATHER: {
      headerBar("METEO FRIBOURG");
      clearBody();
      bodyPrint(2, BODY_Y + 6, weatherLine1, 2);
      bodyPrint(2, BODY_Y + 40, weatherLine2, 1);
      break;
    }
    case CARD_BTC: {
      headerBar("BTC/CHF");
      clearBody();
      bodyPrint(2, BODY_Y + 6, "BTC spot", 1);
      bodyPrint(2, BODY_Y + 24, btcLine1, 1);
      bodyPrint(2, BODY_Y + 40, btcLine2, 1);
      break;
    }
    case CARD_WIFI: {
      headerBar("WIFI");
      clearBody();
      bodyPrint(2, BODY_Y + 2, "SSID: " + WiFi.SSID(), 1);
      bodyPrint(2, BODY_Y + 14, "IP: " + ipToString(WiFi.localIP()), 1);
      bodyPrint(2, BODY_Y + 26, "RSSI: " + String(WiFi.RSSI()) + " dBm", 1);
      bodyPrint(2, BODY_Y + 35, (WiFi.status() == WL_CONNECTED ? "OK" : "OFF"), 2);
      break;
    }
    case CARD_UPTIME: {
      headerBar("UPTIME");
      clearBody();
      unsigned long s = millis() / 1000;
      unsigned long m = s / 60; s %= 60;
      unsigned long h = m / 60; m %= 60;
      bodyPrint(2, BODY_Y + 10, String(h) + "h " + String(m) + "m", 2);
      bodyPrint(2, BODY_Y + 32, "Wemos running", 1);
      break;
    }
    case CARD_TIME: {
      headerBar("HEURE");
      clearBody();

      if (!timeReady()) {
        bodyPrint(2, BODY_Y + 10, "Sync NTP...", 1);
      } else {
        time_t nowT = time(nullptr);
        struct tm* t = localtime(&nowT);

        char buf1[16];
        snprintf(buf1, sizeof(buf1), "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);

        char buf2[16];
        snprintf(buf2, sizeof(buf2), "%02d.%02d.%04d", t->tm_mday, t->tm_mon + 1, t->tm_year + 1900);

        bodyPrint(2, BODY_Y + 6, String(buf1), 2);
        bodyPrint(2, BODY_Y + 40, String(buf2), 1);
      }
      break;
    }
    default:
      break;
  }

  display.display();
}

void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  display.clearDisplay();
  headerBar("WIFI");
  clearBody();
  bodyPrint(2, BODY_Y + 10, "Connecting...", 1);
  display.display();

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(200);
  }

  display.clearDisplay();
  headerBar("WIFI");
  clearBody();
  if (WiFi.status() == WL_CONNECTED) {
    bodyPrint(2, BODY_Y + 10, "Connected", 1);
    bodyPrint(2, BODY_Y + 22, ipToString(WiFi.localIP()), 1);
  } else {
    bodyPrint(2, BODY_Y + 10, "FAILED", 2);
  }
  display.display();
  delay(800);
}

void setup() {
  Serial.begin(115200);

  pinMode(BTN_PREV, INPUT_PULLUP);
  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_MOOD, INPUT_PULLUP); // configuré

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    for(;;);
  }

  // ✅ Init RoboEyes (framerate ~60)
  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 60);
  roboEyes.setDisplayColors(0, 1);
  roboEyes.setAutoblinker(ON, 3, 2);
  roboEyes.setIdleMode(ON, 2, 2);
  roboEyes.setMood(DEFAULT);
  roboEyes.open();

  connectWifi();

  // ✅ Time sync (NTP)
  setupTimeNtp();

  refreshAll(true);
  drawCard();
}

void loop() {
  updateBtn(bPrev);
  updateBtn(bNext);
  updateBtn(bMood); // lecture du nouveau bouton

  // ✅ EYES animation (no delay)
  if (cardIndex == CARD_EYES) {
    roboEyes.update();
  }

  // ✅ Auto refresh
  unsigned long now = millis();
  if (now - lastRefreshMs >= REFRESH_INTERVAL_MS) {
    refreshAll(false);
    if (cardIndex != CARD_EYES) drawCard();
  }

  // ✅ Auto redraw time every second
  if (cardIndex == CARD_TIME) {
    if (now - lastTimeRedrawMs >= 1000) {
      lastTimeRedrawMs = now;
      drawCard();
    }
  }

  // Navigation: Prev
  if (bPrev.pressed) {
    cardIndex = (cardIndex - 1 + CARD_COUNT) % CARD_COUNT;
    drawCard();
    if (cardIndex != CARD_EYES) delay(80);
  }

  // Navigation: Next
  if (bNext.pressed) {
    cardIndex = (cardIndex + 1) % CARD_COUNT;
    drawCard();
    if (cardIndex != CARD_EYES) delay(80);
  }

  // Option: long press NEXT = manual refresh
  if (bNext.longPress) {
    refreshAll(true);
    if (cardIndex != CARD_EYES) drawCard();
    delay(120);
  }

  // Nouveau: changement d'humeur via BTN_MOOD (D7)
  if (bMood.pressed) {
    moodIndex = (moodIndex + 1) % MOODS_COUNT;
    roboEyes.setMood(moodIndex);
    roboEyes.open(); // forcer l'ouverture/rafraîchissement
    // Si on est sur la carte EYES, roboEyes.update() affichera la nouvelle humeur rapidement
    // sinon on redessine la carte courante pour afficher l'effet
    if (cardIndex != CARD_EYES) drawCard();
    delay(80);
  }

}
