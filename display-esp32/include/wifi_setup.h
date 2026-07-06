#ifndef __WIFI_SETUP_H__
#define __WIFI_SETUP_H__

// Connects to Wifi using stored credentials, falling back to the
// "TapDisplay_AP" captive portal (WiFiManager) if none are set/reachable.
// Kept in its own translation unit: WiFiManager pulls in the classic
// (synchronous) WebServer/http_parser headers, whose unscoped HTTP_GET /
// HTTP_POST enum values collide with ESPAsyncWebServer's own if both ever
// get included in the same .cpp file.
extern void connectWifi();

#endif
