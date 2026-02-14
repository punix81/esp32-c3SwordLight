# Configuration Web pour Station Météo ESP32

## Installation et Configuration

### Étapes d'utilisation :

#### 1. **Première démarrage - Mode Point d'Accès (AP)**

Quand l'ESP32 ne peut pas se connecter au WiFi (ou au premier démarrage), il crée un point d'accès WiFi :

- **SSID** : `WeatherStation`
- **Mot de passe** : `123456789`
- **Adresse IP** : `192.168.4.1`

#### 2. **Accéder à l'interface de configuration**

1. Connectez-vous au WiFi `WeatherStation` depuis votre téléphone, tablette ou ordinateur
2. Ouvrez un navigateur web et allez à l'adresse : **http://192.168.4.1**

#### 3. **Configuration WiFi**

- Cliquez sur le bouton **"🔄 Scanner les réseaux"** pour voir les WiFi disponibles
- Sélectionnez votre réseau WiFi
- Entrez votre mot de passe WiFi
- Les paramètres de confiance sont sauvegardés automatiquement

#### 4. **Configuration de la Météo**

Remplissez les champs suivants :

- **Ville/Localité** : Le nom de votre ville (ex: Fribourg)
- **Fuseau horaire** : Sélectionnez votre fuseau horaire dans la liste
- **Latitude** : Vous pouvez trouver ces coordonnées sur [Google Maps](https://www.google.com/maps)
- **Longitude** : Même source que la latitude

**Exemple pour Fribourg :**
```
Latitude: 46.8065
Longitude: 7.1619
Fuseau horaire: Europe/Zurich
```

#### 5. **Enregistrer la configuration**

Cliquez sur le bouton **"✓ Enregistrer"** pour sauvegarder tous les paramètres.

#### 6. **Redémarrer l'appareil**

Cliquez sur le bouton **"🔄 Redémarrer"** pour redémarrer l'ESP32 avec la nouvelle configuration.

### Après la configuration

- L'ESP32 se connectera automatiquement au WiFi que vous avez configuré
- L'interface météo s'affichera sur l'écran LVGL
- L'appareil lancera un scan WiFi à chaque démarrage
- Vous pouvez toujours accéder à l'interface de configuration en vous connectant au point d'accès `WeatherStation`

## Fichiers

- **weather-station.ino** : Fichier principal Arduino
- **WebConfig.h** : Fichier header avec toute la logique web (HTML, API REST, configuration)

## API REST (pour développeurs)

### Endpoints disponibles :

#### GET /api/config
Récupère la configuration actuelle
```
Response: {"ssid":"...", "password":"...", "location":"...", "latitude":"...", "longitude":"...", "timezone":"..."}
```

#### POST /api/config
Enregistre une nouvelle configuration
```
Body: {"ssid":"...", "password":"...", "location":"...", "latitude":"...", "longitude":"...", "timezone":"..."}
Response: {"success":true, "message":"Configuration saved"}
```

#### GET /api/scan
Scanne les réseaux WiFi disponibles
```
Response: {"networks":[{"ssid":"...", "rssi":-50}, ...]}
```

#### POST /api/reboot
Redémarre l'appareil
```
Response: {"success":true}
```

## Dépannage

### L'interface ne charge pas
- Vérifiez que vous êtes bien connecté au WiFi `WeatherStation`
- Vérifiez l'adresse IP : http://192.168.4.1
- Essayez de vider le cache du navigateur (Ctrl+Shift+Delete)

### Le WiFi ne se connecte pas
- Vérifiez le SSID et le mot de passe
- Le WiFi doit être en 2.4 GHz (pas de WiFi 5 GHz)
- Pas de support WPA3-only
- Attendez 20 secondes maximum

### Les coordonnées sont incorrectes
- Allez sur [Google Maps](https://www.google.com/maps)
- Cliquez sur votre localisation (elle affichera les coordonnées)
- Copiez la latitude et la longitude

## Stockage des données

Les données de configuration sont sauvegardées dans le système de fichiers SPIFFS :
- Fichier : `/config.json`
- Format : JSON
- Chargement automatique au démarrage

## Sécurité

⚠️ **Note** : Cette interface n'est destinée qu'à la configuration locale. Le mot de passe du point d'accès est par défaut `123456789` et peut être changé dans le code.

