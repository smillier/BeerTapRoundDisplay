// **********************************************************************************
// TapDisplay WEB Server, route web function
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend this library but please abide with the CC-BY-SA license:
// Attribution-NonCommercial-ShareAlike 4.0 International License
// http://creativecommons.org/licenses/by-nc-sa/4.0/
//
// This program works with the TapDisplay board
// see schematic here https://github.com/smillier/BeerTapRoundDisplay
//
//
// All text above must be included in any redistribution.
//
// **********************************************************************************

// Include header
#include "webserver.h"
#include "FS.h"
#include <string>
#include <SPIFFS.h>

// Optimize string space in flash, avoid duplication
const char FP_JSON_START[] PROGMEM = "{\r\n";
const char FP_JSON_END[] PROGMEM = "\r\n}\r\n";
const char FP_QCQ[] PROGMEM = "\":\"";
const char FP_QCNL[] PROGMEM = "\",\r\n\"";
const char FP_QCB[] PROGMEM = "\":";
const char FP_BNL[] PROGMEM = ",\r\n\"";
const char FP_RESTART[] PROGMEM = "OK, Redémarrage en cours\r\n";
const char FP_NL[] PROGMEM = "\r\n";


/* ======================================================================
Function: getContentType
Purpose : return correct mime content type depending on file extension
Input   : -
Output  : Mime content type
Comments: -
====================================================================== */
String getContentType(String filename) {
  if(filename.endsWith(".htm")) return F("text/html");
  else if(filename.endsWith(".html")) return F("text/html");
  else if(filename.endsWith(".css")) return F("text/css");
  else if(filename.endsWith(".json")) return F("application/json");
  else if(filename.endsWith(".js")) return F("application/javascript");
  else if(filename.endsWith(".png")) return F("image/png");
  else if(filename.endsWith(".gif")) return F("image/gif");
  else if(filename.endsWith(".jpg")) return F("image/jpeg");
  else if(filename.endsWith(".ico")) return F("image/x-icon");
  else if(filename.endsWith(".xml")) return F("text/xml");
  else if(filename.endsWith(".pdf")) return F("application/x-pdf");
  else if(filename.endsWith(".zip")) return F("application/x-zip");
  else if(filename.endsWith(".gz")) return F("application/x-gzip");
  else if(filename.endsWith(".otf")) return F("application/x-font-opentype");
  else if(filename.endsWith(".eot")) return F("application/vnd.ms-fontobject");
  else if(filename.endsWith(".svg")) return F("image/svg+xml");
  else if(filename.endsWith(".woff")) return F("application/x-font-woff");
  else if(filename.endsWith(".woff2")) return F("application/x-font-woff2");
  else if(filename.endsWith(".ttf")) return F("application/x-font-ttf");
  return "text/plain";
}

/* ======================================================================
Function: formatSize
Purpose : format a asize to human readable format
Input   : size
Output  : formated string
Comments: -
====================================================================== */
String formatSize(size_t bytes)
{
  if (bytes < 1024){
    return String(bytes) + F(" Byte");
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0) + F(" KB");
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0) + F(" MB");
  } else {
    return String(bytes/1024.0/1024.0/1024.0) + F(" GB");
  }
}

/* ======================================================================
Function: handleFileRead
Purpose : return content of a file stored on SPIFFS file system
Input   : file path
Output  : true if file found and sent
Comments: -
====================================================================== */
bool handleFileRead(String path, AsyncWebServerRequest *request) {
  if ( path.endsWith("/") )
    path += F("index.htm");

  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  bool gzip = false;

  //Log.verbose(F("handleFileRead "));
  //Log.verbose(path.c_str());

  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz)){
      path += F(".gz");
      //Log.verbose(F(".gz"));
      gzip = true;
    }

    //Log.verbose(F(" found on FS\nContentType: "));
    //Log.verbose(contentType.c_str());
    //Log.verbose("\r\n");

    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, path, contentType);
    if (gzip) {
      //Log.verbose(F("Add header content-encoding: gzip\r\n"));
      response->addHeader("Content-Encoding", "gzip");
    }
    request->send(response);
    return true;
  }

  request->send(404, "text/plain", "File Not Found");
  return false;
}

