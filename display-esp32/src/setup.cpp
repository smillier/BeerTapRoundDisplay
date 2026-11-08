
#include "setup.h"
#include "FS.h"
#include <SPIFFS.h>
#include <WiFi.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include "wifi_setup.h"
#include "weightmeasure.h"
#include "display.h"
AsyncWebServer server(80);
#include "config.h"
#include "Preferences.h"

//Preferences preference;

// Gate every custom endpoint below behind the shared API credential (see
// config.h). Call as the first line of every server.on() handler.
static bool checkAuth(AsyncWebServerRequest *request)
{
  if (!request->authenticate(API_USERNAME, getApiPassword().c_str()))
  {
    request->requestAuthentication();
    return false;
  }
  return true;
}

void initialize()
{
  Serial.begin(115200);
  Serial.println("\n");
  Serial.println("Init display");

  String apiPassword = getApiPassword();
  Serial.println("-----------------------------------------------------");
  Serial.println("Web UI / OTA login - username: " API_USERNAME);
  Serial.println("Web UI / OTA login - password: " + apiPassword);
  Serial.println("(change the password from Settings once logged in)");
  Serial.println("-----------------------------------------------------");

  // Must run before MDNS.begin(): mDNS needs an active network interface,
  // which doesn't exist yet on a fresh boot until WiFi actually connects
  // (or the WiFiManager captive portal is used to configure it).
  connectWifi();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Adresse IP: ");
    Serial.println(WiFi.localIP());
  }

  //mDNS Responder
  // Set up mDNS responder:
    // - first argument is the domain name, in this example
    //   the fully-qualified domain name is "esp32.local"
    // - second argument is the IP address to advertise
    //   we send our IP address on the WiFi network
    if (!MDNS.begin("BeerTapDisplay.local")) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
            delay(1000);
        }
    }
    Serial.println("mDNS responder started");

    Serial.println("TCP server started");

    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);
    //----------------------------------------------------OTA
  Serial.println("Starting OTA...");

 ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    // Same credential as the web UI/ElegantOTA - this is a separate OTA
    // channel (the pio/Arduino-IDE "upload over WiFi" protocol, port 8266)
    // that was otherwise just as unauthenticated as the web-based one.
    ArduinoOTA.setPassword(apiPassword.c_str());
    ArduinoOTA.begin();

    ElegantOTA.begin(&server, API_USERNAME, apiPassword.c_str());    // Start ElegantOTA
  server.begin();
  Serial.println("Serveur actif!");

  if(!SPIFFS.begin()){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
     display_init();
   Serial.println("Scale setup");
 scale_setup();
  //scale_calibrate();

}


