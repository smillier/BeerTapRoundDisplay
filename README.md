# BeerTapRoundDisplay V2 - Balance déportée (ESP32-C6 + BLE)

Fork de [BeerTapRoundDisplay](https://github.com/smillier/BeerTapRoundDisplay) qui sépare
la balance (HX711 + capteurs de poids) de l'écran rond GC9A01. Les deux cartes
communiquent en Bluetooth Low Energy au lieu d'être câblées ensemble sur un seul ESP32.

```
┌─────────────────────────┐        BLE (GATT)        ┌──────────────────────────┐
│   scale-esp32c6          │  ────────────────────►   │   display-esp32          │
│   ESP32-C6 + HX711       │   notify poids (g)        │   ESP32 + GC9A01 rond    │
│   sous le fût            │  ◄────────────────────   │   Wifi + WebServer + OTA │
│                          │   commande tare            │   calcule le volume      │
└─────────────────────────┘                            └──────────────────────────┘
```

## Pourquoi deux cartes ?

- La balance (HX711 + jauges de contrainte) reste au plus près du fût, sous le pied,
  loin de l'écran qui lui est déporté sur la façade du meuble/frigo.
- L'ESP32-C6 est un choix naturel pour ce nœud : petit, peu cher, BLE natif, pas besoin
  de Wifi/écran/flash volumineux côté balance.
- L'écran garde le Wifi/OTA/serveur web existants (config bière, IBU, ABV, SG...) et
  affiche un point d'état (vert/orange/rouge) indiquant la fraîcheur de la liaison BLE.

## Structure du repo

```
scale-esp32c6/     Firmware de la balance (HX711 -> BLE GATT server)
display-esp32/     Firmware de l'écran (BLE GATT client -> affichage + Wifi/webserver)
```
Ce sont deux projets PlatformIO indépendants, à flasher séparément (deux `platformio.ini`).

## Protocole BLE

La balance (`scale-esp32c6`) est **serveur BLE (peripheral)** ; l'écran
(`display-esp32`) est **client BLE (central)**. Il n'y a pas de pairing/bonding
(pas de données sensibles échangées) - la connexion se fait par filtrage sur l'UUID
de service.

| Élément | UUID | Type | Contenu |
|---|---|---|---|
| Service balance | `a1f4e100-3b7a-4f56-9d1b-2f6a9c5e10a0` | - | - |
| Caractéristique poids | `a1f4e101-3b7a-4f56-9d1b-2f6a9c5e11a0` | READ + NOTIFY | `float32` poids en grammes |
| Caractéristique commande | `a1f4e102-3b7a-4f56-9d1b-2f6a9c5e12a0` | WRITE | `uint8` : `0x01` = tare |

Ces valeurs sont définies dans `scale-esp32c6/include/config.h` et
`display-esp32/include/config.h` (doivent rester identiques des deux côtés).

Comportement :
- La balance mesure le poids toutes les `WEIGHT_UPDATE_PERIOD_MS` (2 s par défaut) et
  notifie la valeur si un client est connecté.
- L'écran scanne, se connecte au premier appareil qui annonce le service ci-dessus, et
  se réabonne automatiquement en cas de déconnexion (`BLE_RECONNECT_PERIOD_MS`).
- Le volume (poids / densité SG configurée sur l'écran) est toujours calculé côté
  écran, comme avant - la balance ne fait que remonter le poids brut.
- Un appui "Tare" côté interface web de l'écran (`POST /tare`) envoie la commande à la
  balance, qui effectue `scale.tare()`.
- `GET /scale_status` renvoie `{"connected":bool,"stale":bool}` pour du monitoring externe.
- Sur l'écran, un point en haut du cadran indique l'état : vert = donnée fraîche,
  orange = connecté mais donnée ancienne (> `BLE_WEIGHT_STALE_MS` = 15 s), rouge =
  balance non connectée.

## scale-esp32c6/ - Câblage HX711

| HX711 | ESP32-C6 |
|---|---|
| DOUT | GPIO2 |
| SCK  | GPIO3 |
| VCC  | 3.3V (ou 5V selon module HX711) |
| GND  | GND |

GPIO2/3 ont été choisis car les GPIO4, 5, 8, 9, 15 sont des pins de strapping au boot
sur l'ESP32-C6 et doivent être évités. Adaptez `SCALE_CALIBRATION_FACTOR` dans
`scale-esp32c6/include/config.h` à votre propre jauge de contrainte (procédure de
calibration identique à l'originale : peser un poids connu et ajuster le facteur).

## display-esp32/ - Écran

Câblage GC9A01 inchangé par rapport au repo d'origine (voir `display-esp32/include/config.h`).
Le HX711 n'est plus câblé sur cette carte.

## Flash des deux cartes

Chaque dossier est un projet PlatformIO à part entière :

```bash
cd scale-esp32c6
pio run -t upload         # ESP32-C6 en USB

cd ../display-esp32
pio run -t upload         # ESP32 classique, en USB la première fois puis OTA
```

`display-esp32` garde l'OTA (ArduinoOTA + AsyncElegantOTA) comme dans le projet
d'origine ; `scale-esp32c6` se flashe en USB (pas d'OTA, pas de Wifi sur cette carte).

## Limitations / TODO

- Les fichiers `Case/` et `PCB/` du repo d'origine (boîtier 3D, PCB Eagle) ont été
  conçus pour la carte combinée écran+HX711 : ils devront être redessinés/scindés en
  deux boîtiers (non traité dans cette version, hors périmètre logiciel).
- `data/config.json` (contenu de démonstration du repo d'origine) contient un SSID et
  une clé Wifi factices en clair - à vérifier/nettoyer avant publication si ce n'est
  pas volontaire.
- Pas de pairing BLE (LE Secure Connections) : suffisant pour un usage domestique où
  seul le poids transite, mais à garder en tête si le protocole évolue.