/* ======================================================================
Function: formatNumberJSON
Purpose : check if data value is full number and send correct JSON format
Input   : String where to add response
          char * value to check
Output  : -
Comments: 00150 => 150
          ADCO  => "ADCO"
          1     => 1
====================================================================== */
void formatNumberJSON( String &response, char * value)
{
  // we have at least something ?
  if (value && strlen(value))
  {
    boolean isNumber = true;
    char * p = value;

    // just to be sure
    if (strlen(p)<=16) {
      // check if value is number
      while (*p && isNumber) {
        if ( *p < '0' || *p > '9' )
          isNumber = false;
        p++;
      }

      // this will add "" on not number values
      if (!isNumber) {
        response += '\"' ;
        response += value ;
        response += '\"' ;
      } else {
        // this will remove leading zero on numbers
        p = value;
        while (*p=='0' && *(p+1) )
          p++;
        response += p ;
      }
    } else {
      response += F("\"Error Value too long\"");
      //Log.error(F("formatNumberJSON Value too long!\r\n"));
    }
  } else {
    response += F("\"Error Bad Value\"");
    //Log.error(F("formatNumberJSON Bad Value!\r\n"));
  }
}

/* ======================================================================
Function: tinfoJSONTable
Purpose : dump all teleinfo values in JSON table format for browser
Input   : linked list pointer on the concerned data
          true to dump all values, false for only modified ones
Output  : -
Comments: -
====================================================================== */
void tinfoJSONTable(AsyncWebServerRequest *request)
{
  // Just to debug where we are
 // Log.verbose(F("Serving /tinfo page...\r\n"));

  #ifdef MOD_TELEINFO

    ValueList * me = tinfo.getList();
    String response = "";

    // Got at least one ?
    if (me) {
      boolean first_item = true;
      // Json start
      response += F("[\r\n");

      // Loop thru the node
      while (me->next) {

        // we're there
        ESP.wdtFeed();

        // go to next node
        me = me->next;

        // First item do not add , separator
        if (first_item)
          first_item = false;
        else
          response += F(",\r\n");

        response += F("{\"na\":\"");
        response +=  me->name ;
        response += F("\", \"va\":\"") ;
        response += me->value;
        response += F("\", \"ck\":\"") ;
        if (me->checksum == '"' || me->checksum == '\\' || me->checksum == '/')
          response += '\\';
        response += (char) me->checksum;
        response += F("\", \"fl\":");
        response += me->flags ;
        response += '}' ;

      }
     // Json end
     response += F("\r\n]");

    } else {
      Log.verbose(F("sending 404...\r\n"));
      request->send(404, "text/plain", "No data");
    }
    Log.verbose(F("sending..."));
    request->send(200, "application/json", response);
    Log.verbose(F("OK!\r\n"));
  #else
    //Log.verbose(F("sending 404...\r\n"));
    request->send(404, "text/plain", "Teleinfo non activée");
  #endif // MOD_TELEINFO

}

