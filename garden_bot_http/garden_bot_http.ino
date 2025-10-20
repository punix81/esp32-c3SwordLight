#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// === ⚙️ configuraton pour un wemos d1 mini , il faut peux t etre installer le driver CH340 sous windows voir https://sparks.gogo.co.nz/ch340.html  ===
// === ⚙️ Configuration WiFi ===
const char* ssid = "Punix81";          // 👉 Remplace par le nom de ton WiFi
const char* password = "Cyb:admin04jo4yJ"; // 👉 Et ici le mot de passe
 
// === 🌡️ DHT11 ===
#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// === 🖥️ OLED ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// === 🌐 Serveur Web ===
ESP8266WebServer server(80);

float temperature = 0.0;
float humidity = 0.0;
bool dhtError = false;

// === 📊 Lecture du DHT11 ===
void readDHT() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    dhtError = true;
  } else {
    dhtError = false;
    temperature = t;
    humidity = h;
  }
}

// === 🖥️ Mise à jour de l’écran OLED ===
void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("🌐 Cyberpunk Monitor");

  if (dhtError) {
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("❌ Capteur DHT11");
    display.println("non detecte !");
  } else {
    display.setTextSize(2);
    display.setCursor(0, 20);
    display.print(temperature, 1);
    display.print("C");

    display.setCursor(0, 45);
    display.print(humidity, 0);
    display.print("%");
  }

  display.display();
}

// === 🔧 Page HTML ===
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width, initial-scale=1.0" />
<title>Cyberpunk Monitoring</title>
<style>
  body {
    background-color: #0a0a0f;
    color: #00fff7;
    font-family: "Orbitron", Arial, sans-serif;
    text-shadow: 0 0 8px #00fff7;
    margin: 0;
    padding: 0;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    height: 100vh;
  }
  h1 {
    color: #ff00ff;
    text-shadow: 0 0 15px #ff00ff, 0 0 25px #00fff7;
  }
  .panel {
    border: 2px solid #00fff7;
    border-radius: 10px;
    box-shadow: 0 0 15px #00fff7;
    padding: 20px 30px;
    background: rgba(0, 255, 247, 0.05);
    width: 300px;
    text-align: center;
  }
  .error {
    color: red;
    text-shadow: 0 0 10px red;
  }
</style>
</head>
<body>
  <h1>System Monitor</h1>
  <div class="panel">
)rawliteral";

  if (dhtError) {
    html += "<div class='error'>❌ Erreur : capteur DHT11 non détecté !</div>";
  } else {
    html += "<div>🌡️ Température : " + String(temperature, 1) + " °C</div>";
    html += "<div>💧 Humidité : " + String(humidity, 0) + " %</div>";
  }

  html += R"rawliteral(
  </div>
  <div style="margin-top:20px;font-size:0.8em;color:#666;">🧠 Cyberpunk Monitoring - v2.0</div>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

// === 🚀 Setup ===
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Démarrage...");

  // DHT
  dht.begin();

  // OLED
  Wire.begin(D2, D1); // SDA, SCL
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("❌ Écran OLED introuvable !");
    for (;;);
  }
  display.clearDisplay();
  display.display();

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connexion à ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connecté !");
  Serial.print("🌐 IP : ");
  Serial.println(WiFi.localIP());

  // Routes serveur
  server.on("/", handleRoot);
  server.begin();
  Serial.println("🚀 Serveur HTTP démarré !");
}

// === ♻️ Boucle principale ===
void loop() {
  readDHT();
  updateOLED();
  server.handleClient();
  delay(2000);
}