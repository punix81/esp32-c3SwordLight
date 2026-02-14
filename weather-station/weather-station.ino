#include <FS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>

#include <lvgl.h>
#include <TFT_eSPI.h>

#include "weather_images.h"
#include "WebConfig.h"

#include <HTTPClient.h>
#include <ArduinoJson.h>

// Les valeurs seront chargées depuis la configuration web
String ssid;
String password;
String latitude;
String longitude;
String location;
String timezone;

// Store date and time
String current_date;
String last_weather_update;
String temperature;
String humidity;
int is_day = 1;
int weather_code = 2;
String weather_description;

// SET VARIABLE TO 0 FOR TEMPERATURE IN FAHRENHEIT DEGREES
#define TEMP_CELSIUS 1

#if TEMP_CELSIUS
  String temperature_unit = "";
  const char degree_symbol[] = "\u00B0C";
#else
  String temperature_unit = "&temperature_unit=fahrenheit";
  const char degree_symbol[] = "\u00B0F";
#endif

// ======= ORIENTATION =======
// Portrait (comme RNT) : 240x320
// Pour paysage : mets 1 (et adapte plus bas si tu veux)
#define USE_LANDSCAPE 0

#if USE_LANDSCAPE
  #define SCREEN_WIDTH  320
  #define SCREEN_HEIGHT 240
#else
  #define SCREEN_WIDTH  240
  #define SCREEN_HEIGHT 320
#endif

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
static uint32_t draw_buf[DRAW_BUF_SIZE / 4];

// Backlight CYD
#define TFT_BL 21

void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

static lv_obj_t * weather_image;
static lv_obj_t * text_label_date;
static lv_obj_t * text_label_temperature;
static lv_obj_t * text_label_humidity;
static lv_obj_t * text_label_weather_description;
static lv_obj_t * text_label_time_location;

static void get_weather_data();
static void get_weather_description(int code);

static void timer_cb(lv_timer_t * timer){
  LV_UNUSED(timer);
  get_weather_data();
  get_weather_description(weather_code);

  lv_label_set_text(text_label_date, current_date.c_str());
  lv_label_set_text(text_label_temperature, String("      " + temperature + degree_symbol).c_str());
  lv_label_set_text(text_label_humidity, String("   " + humidity + "%").c_str());
  lv_label_set_text(text_label_weather_description, weather_description.c_str());
  lv_label_set_text(text_label_time_location, String("Last Update: " + last_weather_update + "  |  " + location).c_str());
}