/* ======================================================================
Function: getSysJSONData
Purpose : Return JSON string containing system data
Input   : Response String
Output  : -
Comments: -
====================================================================== */
void getSysJSONData(String & response)
{
  const size_t capacity = JSON_OBJECT_SIZE(21) + 429;
  StaticJsonDocument<capacity> doc;
  char buffer[60];

  doc["uptime"] = uptime;

  // Version Logiciel
  doc["version"] = VERSION;

  strcpy(buffer, __DATE__);
  strcat(buffer, " ");
  strcat(buffer, __TIME__);
  doc["compilation_date"] = buffer;
 
  // Version Matériel
  doc["board"] = TAPDISPLAY_BOARD;

  // Modules activés
  strcpy(buffer, "");
  #ifdef MOD_OLED
    strcat_P(buffer, PSTR("OLED "));
  #endif
  #ifdef MOD_TELEINFO
    strcat_P(buffer, PSTR("TELEINFO "));
  #endif
  #ifdef MOD_RF69
    strcat_P(buffer, PSTR("RF69 "));
  #endif
  #ifdef MOD_ADPS
    strcat_P(buffer, PSTR("ADPS "));
  #endif
  #ifdef MOD_MQTT
    strcat_P(buffer, PSTR("MQTT "));
  #endif
  #ifdef MOD_EMONCMS
    strcat_P(buffer, PSTR("EMONCMS "));
  #endif
  #ifdef MOD_JEEDOM
    strcstrcat_Pat(buffer, PSTR("JEEDOM "));
  #endif
  doc["modules"] = buffer;
  
  doc["sdk_version"] = system_get_sdk_version();
  

  //sprintf_P(buffer, PSTR("0x%0X"), system_get_chip_id());
  //doc["chip_id"] = buffer;

  //sprintf_P(buffer, PSTR("0x%0X"), system_get_boot_version());
  //doc["boot_version"] = buffer;
  
  doc["cpu_freq"]        = ESP.getCpuFreqMHz();
  doc["flash_real_size"] = ESP.getFlashChipSize();
  doc["firmware_size"]   = ESP.getSketchSize();
  doc["free_size"]       = ESP.getFreeSketchSpace();

  doc["ip"]   = WiFi.localIP().toString();
  doc["mac"]  = WiFi.macAddress();
  doc["ssid"] = WiFi.SSID();
  doc["rssi"] = WiFi.RSSI();

  //FSInfo info;
 // SPIFFS.info(info);
  //doc["spiffs_total"] = info.totalBytes;
  //doc["spiffs_used"] = info.usedBytes;
  //sprintf_P(buffer, PSTR("%d"), 100 * info.usedBytes / info.totalBytes);
  //doc["spiffs_used_percent"] = buffer;

  //doc["free_ram"] = system_get_free_heap_size();
  //sprintf_P(buffer, PSTR("%.2f"), 100 - getLargestAvailableBlock() * 100.0 / getTotalAvailableMemory());
  //doc["heap_fragmentation"] = buffer;

  serializeJson(doc, response);
}

/* ======================================================================
Function: sysJSONTable
Purpose : dump all sysinfo values in JSON table format for browser
Input   : -
Output  : -
Comments: -
====================================================================== */
void sysJSONTable(AsyncWebServerRequest *request)
{
  String response;
  response.reserve(600);

  getSysJSONData(response);

  // Just to debug where we are
  //Log.verbose(F("Serving /system page..."));
  request->send(200, "application/json", response);
  //Log.verbose(F("Ok!\r\n"));
}

