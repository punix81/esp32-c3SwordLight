#ifndef WEBCONFIG_H
#define WEBCONFIG_H

#include <FS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

using namespace fs;

// Configuration par défaut
struct ConfigData {
  char ssid[32];
  char password[64];
  char location[32];
  char latitude[16];
  char longitude[16];
  char timezone[32];
};

ConfigData config = {
  "Punix81",
  "Cyb:admin04jo4yJ",
  "Fribourg",
  "46.8065",
  "7.1619",
  "Europe/Zurich"
};

// Port pour le serveur web
WebServer webServer(80);

// Réseau AP
const char* AP_SSID = "WeatherStation";
const char* AP_PASSWORD = "123456789";
const IPAddress AP_IP(192, 168, 4, 1);
const IPAddress AP_Gateway(192, 168, 4, 1);
const IPAddress AP_Subnet(255, 255, 255, 0);

// Flag pour indiquer une nouvelle configuration
bool configUpdated = false;
String wifiStatus = "";
int wifiScannedNetworks = 0;
String wifiNetworks[20];
int wifiSignals[20];

// Sauvegarde configuration en SPIFFS
void saveConfig() {
  File file = SPIFFS.open("/config.json", "w");
  if (!file) {
    Serial.println("Failed to open config file for writing");
    return;
  }

  JsonDocument doc;
  doc["ssid"] = config.ssid;
  doc["password"] = config.password;
  doc["location"] = config.location;
  doc["latitude"] = config.latitude;
  doc["longitude"] = config.longitude;
  doc["timezone"] = config.timezone;

  if (serializeJson(doc, file) == 0) {
    Serial.println("Failed to write to config file");
  } else {
    Serial.println("Config saved successfully");
  }
  file.close();
}

// Charge configuration depuis SPIFFS
void loadConfig() {
  if (!SPIFFS.exists("/config.json")) {
    Serial.println("Config file not found, using defaults");
    return;
  }

  File file = SPIFFS.open("/config.json", "r");
  if (!file) {
    Serial.println("Failed to open config file for reading");
    return;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.print("Failed to parse config: ");
    Serial.println(error.c_str());
    return;
  }

  if (doc.containsKey("ssid")) strlcpy(config.ssid, doc["ssid"], sizeof(config.ssid));
  if (doc.containsKey("password")) strlcpy(config.password, doc["password"], sizeof(config.password));
  if (doc.containsKey("location")) strlcpy(config.location, doc["location"], sizeof(config.location));
  if (doc.containsKey("latitude")) strlcpy(config.latitude, doc["latitude"], sizeof(config.latitude));
  if (doc.containsKey("longitude")) strlcpy(config.longitude, doc["longitude"], sizeof(config.longitude));
  if (doc.containsKey("timezone")) strlcpy(config.timezone, doc["timezone"], sizeof(config.timezone));

  Serial.println("Config loaded successfully");
}

// Scan les réseaux WiFi disponibles
void scanWiFiNetworks() {
  Serial.println("Scanning WiFi networks...");
  WiFi.mode(WIFI_AP_STA);  // AP + STA mode

  int numNetworks = WiFi.scanNetworks();
  wifiScannedNetworks = min(numNetworks, 20);

  for (int i = 0; i < wifiScannedNetworks; i++) {
    wifiNetworks[i] = WiFi.SSID(i);
    wifiSignals[i] = WiFi.RSSI(i);
  }
  WiFi.scanDelete();
}