void lv_create_main_gui(void) {
  LV_IMAGE_DECLARE(image_weather_sun);
  LV_IMAGE_DECLARE(image_weather_cloud);
  LV_IMAGE_DECLARE(image_weather_rain);
  LV_IMAGE_DECLARE(image_weather_thunder);
  LV_IMAGE_DECLARE(image_weather_snow);
  LV_IMAGE_DECLARE(image_weather_night);
  LV_IMAGE_DECLARE(image_weather_temperature);
  LV_IMAGE_DECLARE(image_weather_humidity);

  // placeholders (pour éviter écran vide)
  if (current_date.length() == 0) current_date = "----/--/--";
  if (last_weather_update.length() == 0) last_weather_update = "--:--:--";
  if (temperature.length() == 0) temperature = "--";
  if (humidity.length() == 0) humidity = "--";

  weather_image = lv_image_create(lv_screen_active());

#if USE_LANDSCAPE
  lv_obj_align(weather_image, LV_ALIGN_LEFT_MID, 15, -10);
#else
  lv_obj_align(weather_image, LV_ALIGN_CENTER, -80, -20);
#endif

  get_weather_description(weather_code);

  text_label_date = lv_label_create(lv_screen_active());
  lv_label_set_text(text_label_date, current_date.c_str());
#if USE_LANDSCAPE
  lv_obj_align(text_label_date, LV_ALIGN_TOP_RIGHT, -10, 10);
#else
  lv_obj_align(text_label_date, LV_ALIGN_CENTER, 70, -70);
#endif
  lv_obj_set_style_text_font(text_label_date, &lv_font_montserrat_26, 0);
  lv_obj_set_style_text_color(text_label_date, lv_palette_main(LV_PALETTE_TEAL), 0);

  lv_obj_t * weather_image_temperature = lv_image_create(lv_screen_active());
  lv_image_set_src(weather_image_temperature, &image_weather_temperature);

  text_label_temperature = lv_label_create(lv_screen_active());
  lv_label_set_text(text_label_temperature, String("      " + temperature + degree_symbol).c_str());
  lv_obj_set_style_text_font(text_label_temperature, &lv_font_montserrat_22, 0);

  lv_obj_t * weather_image_humidity = lv_image_create(lv_screen_active());
  lv_image_set_src(weather_image_humidity, &image_weather_humidity);

  text_label_humidity = lv_label_create(lv_screen_active());
  lv_label_set_text(text_label_humidity, String("   " + humidity + "%").c_str());
  lv_obj_set_style_text_font(text_label_humidity, &lv_font_montserrat_22, 0);

#if USE_LANDSCAPE
  lv_obj_align(weather_image_temperature, LV_ALIGN_LEFT_MID, 170, -35);
  lv_obj_align(text_label_temperature,    LV_ALIGN_LEFT_MID, 205, -35);
  lv_obj_align(weather_image_humidity,    LV_ALIGN_LEFT_MID, 170, 5);
  lv_obj_align(text_label_humidity,       LV_ALIGN_LEFT_MID, 205, 5);
#else
  lv_obj_align(weather_image_temperature, LV_ALIGN_CENTER, 30, -25);
  lv_obj_align(text_label_temperature,    LV_ALIGN_CENTER, 70, -25);
  lv_obj_align(weather_image_humidity,    LV_ALIGN_CENTER, 30, 20);
  lv_obj_align(text_label_humidity,       LV_ALIGN_CENTER, 70, 20);
#endif

  text_label_weather_description = lv_label_create(lv_screen_active());
  lv_label_set_text(text_label_weather_description, weather_description.c_str());
#if USE_LANDSCAPE
  lv_obj_align(text_label_weather_description, LV_ALIGN_BOTTOM_LEFT, 15, -35);
#else
  lv_obj_align(text_label_weather_description, LV_ALIGN_BOTTOM_MID, 0, -40);
#endif
  lv_obj_set_style_text_font(text_label_weather_description, &lv_font_montserrat_18, 0);

  text_label_time_location = lv_label_create(lv_screen_active());
  lv_label_set_text(text_label_time_location, String("Last Update: " + last_weather_update + "  |  " + location).c_str());
#if USE_LANDSCAPE
  lv_obj_align(text_label_time_location, LV_ALIGN_BOTTOM_LEFT, 15, -10);
#else
  lv_obj_align(text_label_time_location, LV_ALIGN_BOTTOM_MID, 0, -10);
#endif
  lv_obj_set_style_text_font(text_label_time_location, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(text_label_time_location, lv_palette_main(LV_PALETTE_GREY), 0);

  // refresh toutes les 10 min
  lv_timer_t * timer = lv_timer_create(timer_cb, 600000, NULL);
  lv_timer_ready(timer);
}

void get_weather_description(int code) {
  // Sécurité si appelé avant création de l'image
  if (!weather_image) return;

  switch (code) {
    case 0:
      if(is_day==1) lv_image_set_src(weather_image, &image_weather_sun);
      else          lv_image_set_src(weather_image, &image_weather_night);
      weather_description = "CLEAR SKY";
      break;
    case 1:
      if(is_day==1) lv_image_set_src(weather_image, &image_weather_sun);
      else          lv_image_set_src(weather_image, &image_weather_night);
      weather_description = "MAINLY CLEAR";
      break;
    case 2:
      lv_image_set_src(weather_image, &image_weather_cloud);
      weather_description = "PARTLY CLOUDY";
      break;
    case 3:
      lv_image_set_src(weather_image, &image_weather_cloud);
      weather_description = "OVERCAST";
      break;
    default:
      lv_image_set_src(weather_image, &image_weather_cloud);
      weather_description = "WEATHER";
      break;
  }
}

void get_weather_data() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected (offline UI)");
    return;
  }

  HTTPClient http;
  String tzEncoded = timezone;
  tzEncoded.replace("/", "%2F");

  String url = String("http://api.open-meteo.com/v1/forecast?latitude=" + latitude +
                      "&longitude=" + longitude +
                      "&current=temperature_2m,relative_humidity_2m,is_day,precipitation,rain,weather_code" +
                      temperature_unit +
                      "&timezone=" + tzEncoded +
                      "&forecast_days=1");

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      const char* datetime = doc["current"]["time"];
      temperature = String(doc["current"]["temperature_2m"]);
      humidity    = String(doc["current"]["relative_humidity_2m"]);
      is_day      = String(doc["current"]["is_day"]).toInt();
      weather_code= String(doc["current"]["weather_code"]).toInt();

      String datetime_str = String(datetime);
      int splitIndex = datetime_str.indexOf('T');
      current_date = datetime_str.substring(0, splitIndex);
      last_weather_update = datetime_str.substring(splitIndex + 1, splitIndex + 9);
      Serial.println("Weather updated OK");
    } else {
      Serial.print("deserializeJson failed: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.printf("HTTP error: %d\n", httpCode);
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(200);

  // Initialiser SPIFFS pour sauvegarder les configurations
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  Serial.println("SPIFFS initialized");

  // Charger les configurations sauvegardées
  loadConfig();
  ssid = config.ssid;
  password = config.password;
  latitude = config.latitude;
  longitude = config.longitude;
  location = config.location;
  timezone = config.timezone;

  // Initialiser le serveur web et le point d'accès
  initWebServer();

  // Force backlight ON (sinon écran noir)
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // LVGL
  lv_init();
  lv_log_register_print_cb(log_print);

  lv_display_t * disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));