/* ======================================================================
Function: getConfigJSONData
Purpose : Return JSON string containing configuration data
Input   : Response String
Output  : -
Comments: -
====================================================================== */
void getConfJSONData(String & r)
{
  // Json start
  r = FPSTR(FP_JSON_START);

  r+="\"";
  r+=CFG_FORM_SSID;            r+=FPSTR(FP_QCQ); r+=config.ssid;            r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_PSK;             r+=FPSTR(FP_QCQ); r+=config.psk;             r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_HOST;            r+=FPSTR(FP_QCQ); r+=config.host;            r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_AP_PSK;          r+=FPSTR(FP_QCQ); r+=config.ap_psk;          r+= FPSTR(FP_QCNL);
  #ifdef MOD_EMONCMS
  r+=CFG_FORM_EMON_HOST;       r+=FPSTR(FP_QCQ); r+=config.emoncms.host;    r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_EMON_PORT;       r+=FPSTR(FP_QCQ); r+=config.emoncms.port;    r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_EMON_URL;        r+=FPSTR(FP_QCQ); r+=config.emoncms.url;     r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_EMON_KEY;        r+=FPSTR(FP_QCQ); r+=config.emoncms.apikey;  r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_EMON_NODE;       r+=FPSTR(FP_QCQ); r+=config.emoncms.node;    r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_EMON_FREQ;       r+=FPSTR(FP_QCQ); r+=config.emoncms.freq;    r+= FPSTR(FP_QCNL);
  #endif
  r+=CFG_FORM_OTA_AUTH;        r+=FPSTR(FP_QCQ); r+=config.ota_auth;        r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_OTA_PORT;        r+=FPSTR(FP_QCQ); r+=config.ota_port;        r+= FPSTR(FP_QCNL);
  
  #ifdef MOD_JEEDOM
  r+=CFG_FORM_JDOM_HOST;       r+=FPSTR(FP_QCQ); r+=config.jeedom.host;     r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_JDOM_PORT;       r+=FPSTR(FP_QCQ); r+=config.jeedom.port;     r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_JDOM_URL;        r+=FPSTR(FP_QCQ); r+=config.jeedom.url;      r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_JDOM_KEY;        r+=FPSTR(FP_QCQ); r+=config.jeedom.apikey;   r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_JDOM_ADCO;       r+=FPSTR(FP_QCQ); r+=config.jeedom.adco;     r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_JDOM_FING;       r+=FPSTR(FP_QCQ); r+=getFingerPrint();       r+= FPSTR(FP_QCNL);
  r+=CFG_FORM_JDOM_FREQ;       r+=FPSTR(FP_QCQ); r+=config.jeedom.freq;
  #else
  r+= FPSTR(FP_QCNL);
  #endif

  #ifdef MOD_MQTT
    r+= FPSTR(FP_QCNL);
    r+=CFG_FORM_MQTT_ACTIVATED; r+=FPSTR(FP_QCB); r+=config.mqtt.isActivated; r+= FPSTR(FP_BNL);
    r+=CFG_FORM_MQTT_PROTO;     r+=FPSTR(FP_QCQ); r+=config.mqtt.protocol;    r+= FPSTR(FP_QCNL);
    r+=CFG_FORM_MQTT_HOST;      r+=FPSTR(FP_QCQ); r+=config.mqtt.host;        r+= FPSTR(FP_QCNL);
    r+=CFG_FORM_MQTT_PORT;      r+=FPSTR(FP_QCQ); r+=config.mqtt.port;        r+= FPSTR(FP_QCNL);
    r+=CFG_FORM_MQTT_AUTH;      r+=FPSTR(FP_QCB); r+=config.mqtt.hasAuth;     r+= FPSTR(FP_BNL);
    r+=CFG_FORM_MQTT_USER;      r+=FPSTR(FP_QCQ); r+=config.mqtt.user;        r+= FPSTR(FP_QCNL);
    r+=CFG_FORM_MQTT_PASS;      r+=FPSTR(FP_QCQ); r+=config.mqtt.password;
  #endif

  r+= F("\"");
  // Json end
  r += FPSTR(FP_JSON_END);
}

/* ======================================================================
Function: confJSONTable
Purpose : dump all config values in JSON table format for browser
Input   : -
Output  : -
Comments: -
====================================================================== */
void confJSONTable(AsyncWebServerRequest *request)
{
  String response;
  getConfJSONData(response);
  // Just to debug where we are
  //Log.verbose(F("Serving /config page..."));
  request->send(200, "application/json", response);
  //Log.verbose(F("Ok!\r\n"));
}

/* ======================================================================
Function: getSpiffsJSONData
Purpose : Return JSON string containing list of SPIFFS files
Input   : Response String
Output  : -
Comments: -
====================================================================== */
void getSpiffsJSONData(String & response)
{
  bool first_item = true;

  // Json start
  response = FPSTR(FP_JSON_START);

  // Files Array
  response += F("\"files\":[\r\n");

  response += F("],\r\n");


  // SPIFFS File system array
  response += F("\"spiffs\":[\r\n{");

  // Get SPIFFS File system informations
  //FSInfo info;
  //LittleFS.info(info);
  //response += F("\"Total\":");
 // response += info.totalBytes ;
 // response += F(", \"Used\":");
  //response += info.usedBytes ;
  //response += F(", \"ram\":");
  //response += system_get_free_heap_size() ;
  response += F("}\r\n]");

  // Json end
  response += FPSTR(FP_JSON_END);
}

/* ======================================================================
Function: spiffsJSONTable
Purpose : dump all spiffs system in JSON table format for browser
Input   : -
Output  : -
Comments: -
====================================================================== */
void spiffsJSONTable(AsyncWebServerRequest *request)
{
  String response;
  getSpiffsJSONData(response);
  request->send(200, "application/json", response);
}

