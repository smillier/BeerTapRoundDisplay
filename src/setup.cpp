
#include "setup.h"
#include "FS.h"
#include <SPIFFS.h>
#include <WiFi.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#define _ESPASYNC_WIFIMGR_LOGLEVEL_    3
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <ESPAsync_WiFiManager.h> 
#include "weightmeasure.h"
#include "display.h"
AsyncWebServer server(80);


void initialize()
{
    //AsyncWebServer webServer(80);
#if !( USING_ESP32_S2 || USING_ESP32_C3 )
DNSServer dnsServer;
#endif
//const char *ssid = "";
//const char *password = "";


  Serial.println("\n");
  Serial.println("Connexion etablie!");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());
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
  
    #if ( USING_ESP32_S2 || USING_ESP32_C3 )
  ESPAsync_WiFiManager ESPAsync_wifiManager(&server, NULL, "Bottle_Filler_Autoconnect");
#else
  ESPAsync_WiFiManager ESPAsync_wifiManager(&server, &dnsServer, "TapDisplay");
#endif  
  //ESPAsync_wifiManager.resetSettings();   //reset saved settings
  ESPAsync_wifiManager.setAPStaticIPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
  ESPAsync_wifiManager.autoConnect("TapDisplay_AP");
  if (WiFi.status() == WL_CONNECTED) { Serial.print(F("Connected. Local IP: ")); Serial.println(WiFi.localIP()); }
  else { Serial.println(ESPAsync_wifiManager.getStatus(WiFi.status())); }
 
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
    
    ArduinoOTA.begin();

    AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("Serveur actif!");
  
  scale_setup();
  //scale_calibrate();
  display_init();

}