void initWebServer()
{
   //----------------------------------------------------SERVER
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { if (!checkAuth(request)) return;
              request->send(SPIFFS, "/index.html", "text/html"); });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { if (!checkAuth(request)) return;
              request->send(SPIFFS, "/style.css", "text/css"); });

  server.on("/app.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { if (!checkAuth(request)) return;
              request->send(SPIFFS, "/app.js", "text/javascript"); });

  server.on("/weight", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              if (!checkAuth(request)) return;
              float sensorValue = getWeight();
              //int val = analogRead(capteurPression);
              String Value = String(sensorValue);
              request->send(200, "text/plain", Value);
              Serial.println(Value);
            });

  // Tare command relayed over BLE to the remote scale (ESP32-C6 + HX711)
  server.on("/tare", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (!checkAuth(request)) return;
              scale_calibrate();
              request->send(204);
            });

  // Reports whether the display is currently linked to the remote scale over BLE
  server.on("/scale_status", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              if (!checkAuth(request)) return;
              String json = "{\"connected\":";
              json += isScaleConnected() ? "true" : "false";
              json += ",\"stale\":";
              json += isWeightStale() ? "true" : "false";
              json += "}";
              request->send(200, "application/json", json);
            });
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              if (!checkAuth(request)) return;
              //etatLedVoulu = 1;
              request->send(204);
             // Serial.println("On");
            });

  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              if (!checkAuth(request)) return;
              //etatLedVoulu = 0;
              //digitalWrite(led, LOW);
              //etatLed = 0;
              request->send(204);
              // Serial.println("off");
            });

  server.on("/delayLed", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (!checkAuth(request)) return;
              if (request->hasParam("valeurDelayLed", true))
              {
                String message;
                message = request->getParam("valeurDelayLed", true)->value();
                //if(message.toInt()!=0)
                //valeurDelayLed = message.toInt();
                Serial.println("valeurDelayLed");
              }
              request->send(204);
            });
  server.on("/beer_name", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (!checkAuth(request)) return;
              if (request->hasParam("beer_name", true))
              {
                String message;
                message = request->getParam("beer_name", true)->value();
                setBeerName(message);
               // preference.putString("beer_name",message);
                
                 Serial.println("beer_name");
              }
              request->send(204);
            });

    server.on("/beer_ibu", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (!checkAuth(request)) return;
              if (request->hasParam("beer_ibu", true))
              {
                String message;
                message = request->getParam("beer_ibu", true)->value();
                if(message.toInt()!=0)
                setBeerIBU(message.toInt());
                Serial.println("beer_ibu");
              }
              int ibu = getBeerIBU();
              String Value = String(ibu);
                Serial.println(Value);
              request->send(200, "text/plain", Value);
              //request->send(204);
            });
   server.on("/beer_ebc", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (!checkAuth(request)) return;
              int EBC = getBeerEBC();
              if (request->hasParam("beer_ebc", true))
              {
                String message;
                message = request->getParam("beer_ebc", true)->value();
                if(message.toInt()!=0)
                  setBeerEBC(message.toInt());
                //valeurDelayLed = message.toInt();
                Serial.println("beer_ebc");
              }
              
               Serial.println((String)EBC);
              request->send(200, "text/plain", (String)EBC);
             
            });
   server.on("/beer_abv", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (!checkAuth(request)) return;
              float ABV = getBeerABV();
              if (request->hasParam("beer_abv", true))
              {
                String message;
                message = request->getParam("beer_abv", true)->value();
               if(message.toFloat()!=0.0)
                ABV = message.toFloat();
                setBeerABV(ABV);
                Serial.println("beer_abv");
              }
              request->send(200, "text/plain", (String)ABV);
            });
    server.on("/beer_sg", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (!checkAuth(request)) return;
              float sg = getBeerSg();
              if (request->hasParam("beer_sg", true))
              {
                String message;
                message = request->getParam("beer_sg", true)->value();
               if(message.toFloat()!=0.0)
                sg = message.toFloat();
                setBeerSg(sg);
                Serial.println("beer_sg");
              }
              request->send(200, "text/plain", (String)sg);
            });

  server.on("/beer_capacity", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (!checkAuth(request)) return;
              float capacity = getBeerCapacity();
              if (request->hasParam("beer_capacity", true))
              {
                String message = request->getParam("beer_capacity", true)->value();
                if (message.toFloat() != 0.0)
                  capacity = message.toFloat();
                setBeerCapacity(capacity);
              }
              request->send(200, "text/plain", (String)capacity);
            });

  // Single read of the whole beer profile, used by the web UI to populate
  // its form on load instead of a POST-with-no-params per field.
  server.on("/beer_profile", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              if (!checkAuth(request)) return;
              StaticJsonDocument<256> doc;
              doc["name"] = getBeerName();
              doc["sg"] = getBeerSg();
              doc["ibu"] = getBeerIBU();
              doc["ebc"] = getBeerEBC();
              doc["abv"] = getBeerABV();
              doc["capacity"] = getBeerCapacity();
              String json;
              serializeJson(doc, json);
              request->send(200, "application/json", json);
            });

  server.on("/restart", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (!checkAuth(request)) return;
              request->send(204);
              delay(300);
              ESP.restart();
            });

  // Clears saved Wifi credentials and reboots into the TapDisplay_AP portal
  server.on("/reset_wifi", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (!checkAuth(request)) return;
              request->send(204);
              resetWifiSettings();
              delay(300);
              ESP.restart();
            });

  // Lets the owner set a memorable password (via Settings) in place of the
  // random one generated and printed to Serial on first boot. Requires the
  // *current* password to change it.
  server.on("/api_password", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (!checkAuth(request)) return;
              if (request->hasParam("api_password", true))
              {
                String newPassword = request->getParam("api_password", true)->value();
                if (newPassword.length() >= 8)
                {
                  setApiPassword(newPassword);
                  request->send(204);
                  return;
                }
                request->send(400, "text/plain", "Password must be at least 8 characters");
                return;
              }
              request->send(400, "text/plain", "Missing api_password");
            });
           // preference.end();
}