/* ======================================================================
Function: wifiScanJSON
Purpose : scan Wifi Access Point and return JSON code
Input   : -
Output  : -
Comments: -
====================================================================== */
void wifiScanJSON(AsyncWebServerRequest *request)
{
  String response;
  bool first = true;
  int scanStatus = WiFi.scanComplete();

  // Just to debug where we are
  //Log.verbose(F("Serving /wifiscan page..."));

  // Json start
  response += F("{\r\n");

  //int n = WiFi.scanNetworks();
  if (scanStatus == WIFI_SCAN_FAILED) {
    WiFi.scanNetworks(true);
    response += F("\"status\": \"Scan in progess\"");
  } else if (scanStatus >= 0) {
    response += F("\"result\": [");
    for (uint8_t i = 0; i < scanStatus; ++i) {
      int8_t rssi = WiFi.RSSI(i);

      //uint8_t percent = 0;

      // dBm to Quality
      /*if(rssi<=-100) {
        percent = 0;
      } else if (rssi>=-50) {
        percent = 100;
      } else {
        percent = 2 * (rssi + 100);
      }*/

      if (first)
        first = false;
      else
        response += F(",");

      response += F("{\"ssid\":\"");
      response += WiFi.SSID(i);
      response += F("\",\"rssi\":") ;
      response += rssi;
      response += FPSTR(FP_JSON_END);
    }
    response += F("],\"status\": \"OK\"");
    WiFi.scanDelete();
  }

  // Json end
  response += F("}\r\n");

  //Log.verbose(response.c_str());
  //Log.verbose(F("\r\nsending..."));
  request->send(200, "application/json", response);
  //Log.verbose(F("Ok!\r\n"));
}


/* ======================================================================
Function: tinfoJSON
Purpose : dump all teleinfo values in JSON
Input   : -
Output  : -
Comments: -
====================================================================== */
void tinfoJSON(AsyncWebServerRequest *request)
{
  #ifdef MOD_TELEINFO
    String response;
    getTinfoListJson(response);
    if (response != "-1")
      request->send(200, "application/json", response);
    else
      request->send(404, "text/plain", "No data");
  #else
    request->send(404, "text/plain", "teleinfo not enabled");
  #endif
}









/* ======================================================================
Function: handleFactoryReset
Purpose : reset the module to factory settingd
Input   : -
Output  : -
Comments: -
====================================================================== */
void handleFactoryReset(AsyncWebServerRequest *request)
{
  // Just to debug where we are
  //Log.verbose(F("Serving /factory_reset page...\r\n"));
  resetConfig();
  ///ESP.eraseConfig();
  request->send(200, "text/plain", FPSTR(FP_RESTART));
  delay(1000);
  ESP.restart();
  while (true)
    delay(1);
}

/* ======================================================================
Function: handleReset
Purpose : reset the module
Input   : -
Output  : -
Comments: -
====================================================================== */
void handleReset(AsyncWebServerRequest *request)
{
  // Just to debug where we are
  //Log.verbose(F("Serving /reset page...\r\n"));
  request->send(200, "PSTR(""text/plain", FPSTR(FP_RESTART));
  delay(1000);
  ESP.restart();
  // This will fire watchdog
  while (true)
    delay(1);
}

/* ======================================================================
Function: parseHexNibble
Purpose : converti un caractère en valeur numérique
Input   : pb  - caractère a convertir
          res - la variable qui contient le résultat
Output  : bool
Comments: -
====================================================================== */
#ifdef MOD_JEEDOM
static bool parseHexNibble(char pb, uint8_t* res)
{
  if (pb >= '0' && pb <= '9') {
    *res = (uint8_t) (pb - '0'); return true;
  } else if (pb >= 'a' && pb <= 'f') {
    *res = (uint8_t) (pb - 'a' + 10); return true;
  } else if (pb >= 'A' && pb <= 'F') {
    *res = (uint8_t) (pb - 'A' + 10); return true;
  }
  return false;
}
#endif

