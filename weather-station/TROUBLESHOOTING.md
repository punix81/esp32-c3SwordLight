# 🆘 Guide de Dépannage Rapide

## ⚡ Problèmes & Solutions

### 1. L'ESP32 Refuse de Compiler

**❌ Erreur :** `WebConfig.h: No such file or directory`

**✅ Solution :**
```
1. Vérifiez que WebConfig.h est dans le même dossier que weather-station.ino
2. Chemins corrects :
   - C:\Users\paulo\OneDrive\Documents\Arduino\esp32-c3SwordLight\weather-station\
     ├── weather-station.ino
     ├── WebConfig.h          ← ICI
     └── weather_images.h
```

---

### 2. Erreur : "SPIFFS Mount Failed"

**❌ Erreur :** `SPIFFS Mount Failed`

**✅ Causees Possibles :**
- SPIFFS pas formaté
- Problème de partition

**✅ Solution :**
```
1. Arduino IDE → Tools → Erase All Flash Contents
2. Puis → Tools → Upload
3. Attendre le formatage complet (~1 minute)
4. Redémarrer l'ESP32
```

---

### 3. Point d'Accès WiFi n'Apparaît Pas

**❌ Problème :** Pas de WiFi "WeatherStation" visible

**✅ Vérification :**
1. Allez sur le moniteur série (115200 baud)
2. Cherchez : `AP SSID: WeatherStation`
3. Vous devriez voir l'IP : `AP IP: 192.168.4.1`

**✅ Si Absent :**
```
1. Vérifiez que initWebServer() est appelé dans setup()
2. Redémarrez l'appareil (débranchez 10 sec)
3. Attendez 2-3 secondes après le démarrage
4. Scannez les réseaux WiFi à nouveau
```

---

### 4. Interface Web ne Charge Pas

**❌ Erreur :** Page blanche / ERR_CONNECTION_REFUSED

**✅ Checklist :**
```
1. ✓ Connecté au WiFi "WeatherStation" ?
   → Vérifiez dans les paramètres WiFi

2. ✓ Bonne URL ?
   → http://192.168.4.1 (pas https !)
   → Pas de port après l'IP

3. ✓ Navigateur compatible ?
   → Chrome, Firefox, Safari, Edge
   → Pas d'Internet Explorer

4. ✓ Cache du navigateur ?
   → Ctrl+Shift+Delete
   → Sélectionnez "Tous les fichiers"
   → Rafraîchissez la page
```

---

### 5. Scan WiFi Très Lent

**❌ Problème :** Scan prend plus de 10 secondes

**✅ Causes :**
- Trop de réseaux à proximité (>20)
- Interférence WiFi

**✅ Solutions :**
```
1. Attendez 15-20 secondes (c'est normal)
2. Éloignez-vous des autres appareils
3. Essayez à une autre heure
4. Si vraiment trop lent : commentez scanWiFiNetworks() 
   et testez avec SSID manuel
```

---

### 6. Configuration Enregistrée Mais Oubliée Après Redémarrage

**❌ Problème :** La config est réinitialisée

**✅ Cause :**
- Corruption du fichier SPIFFS

**✅ Solution :**
```
1. Arduino IDE → Tools → Erase All Flash Contents
2. Redéployer le code
3. Reconfigurer
```

---

### 7. WiFi ne se Connecte Pas Après Configuration

**❌ Problème :** Reste sur AP mode, n'utilise pas le WiFi configuré

**✅ Diagnostic dans les Logs Série :**
```
Cherchez des messages comme :
- "Connecting to SSID: MonWiFi"
- "WiFi.status=X"  (0=disconnected, 3=connected)
- "WiFi OK - IP: 192.168.1.100" ← Succès !
- "WiFi FAIL -> Using AP mode" ← Échec
```

**✅ Causes Possibles :**
1. **WiFi 5 GHz** → ESP32-C3 supporte SEULEMENT 2.4 GHz
2. **WPA3 seul** → Non supporté, besoin WPA2
3. **Mot de passe incorrect** → Vérifier caractère par caractère
4. **SSID caché** → Peut ne pas fonctionner

**✅ Solutions :**
```
1. Vérifier le WiFi est bien 2.4 GHz
   → Généralement nommé "MonWiFi" ou "MonWiFi-2.4G"
   → NON "MonWiFi-5G" ou "MonWiFi-6"

2. Tester le WiFi sur un autre appareil
   → Confirmez que mot de passe fonctionne

3. Augmenter timeout WiFi
   → Dans weather-station.ino, ligne ~240:
   ```cpp
   while (WiFi.status() != WL_CONNECTED && millis() - start < 20000)
   // ← Augmentez 20000 à 30000 (30 secondes)
   ```

4. Forcer reconnexion
   → Arduino IDE → Serial Monitor
   → Appuyez sur le bouton RESET
   → Attendez les logs
```

---

### 8. Écran LVGL Affiche "Initializing..." Longtemps

**❌ Problème :** L'écran reste figé sur "Initializing..."

**✅ Cause :**
- Le WiFi prend trop de temps à se connecter

**✅ Solution :**
```
1. Attendez 25 secondes (timeout par défaut)
2. Allez sur le moniteur série pour voir les logs
3. Connectez à "WeatherStation"
4. Reconfigurer le WiFi
5. Revenez à cet écran "Initializing..."
6. Redémarrez
```

---

### 9. "API /config Retourne Erreur 400"

**❌ Erreur :** `{"success": false}`

**✅ Cause :**
- Données JSON mal formatées