#if USE_LANDSCAPE
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_90);
#else
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
#endif

  // Affiche tout de suite quelque chose
  lv_obj_t *boot = lv_label_create(lv_screen_active());
  lv_label_set_text(boot, "Initializing...");
  lv_obj_center(boot);
  lv_timer_handler();

  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect(true, false);
  delay(200);

  // Debug: scan pour voir les réseaux disponibles
  Serial.println("Scanning WiFi networks...");
  int n = WiFi.scanNetworks();
  Serial.printf("Found %d networks\n", n);
  for (int i = 0; i < n; i++) {
    Serial.printf("  %2d) %s  RSSI:%d  CH:%d  %s\n",
                  i + 1,
                  WiFi.SSID(i).c_str(),
                  WiFi.RSSI(i),
                  WiFi.channel(i),
                  (WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "OPEN" : "SEC"));
  }
  WiFi.scanDelete();

  // Connexion au WiFi STA
  Serial.printf("Connecting to SSID: %s\n", ssid.c_str());
  WiFi.begin(ssid.c_str(), password.c_str());

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    lv_timer_handler();
    delay(200);
    Serial.print(".");
  }

  Serial.println();
  Serial.printf("WiFi.status=%d\n", (int)WiFi.status());

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi OK - IP: ");
    Serial.println(WiFi.localIP());

    lv_obj_t *bootMsg = lv_label_create(lv_screen_active());
    lv_label_set_text(bootMsg, "WiFi Connected");
    lv_obj_center(bootMsg);
    lv_timer_handler();
    delay(2000);
  } else {
    Serial.println("WiFi FAIL -> Using AP mode");
    Serial.print("Connect to AP: ");
    Serial.println(AP_SSID);
    Serial.print("IP: ");
    Serial.println(AP_IP);
  }

  // Nettoie l'écran et construit l'UI
  lv_obj_clean(lv_screen_active());
  get_weather_data();
  lv_create_main_gui();
}

void loop() {
  // Gérer les requêtes HTTP du serveur web
  handleWebRequests();

  // Gestion LVGL
  lv_timer_handler();
  lv_tick_inc(5);
  delay(5);
}