/* ======================================================================
Function: convertFinger
Purpose : converti la chaine fingerprint en tableau hexa
Input   : fp   - chaine finger print
          sha1 - tableau hexa
Output  : bool
Comments: -
====================================================================== */
#ifdef MOD_JEEDOM
bool convertFinger(const char* fp, uint8_t sha1[]) {
  int len = strlen(fp);
  int pos = 0;
  for (size_t i = 0; i < CFG_JDOM_FINGER_PRINT_SIZE; ++i) {
    while (pos < len && ((fp[pos] == ' ') || (fp[pos] == ':'))) {
      ++pos;
    }
    if (pos > len - 2) {
      #ifdef DEBUG
        Debugf("pos:%d len:%d fingerprint too short\r\n", pos, len);
      #endif
      return false;
    }
    uint8_t high, low;
    if (!parseHexNibble(fp[pos], &high) || !parseHexNibble(fp[pos+1], &low)) {
      #ifdef DEBUG
        Debugf("pos:%d len:%d invalid hex sequence: %c%c\r\n", pos, len, fp[pos], fp[pos+1]);
      #endif
      return false;
    }
    //Debugf("fp[%d-%d]: %c%c ", pos, i, fp[pos], fp[pos+1]);
    // Debugflush();
    pos += 2;
    sha1[i] = low | (high << 4);
    //Debugf("%X %d\n", sha1[i], sha1[i]);
  }
  return true;
}
#endif