**✅ Vérification :**
```bash
# Test avec curl :
curl -X POST http://192.168.4.1/api/config \
  -H "Content-Type: application/json" \
  -d '{
    "ssid":"WIFI",
    "password":"PASSWORD",
    "location":"City",
    "latitude":"46.8",
    "longitude":"7.1",
    "timezone":"Europe/Zurich"
  }'

# Doit retourner :
{"success":true,"message":"Configuration saved"}
```

---

### 10. Page HTML Très Lente à Charger

**❌ Problème :** Page met 10+ secondes à afficher

**✅ Cause :**
- ESP32 charge le grand HTML en RAM

**✅ Solution :**
```
1. Attendez 15 secondes (c'est un démarrage)
2. Les chargements suivants seront plus rapides
3. Le navigateur met en cache le HTML
4. Pour optimiser : minifiez le CSS/JS dans WebConfig.h
```

---

### 11. Redémarrage ne Fonctionne Pas

**❌ Problème :** Clic "Redémarrer" = rien

**✅ Cause :**
- JavaScript n'envoie pas correctement la requête

**✅ Solution :**
```
1. F12 → Console (dans le navigateur)
2. Cherchez les erreurs en rouge
3. Ou utilisez curl directement :
   curl -X POST http://192.168.4.1/api/reboot
   → Devrait redémarrer l'ESP32
```

---

### 12. Coordonnées GPS Incorrectes / Météo Mauvaise

**❌ Problème :** La météo affichée ne correspond pas à la ville

**✅ Solution :**
```
1. Allez sur Google Maps : https://maps.google.com
2. Cherchez votre ville
3. Clic-droit sur le repère rouge
4. Les coordonnées apparaissent en bas
5. Copier exact (décimales incluses)

Exemple pour Fribourg :
- Cliquer sur le repère
- Copier : "46.8065, 7.1619"
- Diviser en :
  - Latitude: 46.8065
  - Longitude: 7.1619

⚠️ ATTENTION À L'ORDRE :
- Google Maps : latitude, longitude
- L'interface : latitude, longitude
```

---

### 13. Erreur de Compilation : "ReferenceError: config not defined"

**❌ Erreur :** À la compilation

**✅ Cause :**
- WebConfig.h n'est pas bien inclus

**✅ Solution :**
```
1. Vérifiez la ligne de include dans weather-station.ino :
   #include "WebConfig.h"  ← Guillemets (pas <>)

2. Vérifiez WebConfig.h est dans le même dossier

3. Reconstructez le projet :
   - Fermer Arduino IDE
   - Supprimer le dossier : 
     ~/Arduino/libraries/__pycache__
   - Rouvrir Arduino IDE
   - Recompiler
```

---

### 14. Navigateur Affiche Warning HTTPS

**⚠️ Attention :**
```
"Ce site n'est pas sécurisé"
"La connexion n'est pas privée"
```

**✅ C'est normal !**
```
- C'est du HTTP local (pas HTTPS)
- Pas de certificat SSL/TLS
- Aucun danger (réseau local)
- Ignorer le warning en toute sécurité
```

---

### 15. Les Paramètres WiFi ne S'Affichent pas

**❌ Problème :** Form vide après chargement

**✅ Cause :**
- Configuration non chargée

**✅ Solution :**
```
1. F12 → Console → Cherchez les erreurs
2. Testez l'API directement :
   curl http://192.168.4.1/api/config
3. Doit retourner JSON valide
4. Si erreur : vérifiez handleConfigGet() dans WebConfig.h
```

---

## 🔍 Diagnostic Complet

### Étape 1 : Vérifiez les Logs Série

```
Démarrage normal ressemble à :
─────────────────────────────────────
Booting...
SPIFFS initialized
Initializing Web Server...
AP SSID: WeatherStation
AP IP: 192.168.4.1
Web Server started
Scanning WiFi networks...
Found 8 networks
[Affiche la liste]
Connecting to SSID: Punix81
..........
WiFi OK - IP: 192.168.x.x
─────────────────────────────────────
```

### Étape 2 : Test de Connectivité

```bash
# Ping l'ESP32
ping 192.168.4.1
# Doit répondre "reply from..."

# Test HTTP
curl http://192.168.4.1
# Doit retourner du HTML

# Test API
curl http://192.168.4.1/api/config
# Doit retourner du JSON
```

### Étape 3 : Vérifiez SPIFFS

```cpp
// Ajoutez dans setup() après initWebServer():
void testSPIFFS() {
  Serial.println("SPIFFS Contents:");
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while(file){
    Serial.print(file.name());
    Serial.print(" - ");
    Serial.println(file.size());
    file = root.openNextFile();
  }
}
// Appelez : testSPIFFS();
// Doit afficher : config.json - XXX bytes
```

---

## 🔧 Réinitialisation Complète

Si rien ne fonctionne :

```
1. Arduino IDE → Tools → Erase All Flash Contents
2. Attendez le formatage (~1 minute)
3. Redéployer le code
4. Attendre 2-3 secondes
5. Redémarrer l'ESP32
6. Cherchez "WeatherStation"
7. Allez à http://192.168.4.1
8. Reconfigurer depuis zéro
```

---

## 📞 Aide Supplémentaire

**Vérifiez :**
1. ✅ Logs série (15:30 baud)
2. ✅ README_CONFIGURATION.md
3. ✅ GUIDE_COMPLET_CONFIGURATION_WEB.md
4. ✅ ARCHITECTURE_WEBCONFIG_H.md

**Solutions Avancées :**
- Utilisez Chrome DevTools (F12)
- Activez Network tab pour voir les requêtes
- Vérifiez les réponses HTTP
- Testez les APIs avec Postman ou curl

---

**Bon dépannage ! 🚀**

