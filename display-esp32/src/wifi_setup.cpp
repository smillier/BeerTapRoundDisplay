#include <Arduino.h>
#include <WiFiManager.h>
#include "wifi_setup.h"

void connectWifi()
{
  WiFiManager wifiManager;
  //wifiManager.resetSettings();   //reset saved settings
  wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  wifiManager.autoConnect("TapDisplay_AP");

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print(F("Connected. Local IP: "));
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("Wifi not connected");
  }
}

void resetWifiSettings()
{
  WiFiManager wifiManager;
  wifiManager.resetSettings();
}