// HTML de la page de configuration
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Configuration Station Météo</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }

        .container {
            background: white;
            border-radius: 15px;
            box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
            max-width: 500px;
            width: 100%;
            padding: 40px;
        }

        h1 {
            color: #333;
            margin-bottom: 10px;
            text-align: center;
            font-size: 28px;
        }

        .subtitle {
            text-align: center;
            color: #666;
            margin-bottom: 30px;
            font-size: 14px;
        }

        .section {
            margin-bottom: 30px;
        }

        .section-title {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 12px;
            border-radius: 8px;
            margin-bottom: 15px;
            font-weight: 600;
        }

        .form-group {
            margin-bottom: 15px;
        }

        label {
            display: block;
            margin-bottom: 6px;
            color: #333;
            font-weight: 500;
            font-size: 14px;
        }

        input[type="text"],
        input[type="password"],
        select {
            width: 100%;
            padding: 10px;
            border: 2px solid #e0e0e0;
            border-radius: 6px;
            font-size: 14px;
            transition: border-color 0.3s;
        }

        input[type="text"]:focus,
        input[type="password"]:focus,
        select:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }

        .wifi-list {
            background: #f5f5f5;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            max-height: 200px;
            overflow-y: auto;
            margin-bottom: 10px;
        }

        .wifi-item {
            padding: 10px;
            border-bottom: 1px solid #e0e0e0;
            cursor: pointer;
            display: flex;
            justify-content: space-between;
            align-items: center;
            transition: background 0.2s;
        }

        .wifi-item:hover {
            background: #efefef;
        }

        .wifi-name {
            font-weight: 500;
            color: #333;
            flex: 1;
        }

        .wifi-signal {
            color: #999;
            font-size: 12px;
            margin-left: 10px;
        }

        .btn-group {
            display: flex;
            gap: 10px;
            margin-top: 20px;
        }

        button {
            flex: 1;
            padding: 12px;
            border: none;
            border-radius: 6px;
            font-weight: 600;
            cursor: pointer;
            font-size: 14px;
            transition: all 0.3s;
        }

        .btn-primary {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }

        .btn-primary:hover {
            transform: translateY(-2px);
            box-shadow: 0 10px 20px rgba(102, 126, 234, 0.3);
        }

        .btn-primary:active {
            transform: translateY(0);
        }

        .btn-secondary {
            background: #e0e0e0;
            color: #333;
        }

        .btn-secondary:hover {
            background: #d0d0d0;
        }

        .status {
            background: #f0f4ff;
            border: 2px solid #667eea;
            border-radius: 6px;
            padding: 12px;
            margin-bottom: 20px;
            font-size: 13px;
            color: #333;
        }

        .status.success {
            background: #f0fdf4;
            border-color: #22c55e;
            color: #166534;
        }

        .status.error {
            background: #fef2f2;
            border-color: #ef4444;
            color: #991b1b;
        }

        .loading {
            display: none;
            text-align: center;
            padding: 20px;
        }

        .spinner {
            border: 4px solid #f0f0f0;
            border-top: 4px solid #667eea;
            border-radius: 50%;
            width: 30px;
            height: 30px;
            animation: spin 1s linear infinite;
            margin: 0 auto;
        }

        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }

        .info-box {
            background: #f9fafb;
            border-left: 4px solid #667eea;
            padding: 12px;
            border-radius: 4px;
            font-size: 12px;
            color: #666;
            margin-top: 10px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>⚙️ Configuration</h1>
        <p class="subtitle">Station Météo ESP32</p>

        <div id="status" class="status" style="display: none;"></div>

        <!-- Section WiFi -->
        <div class="section">
            <div class="section-title">📡 Configuration WiFi</div>

            <div class="form-group">
                <label>Réseaux disponibles:</label>
                <button class="btn-secondary" onclick="scanNetworks()" style="width: 100%; margin-bottom: 10px;">🔄 Scanner les réseaux</button>
                <div id="wifiList" class="wifi-list">
                    <div style="padding: 20px; text-align: center; color: #999;">Cliquez sur "Scanner" pour voir les réseaux</div>
                </div>
            </div>

            <div class="form-group">
                <label for="ssid">SSID (Nom du réseau):</label>
                <input type="text" id="ssid" placeholder="Entrez le nom du WiFi">
            </div>

            <div class="form-group">
                <label for="password">Mot de passe WiFi:</label>
                <input type="password" id="password" placeholder="Entrez le mot de passe">
            </div>

            <div class="info-box">
                💡 Les paramètres WiFi seront sauvegardés après la configuration météo.
            </div>
        </div>

        <!-- Section Météo -->
        <div class="section">
            <div class="section-title">🌍 Configuration Météo</div>

            <div class="form-group">
                <label for="location">Ville/Localité:</label>
                <input type="text" id="location" placeholder="Ex: Fribourg">
            </div>

            <div class="form-group">
                <label for="timezone">Fuseau horaire:</label>
                <select id="timezone">
                    <option value="Europe/Zurich">Europe/Zurich (UTC+1)</option>
                    <option value="Europe/Paris">Europe/Paris (UTC+1)</option>
                    <option value="Europe/London">Europe/London (UTC+0)</option>
                    <option value="Europe/Berlin">Europe/Berlin (UTC+1)</option>
                    <option value="Europe/Amsterdam">Europe/Amsterdam (UTC+1)</option>
                    <option value="Europe/Brussels">Europe/Brussels (UTC+1)</option>
                    <option value="Europe/Vienna">Europe/Vienna (UTC+1)</option>
                    <option value="Europe/Prague">Europe/Prague (UTC+1)</option>
                    <option value="Europe/Rome">Europe/Rome (UTC+1)</option>
                    <option value="Europe/Madrid">Europe/Madrid (UTC+1)</option>
                    <option value="America/New_York">America/New_York (UTC-5)</option>
                    <option value="America/Chicago">America/Chicago (UTC-6)</option>
                    <option value="America/Denver">America/Denver (UTC-7)</option>
                    <option value="America/Los_Angeles">America/Los_Angeles (UTC-8)</option>
                    <option value="Asia/Tokyo">Asia/Tokyo (UTC+9)</option>
                    <option value="Asia/Shanghai">Asia/Shanghai (UTC+8)</option>
                    <option value="Asia/Hong_Kong">Asia/Hong_Kong (UTC+8)</option>
                    <option value="Australia/Sydney">Australia/Sydney (UTC+11)</option>
                </select>
            </div>

            <div class="form-group">
                <label for="latitude">Latitude:</label>
                <input type="text" id="latitude" placeholder="Ex: 46.8065">
            </div>

            <div class="form-group">
                <label for="longitude">Longitude:</label>
                <input type="text" id="longitude" placeholder="Ex: 7.1619">
            </div>

            <div class="info-box">
                💡 Vous pouvez trouver les coordonnées sur <a href="https://www.google.com/maps" target="_blank">Google Maps</a>
            </div>
        </div>

        <div class="btn-group">
            <button class="btn-primary" onclick="saveConfig()">✓ Enregistrer</button>
            <button class="btn-secondary" onclick="rebootDevice()">🔄 Redémarrer</button>
        </div>

        <div id="loading" class="loading">
            <div class="spinner"></div>
            <p style="margin-top: 15px; color: #666;">Enregistrement en cours...</p>
        </div>
    </div>

    <script>
        // Charge les paramètres sauvegardés au chargement
        window.onload = function() {
            loadConfig();
            scanNetworks();
        };

        function loadConfig() {
            fetch('/api/config')
                .then(r => r.json())
                .then(data => {
                    document.getElementById('ssid').value = data.ssid || '';
                    document.getElementById('password').value = data.password || '';
                    document.getElementById('location').value = data.location || '';
                    document.getElementById('latitude').value = data.latitude || '';
                    document.getElementById('longitude').value = data.longitude || '';
                    document.getElementById('timezone').value = data.timezone || 'Europe/Zurich';
                })
                .catch(e => console.error('Erreur:', e));
        }

        function scanNetworks() {
            showStatus('Scan en cours...', 'info');
            fetch('/api/scan')
                .then(r => r.json())
                .then(data => {
                    displayNetworks(data.networks);
                    showStatus('Scan terminé', 'success');
                })
                .catch(e => {
                    showStatus('Erreur lors du scan: ' + e, 'error');
                });
        }

        function displayNetworks(networks) {
            const list = document.getElementById('wifiList');
            if (networks.length === 0) {
                list.innerHTML = '<div style="padding: 20px; text-align: center; color: #999;">Aucun réseau trouvé</div>';
                return;
            }

            list.innerHTML = networks.map(net => `
                <div class="wifi-item" onclick="selectWiFi('${net.ssid}')">
                    <span class="wifi-name">${net.ssid}</span>
                    <span class="wifi-signal">${net.rssi} dBm</span>
                </div>
            `).join('');
        }

        function selectWiFi(ssid) {
            document.getElementById('ssid').value = ssid;
            document.getElementById('password').focus();
        }

        function saveConfig() {
            const config = {
                ssid: document.getElementById('ssid').value,
                password: document.getElementById('password').value,
                location: document.getElementById('location').value,
                latitude: document.getElementById('latitude').value,
                longitude: document.getElementById('longitude').value,
                timezone: document.getElementById('timezone').value
            };

            if (!config.ssid || !config.password) {
                showStatus('❌ Veuillez entrer SSID et mot de passe', 'error');
                return;
            }

            document.getElementById('loading').style.display = 'block';

            fetch('/api/config', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(config)
            })
            .then(r => r.json())
            .then(data => {
                document.getElementById('loading').style.display = 'none';
                if (data.success) {
                    showStatus('✓ Configuration enregistrée avec succès!', 'success');
                } else {
                    showStatus('❌ Erreur: ' + data.message, 'error');
                }
            })
            .catch(e => {
                document.getElementById('loading').style.display = 'none';
                showStatus('❌ Erreur réseau: ' + e, 'error');
            });
        }

        function rebootDevice() {
            if (confirm('Êtes-vous sûr? L\'appareil va redémarrer.')) {
                fetch('/api/reboot', {method: 'POST'})
                    .then(() => {
                        showStatus('🔄 Redémarrage en cours...', 'info');
                        setTimeout(() => {
                            window.location.reload();
                        }, 5000);
                    });
            }
        }

        function showStatus(msg, type) {
            const el = document.getElementById('status');
            el.textContent = msg;
            el.className = 'status ' + type;
            el.style.display = 'block';
        }
    </script>
