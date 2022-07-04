// **********************************************************************************
// TapDisplay WEB Server, route web function
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend this library but please abide with the CC-BY-SA license:
// Attribution-NonCommercial-ShareAlike 4.0 International License
// http://creativecommons.org/licenses/by-nc-sa/4.0/
//
// This program works with the TapDisplay board
//
//
//
// All text above must be included in any redistribution.
//
// **********************************************************************************

#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

// Include main project include file
#include "config.h"
#include "setup.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
// Exported variables/object instanciated in main sketch
// =====================================================
extern char response[];
extern uint16_t response_idx;

// declared exported function from webserver.cpp
// ===================================================
void getSysJSONData(String & response);
void handleFormConfig(AsyncWebServerRequest *request);
void handleFactoryReset(AsyncWebServerRequest *request);
void handleReset(AsyncWebServerRequest *request);
void tinfoJSON(AsyncWebServerRequest *request);
void tinfoJSONTable(AsyncWebServerRequest *request);
void sysJSONTable(AsyncWebServerRequest *request);
void confJSONTable(AsyncWebServerRequest *request);
void spiffsJSONTable(AsyncWebServerRequest *request);
void wifiScanJSON(AsyncWebServerRequest *request);
void handleNotFound(AsyncWebServerRequest *request);
void handle_fw_upload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);

#endif // WEBSERVER_H