/* ======================================================================
Function: handleFormConfig
Purpose : handle main configuration page
Input   : -
Output  : -
Comments: -
====================================================================== */
void handleFormConfig(AsyncWebServerRequest *request)
{
  String response="";
  int ret;
  boolean showconfig = false;

  // We validated config ?
  if (request->hasParam("save", true)) {
    int itemp;
    //Log.verbose(F("===== Posted configuration\r\n"));

    // WifInfo
    strncpy(config.ssid,        request->getParam("ssid", true)->value().c_str(),     CFG_SSID_SIZE );
    strncpy(config.psk,         request->getParam("psk", true)->value().c_str(),      CFG_PSK_SIZE );
    strncpy(config.host,        request->getParam("host", true)->value().c_str(),     CFG_HOSTNAME_SIZE );
    strncpy(config.ap_psk,      request->getParam("ap_psk", true)->value().c_str(),   CFG_PSK_SIZE );
    if (strcmp(config.ota_auth, request->getParam("ota_auth", true)->value().c_str()) != 0) {
      strncpy(config.ota_auth,  request->getParam("ota_auth", true)->value().c_str(), CFG_PSK_SIZE );
      //reboot = true;
    }
    if (request->hasParam("ota_port", true)) {
      itemp = request->getParam("ota_port", true)->value().toInt();
      config.ota_port = (itemp>=0 && itemp<=65535) ? itemp : DEFAULT_OTA_PORT;
    }
    if (request->hasParam("cfg_led_bright", true)) {
      //Log.verbose("cfg_led_bright: ");
     // Log.verbose(request->getParam("cfg_led_bright", true)->value().c_str());
      //Log.verbose("\r\n");

      config.led_bright = map(request->getParam("cfg_led_bright", true)->value().toInt(), 0, 100, 0, 255);
     // rgb_brightness = config.led_bright;
    }

    // Modele compteur
    //strncpy(config.compteur_modele, request->getParam("compteur_modele", true)->value().c_str(), CFG_COMPTEUR_MODELE_SIZE);
    //strncpy(config.compteur_tic,    request->getParam("compteur_tic", true)->value().c_str(),    CFG_COMPTEUR_TIC_SIZE);

    #ifdef MOD_EMONCMS
      // Emoncms
      strncpy(config.emoncms.host,   request->getParam("emon_host", true)->value().c_str(),   CFG_EMON_HOST_SIZE );
      strncpy(config.emoncms.url,    request->getParam("emon_url", true)->value().c_str(),    CFG_EMON_URL_SIZE );
      strncpy(config.emoncms.apikey, request->getParam("emon_apikey", true)->value().c_str(), CFG_EMON_APIKEY_SIZE );
      itemp = request->getParam("emon_node", true)->value().toInt();
      config.emoncms.node = (itemp>=0 && itemp<=255) ? itemp : 0 ;
      itemp = request->getParam("emon_port", true)->value().toInt();
      config.emoncms.port = (itemp>=0 && itemp<=65535) ? itemp : CFG_EMON_DEFAULT_PORT ;
      itemp = request->getParam("emon_freq", true)->value().toInt();
      if (itemp>0 && itemp<=86400){
        // Emoncms Update if needed
        Tick_emoncms.detach();
        Tick_emoncms.attach(itemp, Task_emoncms);
      } else {
        itemp = 0 ;
      }
      config.emoncms.freq = itemp;
    #endif

    #ifdef MOD_JEEDOM
      // jeedom
      strncpy(config.jeedom.host,   request->getParam("jdom_host", true)->value().c_str(),   CFG_JDOM_HOST_SIZE );
      strncpy(config.jeedom.url,    request->getParam("jdom_url", true)->value().c_str(),    CFG_JDOM_URL_SIZE );
      strncpy(config.jeedom.apikey, request->getParam("jdom_apikey", true)->value().c_str(), CFG_JDOM_APIKEY_SIZE );
      strncpy(config.jeedom.adco,   request->getParam("jdom_adco", true)->value().c_str(),   CFG_JDOM_ADCO_SIZE );
      // On transforme la chaine fingerprint en tableau de valeurs hexadecimales
      if (request->getParam("jdom_finger", true)->value().length() == 59) {
        convertFinger(request->getParam("jdom_finger", true)->value().c_str(), config.jeedom.fingerprint);
      } else {
        // Si la chaine n'est pas correcte, on vide le tableau fingerprint
        for (size_t i = 0; i < CFG_JDOM_FINGER_PRINT_SIZE; i++) {
          config.jeedom.fingerprint[i] = 0;
        }
      }
      if (request->hasParam("jdom_port", true)) {
        itemp = request->getParam("jdom_port", true)->value().toInt();
        config.jeedom.port = (itemp>=0 && itemp<=65535) ? itemp : CFG_JDOM_DEFAULT_PORT;
      }
      if (request->hasParam("jdom_freq", true)) {
        itemp = request->getParam("jdom_freq", true)->value().toInt();
        if (itemp>0 && itemp<=86400){
          // Emoncms Update if needed
          Tick_jeedom.detach();
          Tick_jeedom.attach(itemp, Task_jeedom);
        } else {
          itemp = 0 ;
        }
        config.jeedom.freq = itemp;
      }
    #endif

    #ifdef MOD_MQTT
      // MQTT
      if (request->hasParam("mqtt_isActivated", true)) {
        config.mqtt.isActivated = true;

        strncpy(config.mqtt.protocol, request->getParam("mqtt_protocol", true)->value().c_str(),   CFG_MQTT_PROTOCOL_SIZE);
        strncpy(config.mqtt.host,     request->getParam("mqtt_host", true)->value().c_str(),       CFG_MQTT_HOST_SIZE);
        itemp = request->getParam("mqtt_port", true)->value().toInt();
        config.mqtt.port = (itemp>=0 && itemp<=65535) ? itemp : CFG_MQTT_DEFAULT_PORT ;

        if (request->hasParam("mqtt_hasAuth", true)) {
          config.mqtt.hasAuth = true;

          strncpy(config.mqtt.user,     request->getParam("mqtt_user", true)->value().c_str(),       CFG_MQTT_USER_SIZE);
          strncpy(config.mqtt.password, request->getParam("mqtt_password", true)->value().c_str(),   CFG_MQTT_PASSWORD_SIZE);
        }
        else {
            config.mqtt.hasAuth = false;
        }
      }
      else {
        config.mqtt.isActivated = false;
      }

      if (mqttIsConnected()) {
        disconnectMqtt();
      }

      if (config.mqtt.isActivated && !mqttIsConnected()) {
        connectToMqtt();
      }
    #endif

    if ( saveConfig() ) {
      ret = 200;
      response = F("OK");
    } else {
      ret = 412;
      response = F("Unable to save configuration");
    }

    showconfig = true;
  }
  else
  {
    ret = 400;
    response = F("Missing Form Field");
  }
  //Log.verbose(F("Sending response %d : "), ret);
  //Log.verbose(response.c_str());
  //Log.verbose("\r\n");
  request->send(ret, "text/plain", response);

  // This is slow, do it after response sent
  if (showconfig) {
    showConfig();
  }
}