</body>
</html>
)rawliteral";

// Gestionnaire HTTP - API endpoints
void handleConfigGet() {
  JsonDocument doc;
  doc["ssid"] = config.ssid;
  doc["password"] = config.password;
  doc["location"] = config.location;
  doc["latitude"] = config.latitude;
  doc["longitude"] = config.longitude;
  doc["timezone"] = config.timezone;

  String response;
  serializeJson(doc, response);
  webServer.send(200, "application/json", response);
}

void handleConfigPost() {
  if (!webServer.hasArg("plain")) {
    webServer.send(400, "application/json", "{\"success\":false,\"message\":\"No data\"}");
    return;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, webServer.arg("plain"));

  if (error) {
    webServer.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
    return;
  }

  strlcpy(config.ssid, doc["ssid"] | "", sizeof(config.ssid));
  strlcpy(config.password, doc["password"] | "", sizeof(config.password));
  strlcpy(config.location, doc["location"] | "", sizeof(config.location));
  strlcpy(config.latitude, doc["latitude"] | "", sizeof(config.latitude));
  strlcpy(config.longitude, doc["longitude"] | "", sizeof(config.longitude));
  strlcpy(config.timezone, doc["timezone"] | "", sizeof(config.timezone));

  saveConfig();
  configUpdated = true;

  JsonDocument response;
  response["success"] = true;
  response["message"] = "Configuration saved";

  String responseStr;
  serializeJson(response, responseStr);
  webServer.send(200, "application/json", responseStr);
}

