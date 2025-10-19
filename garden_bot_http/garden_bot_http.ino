#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


// === ⚙️ configuraton pour un wemos d1 mini , il faut peux t etre installer le driver CH340 sous windows voir https://sparks.gogo.co.nz/ch340.html  ===
// === ⚙️ Configuration WiFi ===
const char* ssid = "Punix81";          // 👉 Remplace par le nom de ton WiFi
const char* password = "Cyb:admin04jo4yJ"; // 👉 Et ici le mot de passe

// === 🌐 Création du serveur sur le port 80 (HTTP standard) ===
ESP8266WebServer server(80);

// === 🔧 Page HTML Cyberpunk ===
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
    font-family: 'Orbitron', Arial, sans-serif;
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
    font-size: 2em;
    text-transform: uppercase;
    color: #ff00ff;
    text-shadow: 0 0 15px #ff00ff, 0 0 25px #00fff7;
    margin-bottom: 20px;
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

  .data {
    font-size: 1.5em;
    margin: 10px 0;
  }

  .label {
    color: #ff00ff;
    font-size: 0.9em;
    text-transform: uppercase;
  }

  .glow-bar {
    height: 10px;
    background: linear-gradient(90deg, #00fff7, #ff00ff);
    animation: glowMove 3s infinite linear;
    border-radius: 5px;
  }

  @keyframes glowMove {
    from { background-position: 0 0; }
    to { background-position: 200px 0; }
  }

  .footer {
    margin-top: 20px;
    font-size: 0.8em;
    color: #666;
  }

  .scanline {
    position: absolute;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background: repeating-linear-gradient(
      0deg,
      rgba(0, 255, 255, 0.05),
      rgba(0, 255, 255, 0.05) 1px,
      transparent 2px,
      transparent 4px
    );
    pointer-events: none;
    animation: scanMove 6s linear infinite;
  }

  @keyframes scanMove {
    from { background-position: 0 0; }
    to { background-position: 0 100%; }
  }
</style>
</head>
<body>
  <div class="scanline"></div>
  <h1>System Monitor</h1>
  <div class="panel">
    <div class="data">
      <div class="label">Température</div>
      <span id="temp">-- °C</span>
    </div>
    <div class="data">
      <div class="label">Humidité</div>
      <span id="hum">-- %</span>
    </div>
    <div class="glow-bar"></div>
  </div>
  <div class="footer">🧠 Cyberpunk Monitoring - v1.0</div>

<script>
  // Simulation de données (à remplacer plus tard par des valeurs réelles)
  setInterval(() => {
    document.getElementById("temp").textContent = (20 + Math.random() * 10).toFixed(1) + " °C";
    document.getElementById("hum").textContent = (40 + Math.random() * 30).toFixed(0) + " %";
  }, 2000);
</script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

// === 🔧 Page 404 ===
void handleNotFound() {
  String message = "Erreur 404 : page non trouvée\n";
  message += "URI demandée : ";
  message += server.uri();
  server.send(404, "text/plain", message);
}

// === 🚀 Setup ===
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("Démarrage du Wemos D1 Mini...");

  WiFi.begin(ssid, password);
  Serial.print("Connexion à ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi connecté !");
  Serial.print("🌐 Adresse IP : ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("🚀 Serveur HTTP démarré !");
}

// === ♻️ Boucle principale ===
void loop() {
  server.handleClient();
}