/* ======================================================================
Function: handleNotFound
Purpose : default WEB routing when URI is not found
Input   : -
Output  : -
Comments: We search is we have a name that match to this URI, if one we
          return it's pair name/value in json
====================================================================== */
void handleNotFound(AsyncWebServerRequest *request)
{
  String response = "";

  String sUri = request->url();
  const char * uri;
  bool found = false;

  // convert uri to char * for compare
  uri = sUri.c_str();

  //Log.verbose(F("default WEB routing\r\n"));
  //Log.verbose(F("URI[%d]='"), strlen(uri));
  //Log.verbose(uri);
  //.verbose(F("'\r\n"));

  // Got consistent URI, skip fisrt / ?
  // Attention si ? dans l'URL çà ne fait pas partie de l'URI
  // mais de hasArg traité plus bas
  if (uri && *uri == '/' && *++uri ) {
    uint8_t len = strlen(uri);

    #ifdef MOD_TELEINFO
      // We check for an known label
      ValueList * me = tinfo.getList();

      // Got at least one ?
      if (me) {

        // Loop thru the linked list of values
        while (me->next && !found) {
          // go to next node
          me = me->next;

          //Debugf("compare to '%s' ", me->name);
          // Do we have this one ?
          if (!strcasecmp(me->name, uri)) {
            // no need to continue
            found = true;

            // Add to respone
            response += F("{\r\n\"") ;
            response += me->name ;
            response += F("\":") ;
            formatNumberJSON(response, me->value);
            response += FPSTR(FP_JSON_END);
          }
        }
      }
    #endif

    // Requêtes d'interrogation
    // ========================

    // http://ip_remora/relais
    if (!strcasecmp_P(uri, PSTR("relais"))) {
      //relaisJSON(response);
      found = true;
    // http://ip_remora/delestage
    } else if (!strcasecmp_P(uri, PSTR("delestage"))) {
      //delestageJSON(response);
      found = true;
    // http://ip_remora/fp ou http://ip_remora/fpx
    } else if ((len == 2 || len == 3) && (uri[0] == 'f'|| uri[0] == 'F') && (uri[1] == 'p'|| uri[1] == 'P')) {
      int8_t fp = -1;

      // http://ip_remora/fp
      if (len == 2) {
        fp=0;

      // http://ip_remora/fpx
      } else if (len == 3) {
        fp = uri[2];
        if (fp >= '1' && fp <= ('0' + 0))
         fp -= '0';
      }

      if (fp >= 0 && fp <= 0) {
        //fpJSON(response, fp);
        found = true;
      }
    }
  } // valide URI


  // Requêtes modifiantes (cumulable)
  // ================================
  if (  request->hasParam("fp") ||
        request->hasParam("setfp") ||
        request->hasParam("relais") ||
        request->hasParam("frelais")) {

    int error = 0;
    response = FPSTR(FP_JSON_START);

  

    response += F("\"response\": ") ;
    response += String(error) ;

    response += FPSTR(FP_JSON_END);
    found = true;
  }

  // Got it, send json
  if (found) {
    request->send(200, "application/json", response);
  } else {
    // le fichier demandé existe sur le système SPIFFS ?
    found = handleFileRead(request->url(), request);
  }

  // send error message in plain text
  if (!found) {
    String message = F("File Not Found\r\n");
    message += F("URI: ");
    message += request->url();
    message += F("\r\nMethod: ");
    message += ( request->method() == HTTP_GET ) ? "GET" : "POST";
    message += F("\r\nArguments: ");
    message += request->params();
    message += F("\r\n");

    uint8_t params = request->params();

    for ( uint8_t i = 0; i < params; i++ ) {
      AsyncWebParameter* p = request->getParam(i);
      message += " " + p->name() + ": " + p->value() + "\n";
    }
    request->send(404, "text/plain", message);
  }
}