void handleScanWiFi() {
  scanWiFiNetworks();

  JsonDocument doc;
  JsonArray networks = doc["networks"].to<JsonArray>();

  for (int i = 0; i < wifiScannedNetworks; i++) {
    JsonObject net = networks.add<JsonObject>();
    net["ssid"] = wifiNetworks[i];
    net["rssi"] = wifiSignals[i];
  }

  String response;
  serializeJson(doc, response);
  webServer.send(200, "application/json", response);
}

void handleReboot() {
  webServer.send(200, "application/json", "{\"success\":true}");
  delay(100);
  ESP.restart();
}

void handleRoot() {
  webServer.send(200, "text/html", htmlPage);
}

// Initialisation du serveur web
void initWebServer() {
  Serial.println("Initializing Web Server...");

  // Configuration du point d'accès
  WiFi.softAPConfig(AP_IP, AP_Gateway, AP_Subnet);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  Serial.print("AP SSID: ");
  Serial.println(AP_SSID);
  Serial.print("AP IP: ");
  Serial.println(AP_IP);

  // Routes
  webServer.on("/", handleRoot);
  webServer.on("/api/config", HTTP_GET, handleConfigGet);
  webServer.on("/api/config", HTTP_POST, handleConfigPost);
  webServer.on("/api/scan", handleScanWiFi);
  webServer.on("/api/reboot", HTTP_POST, handleReboot);

  webServer.begin();
  Serial.println("Web Server started");
}

// À appeler dans la boucle principale
void handleWebRequests() {
  webServer.handleClient();
}

#endif // WEBCONFIG_H
