/*  ============================================================================

     MrDIY ULP Trigger Sensors

     THE GATEWAY

  ============================================================================= */

#define   PRGM_VERSION         "3.0.1"
//#define   DEBUG_FLAG

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <NTPClient.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "iconset.h"

#ifdef DEBUG_FLAG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

extern const char css[];
extern const char bootstrap[];
extern const char js[];
extern const char html_saved[];
extern const char html_main[];

struct settings {
  char pversion[8];
  char ssid[32];
  char password[64];
  char mqttServer[64];
  char mqttUserName[32];
  char mqttUserPassword[32];
  char ntpServer[64];
  bool oled_enabled;
  bool unit;
} user_setting = {};

/* ------------------------ Wifi --------------------------------------- */

WiFiClient            wifiClient;
ESP8266WebServer      server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
bool  ap_mode = false;

/* ------------------------ Messages --------------------------------------- */

#define MESH_ID               6734922
#define GROUP_SWITCH          1
#define GROUP_HT              2
#define GROUP_MOTION          3
#define battery_cutoff_volt   3.3

typedef struct struct_message {
  int     mesh_id;
  uint8_t sensor_id[6];
  byte    category;
  bool    status ;
  float   temperature;
  float   humidity;
  float   battery;
} struct_message;

/* --------------------------- Sensors ---------------------------------------------- */

typedef struct sensor_data {
  int     mesh_id;          //4
  uint8_t sensor_id[6];     //6
  byte    category;         //1
  bool    status;           //1
  float   temperature;      //4
  float   humidity;         //4
  float   battery;          //4
  time_t  timestamp;        //8 = 32
} sensor_data;

#define         NUM_SENSORS 30
struct_message  msg;
sensor_data     sensors[NUM_SENSORS];
int             sensors_saved = 0;
uint8_t         incomingData[sizeof(struct struct_message)];
size_t          received_msg_length;
bool            new_sensor_found;

/* --------------------------- MQTT ---------------------------------------------- */

#define       MQTT_MSG_SIZE    200
char          mqttTopic[MQTT_MSG_SIZE];
#define       MSG_BUFFER_SIZE  (50)
PubSubClient  mqttClient(wifiClient);
unsigned long next_mqtt_connection_attempt_timestamp = 0;
String        thingName;
const char*   willTopic         = "LWT";
const char*   willMessage       = "offline";
boolean       willRetain        = false;
byte          willQoS           = 0;
bool          mqtt_connected = false;

/* --------------------------- OLED ---------------------------------------------- */

#define           SCREEN_WIDTH      128
#define           SCREEN_HEIGHT     32
#define           OLED_RESET        -1
#define           SCREEN_ADDRESS    0x3C
unsigned long     last_activity_timestamp = 0;
bool              oled_available = true;
Adafruit_SSD1306  display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const unsigned char logo [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xe0, 0x00, 0x3f, 0x3f, 0x80, 0x01, 0xfc, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xfe, 0x00, 0x3f, 0x1f, 0xc0, 0x03, 0xf8, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0x80, 0x3f, 0x1f, 0xc0, 0x03, 0xf8, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xc0, 0x3f, 0x0f, 0xe0, 0x07, 0xf0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xe0, 0x3f, 0x0f, 0xe0, 0x07, 0xf0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf0, 0x3f, 0x07, 0xf0, 0x0f, 0xe0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf8, 0x3f, 0x07, 0xf0, 0x0f, 0xe0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0x03, 0xf8, 0x1f, 0xc0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0x01, 0xfc, 0x3f, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xfe, 0x3f, 0x01, 0xfc, 0x3f, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0x3f, 0xef, 0xff, 0xfe, 0x3f, 0x00, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0x1f, 0xcf, 0xff, 0xfe, 0x3f, 0x00, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0x1f, 0x8f, 0xff, 0xff, 0x3f, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0x0f, 0x08, 0x1f, 0xff, 0x3f, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0x07, 0x08, 0x1f, 0xff, 0x3f, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0x02, 0x08, 0x7f, 0xff, 0x3f, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0x10, 0x48, 0xff, 0xff, 0x3f, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0x10, 0xc8, 0xff, 0xff, 0x3f, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0x18, 0xc8, 0xff, 0xff, 0x3f, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0x1d, 0xc8, 0xff, 0xff, 0x3f, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0x1f, 0xc8, 0xff, 0xfe, 0x3f, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0x1f, 0xc8, 0xff, 0xfe, 0x3f, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xfe, 0x3f, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf8, 0x3f, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf0, 0x3f, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xe0, 0x3f, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xc0, 0x3f, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0x00, 0x3f, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xfc, 0x00, 0x3f, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xc0, 0x00, 0x3f, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00
};

/* --------------------------- SD CARD -------------------------------------------- */
bool sd_card_found = true;


/* --------------------------- Others -------------------------------------------- */

unsigned long next_receiver_ping_timestamp;
byte          receiver_status = 1;    // tracks the ESPNow receiver status


/* ############################ Setup ############################################ */

void setup() {

  Serial.begin(115200);

  EEPROM.begin(sizeof(struct settings) );
  EEPROM.get( 0, user_setting );

  if ( String(user_setting.pversion) != PRGM_VERSION ) {
    debugln("New PRGM_VERSION found");
    memset(&user_setting, 0, sizeof(settings));
  }

  if ( !display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    oled_available = false;
  } else {
    display.clearDisplay();
    display.dim(true);
  }

  showLogo();
  if (String(user_setting.ssid) != "") showInfo("Wifi", "connecting to " + String(user_setting.ssid), 10);
  else showInfo("Wifi", "not configured ", 10);

  WiFi.mode(WIFI_STA);
  WiFi.begin(user_setting.ssid, user_setting.password);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  byte tries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    debug(".");
    delay(500);
    if (tries++ > 20) {
      WiFi.mode(WIFI_AP);
      WiFi.softAP("Setup Portal", "mrdiy.ca");
      debugln("Setup portal started");
      ap_mode = true;
      break;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    debug("IP address:\t");
    debugln(WiFi.localIP());
    showInfo("Wifi", WiFi.localIP().toString(), 10);
    if (String(user_setting.ntpServer) != "" ) timeClient.setPoolServerName(user_setting.ntpServer);
    timeClient.begin();
  }

  MDNS.begin("thehub3");

  server.on("/",  handleRoot);
  server.on("/config", handleConfig);
  server.on("/raw", handleRaw);
  server.on("/sensor", handleRawFile);
  server.on("/names", handleNames);
  server.on("/status", handleStatus);
  server.on("/json", handleJson);
  server.on("/info", handleInfo);
  server.on("/delete", handleDeleteSensor);
  server.on("/rota_enable", handleROTA);
  server.on("/retry_sd", handleRetrySD);
  server.on("/reboot", handleReboot);
  server.on("/bootstrap.css",  html_bootstrap );
  server.on("/main.css",  html_css );
  server.on("/main.js",   html_js );
  server.onNotFound(handleNotFound);
  server.begin();

  if (!SD.begin(16)) {
    sd_card_found = false;
  } else {
    //loadSavedSensors();
    if ( !SD.exists("/data")) SD.mkdir("/data");
    readMemoryFromFile();
  }

  ArduinoOTA.setHostname("MrDIY-Gateway-3");
  ArduinoOTA.onStart([]() {
    showFirmwareProgress(0);
  });
  ArduinoOTA.onEnd([]() {
    showInfo("Firmware", "update successful", 10);
    delay(100);
    ESP.restart();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int l = (progress / (total / 100));
    showFirmwareProgress(l);

  });
  ArduinoOTA.onError([](ota_error_t error) {
    showInfo("Firmware", "update failed", 10);
  });
  ArduinoOTA.begin();

  mqttClient.setServer( user_setting.mqttServer, 1883);
  String MAC = WiFi.macAddress();
  MAC.replace(":", "");
  thingName = "MrDIY_Hub_3_" + MAC;
  new_sensor_found = false;

}

/* ############################ Loop ############################################# */

void loop() {

  ArduinoOTA.handle();
  timeClient.update();
  server.handleClient();
  pingReceiver();
  mqttReconnect();
  mqttClient.loop();
  if ( millis() > last_activity_timestamp ) showIdle();

  if (Serial.available()) {
    received_msg_length = Serial.readBytesUntil('\n', incomingData, sizeof(incomingData));
    if (received_msg_length == sizeof(incomingData)) {  // got a msg from a sensor
      memcpy(&msg, incomingData, sizeof(msg));
      if ( msg.mesh_id == MESH_ID ) {
        sensorMessageReceived();
        saveSensorData();
      }
    } else {
      if (incomingData[0] == '0') receiver_status = 0;
      if (incomingData[0] == '1') receiver_status = 1;
      if (incomingData[0] == '2') {
        receiver_status = 2;
        showInfo("Receiver", "OTA enabled", 600);
      }
    }
  }
}

void pingReceiver() {

  if ( millis() - next_receiver_ping_timestamp > 0 ) {
    Serial.write('0');
    next_receiver_ping_timestamp =  millis() + 5 * 60 * 1000;  // every 5 minutes
    receiver_status = 0;
  }
}
/* ############################ Sensors ############################################# */

void sensorMessageReceived() {

  char macAddr[18];
  sprintf(macAddr, "%02X%02X%02X%02X%02X%02X", msg.sensor_id[0], msg.sensor_id[1], msg.sensor_id[2], msg.sensor_id[3], msg.sensor_id[4], msg.sensor_id[5]);
  DynamicJsonDocument sensor(256);

  if ( msg.category == GROUP_SWITCH) {
    sensor["data"]["category"] = "switch";
    sensor["data"]["status"]  = (int)msg.status ;
  } else if ( msg.category == GROUP_MOTION) {
    sensor["data"]["category"] = "motion";
    sensor["data"]["status"]  = (int)msg.status ;
  } else if ( msg.category == GROUP_HT) {
    sensor["data"]["category"] = "climate";
    sensor["data"]["temperature"]  = getUserUnitTemperature(msg.temperature);
    sensor["data"]["humidity"]  = msg.humidity;
    if ( user_setting.unit == 1  ) sensor["data"]["unit"]  = "°F";
    else sensor["data"]["unit"]  = "°C";
  }
  sensor["data"]["battery"] = float(int(msg.battery * 100)) / 100;
  char payload[100];
  size_t n = serializeJson(sensor, payload);
  mqttPublish(macAddr, payload, n );
  if (msg.category == GROUP_SWITCH)  showMsg("Switch", macAddr, String(msg.status), String(msg.battery), 20);
  else if (msg.category == GROUP_MOTION) showMsg("Motion", macAddr, String(msg.status), String(msg.battery), 20);
  else if (msg.category == GROUP_HT) showMsg("Climate", macAddr, String((int)getUserUnitTemperature(msg.temperature)), String(msg.battery), 20);

}

void saveSensorData() {

  new_sensor_found = false;

  for (int i = 0; i < sensors_saved; i++) {
    if (    sensors[i].sensor_id[0] == msg.sensor_id[0]
            && sensors[i].sensor_id[1] == msg.sensor_id[1]
            && sensors[i].sensor_id[2] == msg.sensor_id[2]
            && sensors[i].sensor_id[3] == msg.sensor_id[3]
            && sensors[i].sensor_id[4] == msg.sensor_id[4]
            && sensors[i].sensor_id[5] == msg.sensor_id[5]) {
      sensors[i].category = msg.category;
      sensors[i].status = msg.status;
      sensors[i].temperature = msg.temperature;
      sensors[i].humidity = msg.humidity;
      sensors[i].battery = msg.battery;
      sensors[i].timestamp = timeClient.getEpochTime();
      new_sensor_found = true;
    }
  }

  if ( new_sensor_found == false ) {
    sensors[sensors_saved].category = msg.category;
    sensors[sensors_saved].status = msg.status;
    sensors[sensors_saved].temperature = msg.temperature;
    sensors[sensors_saved].humidity = msg.humidity;
    sensors[sensors_saved].battery = msg.battery;
    sensors[sensors_saved].timestamp = timeClient.getEpochTime();
    for (int i = 0; i < 6; i++) sensors[sensors_saved].sensor_id[i] = msg.sensor_id[i];
    sensors_saved++;
  }

  if (sd_card_found) {
    char filename[20];
    sprintf(filename, "/data/%02X%02X%02X%02X%02X%02X.log",  msg.sensor_id[0],  msg.sensor_id[1],  msg.sensor_id[2],  msg.sensor_id[3],  msg.sensor_id[4],  msg.sensor_id[5]);
    File log_file = SD.open( filename, FILE_WRITE);
    log_file.print(timeClient.getEpochTime());
    log_file.print(",");
    log_file.print(msg.category);
    log_file.print(",");
    log_file.print(msg.status);
    log_file.print(",");
    log_file.print(msg.temperature);
    log_file.print(",");
    log_file.print(msg.humidity);
    log_file.print(",");
    log_file.println(msg.battery);
    log_file.flush();
    log_file.close();

    saveMemoryToFile();
  }
}

/* ############################ Portal ############################################# */

void handleRoot() {

  server.sendHeader("Cache-Control", "public, max-age=604800, immutable");
  server.send_P(200, "text/html", html_main);
}

void handleRaw() {

  // format: id,category,status,temperature,humidity,unit,battery,timestamp\n ( per line)
  String raw;
  for (int i = 0; i < sensors_saved; i++) {
    char macAddr[18];
    sprintf(macAddr, "%02X%02X%02X%02X%02X%02X", sensors[i].sensor_id[0], sensors[i].sensor_id[1], sensors[i].sensor_id[2], sensors[i].sensor_id[3], sensors[i].sensor_id[4], sensors[i].sensor_id[5]);
    raw += String(macAddr) + ",";
    raw += String(sensors[i].category) + ",";
    raw += String(sensors[i].status) + ",";
    raw += String(getUserUnitTemperature(sensors[i].temperature)) + ",";
    raw += String(sensors[i].humidity) + ",";
    if ( user_setting.unit == 1  ) raw += "°F,";
    else raw += "°C,";
    raw += String(sensors[i].battery ) + ",";
    raw += String(sensors[i].timestamp);
    if ( i < sensors_saved - 1) raw += "\n";
  }
  server.send(200, "text/plain", raw);
}

void handleReboot() {

  server.send(200, "text/plain", "1");
  Serial.write('1');
  delay(10);
  ESP.restart();
}

void handleROTA() {

  Serial.write('2');    // send '2' to enable the OTA for the receiver ( experimental)
  server.send(200, "text/plain", "1");
}

void handleJson() {

  String sid = server.arg("id");
  if (!sid) {
    server.send(200, "application/json; charset=UTF-8", "0");
    return;
  }

  for (int i = 0; i < sensors_saved; i++) {
    char macAddr[18];
    sprintf(macAddr, "%02X%02X%02X%02X%02X%02X", sensors[i].sensor_id[0], sensors[i].sensor_id[1], sensors[i].sensor_id[2], sensors[i].sensor_id[3], sensors[i].sensor_id[4], sensors[i].sensor_id[5]);
    if (  String(macAddr) == sid) {
      DynamicJsonDocument sensor(256);
      if ( msg.category == GROUP_SWITCH) {
        sensor["data"]["category"] = "switch";
        sensor["data"]["status"]  = (int) msg.status;
      } else if ( msg.category == GROUP_MOTION) {
        sensor["data"]["category"] = "motion";
        sensor["data"]["status"]  = (int) msg.status;
      } else if ( msg.category == GROUP_HT) {
        sensor["data"]["category"] = "climate";
        sensor["data"]["temperature"]  = getUserUnitTemperature(msg.temperature);
        sensor["data"]["humidity"]  = msg.humidity;
        if ( user_setting.unit == 1  ) sensor["data"]["unit"]  = "°F";
        else sensor["data"]["unit"]  = "°C";
      }
      sensor["data"]["battery"] = msg.battery;
      char payload[100];
      serializeJson(sensor, payload);
      server.send(200, "application/json; charset=UTF-8", payload);
      break;
    }
  }

}

void handleInfo() {

  uint32_t realSize = ESP.getFlashChipRealSize();
  uint32_t ideSize = ESP.getFlashChipSize();
  FlashMode_t ideMode = ESP.getFlashChipMode();

  String payload;
  payload += "<!DOCTYPE html><html lang='en'><head><title>The Hub by MrDIY</title><style>*{font-family:system-ui,-apple-system,'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans',sans-serif,'Apple Color Emoji','Segoe UI Emoji','Segoe UI Symbol','Noto Color Emoji'} h4{margin-bottom:5px}</style></head><body><table border=0>";
  payload += "<tr><td width=150><h4>Wifi</h4></td></tr>";
  payload += "<tr><td>mac address</td><td>" + WiFi.macAddress() + "</td></tr>";
  payload += "<tr><td>ssid</td><td>" + String(user_setting.ssid) + "</td></tr>";
  payload += "<tr><td>password</td><td>" + String(user_setting.password[0]) + "*********</td></tr>";
  payload += "<tr><td><h4>MQTT</h4></td></tr>";
  payload += "<tr><td>server</td><td>" + String(user_setting.mqttServer) + "</td></tr>";
  payload += "<tr><td>username</td><td>" + String(user_setting.mqttUserName) + "</td></tr>";
  payload += "<tr><td>password</td><td>" + String(user_setting.mqttUserPassword) + "</td></tr>";
  payload += "<tr><td><h4>NTP</h4></td></tr>";
  payload += "<tr><td>server</td><td>" + String(user_setting.ntpServer) + "</td></tr>";
  payload += "<tr><td><h4>Flash</h4></td></tr>";
  payload += "<tr><td>Id</td><td>" + String(ESP.getFlashChipId()) + "</td></tr>";
  payload += "<tr><td>size (device)</td><td>" + String(realSize) + "</td></tr>";
  payload += "<tr><td>size (ide)</td><td>" + String(ideSize) ;
  if (ideSize != realSize) payload += " (Flash size is configured incorrectly)";
  payload += "</td></tr>";
  payload += "<tr><td>Mode</td><td>";
  if ( ideMode == FM_QIO )  payload += "QIO";
  if ( ideMode == FM_QOUT ) payload += "QOUT";
  if ( ideMode == FM_DIO )  payload += "DIO";
  if ( ideMode == FM_DOUT ) payload += "QOUT";
  payload += "</td></tr>";
  payload += "<tr><td><h4>Firmware</h4></td></tr>";
  payload += "<tr><td>version</td><td>" + String(user_setting.pversion) + "</td></tr>";
  payload += "<tr><td><h4>Time</h4></td></tr>";
  payload += "<tr><td>current</td><td>" + String(timeClient.getEpochTime()) + "</td></tr>";
  payload += "<tr><td>uptime</td><td>" + uptime() + "</td></tr>";
  payload += "</table></body></html>";

  server.send(200, "text/html; charset=UTF-8", payload);

}

void handleConfig() {

  if (server.method() == HTTP_POST) {
    strncpy(user_setting.ssid,              server.arg("ssid").c_str(),             sizeof(user_setting.ssid) );
    strncpy(user_setting.password,          server.arg("password").c_str(),         sizeof(user_setting.password) );
    strncpy(user_setting.mqttServer,        server.arg("mqttServer").c_str(),       sizeof(user_setting.mqttServer) );
    strncpy(user_setting.mqttUserName,      server.arg("mqttUserName").c_str(),     sizeof(user_setting.mqttUserName) );
    strncpy(user_setting.mqttUserPassword,  server.arg("mqttUserPassword").c_str(), sizeof(user_setting.mqttUserPassword) );
    strncpy(user_setting.ntpServer,        server.arg("ntpServer").c_str(),       sizeof(user_setting.ntpServer) );

    if ( server.arg("unit") == "1") user_setting.unit = 1;
    else user_setting.unit = 0;
    if ( server.arg("oled_enabled") == "1") user_setting.oled_enabled = true;
    else user_setting.oled_enabled = false;

    user_setting.ssid[server.arg("ssid").length()]
      = user_setting.password[server.arg("password").length()]
        = user_setting.mqttServer[server.arg("mqttServer").length()]
          = user_setting.mqttUserName[server.arg("mqttUserName").length()]
            = user_setting.mqttUserPassword[server.arg("mqttUserPassword").length()]
              = user_setting.ntpServer[server.arg("ntpServer").length()]
                = 0;  // string terminate
    strncpy(user_setting.pversion, PRGM_VERSION , sizeof(PRGM_VERSION) );
    EEPROM.put(0, user_setting);
    EEPROM.commit();
    debugln("Configuration has been saved to EEPROM");
    server.send_P(200, "text / html", html_saved);
    next_mqtt_connection_attempt_timestamp = millis();
    //showInfo("Portal", "serving info page", 3);
    showIdle();

  } else {

    uint32_t realSize = ESP.getFlashChipRealSize();
    uint32_t ideSize = ESP.getFlashChipSize();
    String c = "", f = "";
    if ( user_setting.unit == 1  ) f = " checked";
    else c = " checked";
    String e = "", d = "";
    if ( user_setting.oled_enabled == 1  ) e = " checked";
    else d = " checked";

    String s = "<!DOCTYPE html><html lang='en'><head><meta name='viewport' content='width=device-width, initial-scale=1, user-scalable=no'/>";
    s += "<meta content='text/html;charset=utf-8' http-equiv='Content-Type'><link rel='icon' type='image/x-icon' href='https://www.mrdiy.ca/favicon.ico'>";
    s += "<title>The Hub by MrDIY</title>";
    s += "  <link rel='icon' type='image/png' sizes='16x16' href='data:image/x-icon;base64,AAABAAEAEBAAAAEAIABoBAAAFgAAACgAAAAQAAAAIAAAAAEAIAAAAAAAQAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAIdUOc6IVTvpiFU654hVOuaIVTvkiVU74IdUOsqFUDaMAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACIVDvli1c9/4pWPf+KVj3/ilY9/4pXPf+LVz3/jVk+/4lWPPaBSS5pAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAgk064oVRPf+EUDz/hFA8/4RQPP98Rz3/fUk8/4VRPP+KVzz/jVk+/4ROMXwAAAAA/8s1///LNf//yzX//8s1///KM///yjL//8oy///KMv//yjL//8ky//C4Nf+wfjr/f0o8/4pWPP+LVz3/AAAAAP/FM///xTP//8Uz///FM///xTP//8Uz///FM///xTP//8Uz///FM///xjP//8wy/76LOf+DTzz/jFg9/4ZTOLX/xTP//8Uz///FM///xTP//8Uz///FM///xTP//8Uz///FM///xTP//8Uz///FM///xzP/f0s8/4pWPP+IVTvj/8Uz///FM///xTP//8Uz///FM///xTP//8Uz///FM///xTP//8Uz///FM///xTP//8oy/6JwO/+HUzz/ilY88//FM///xTP//8Uz///FM///xTP//8Uz///FM///xTP//8Uz///FM///xTP//8Uz///KMv+fbTv/h1Q8/4pWPPP/xTP//8Uz///FM///xTP//8Uz///FM///xTP//8Uz///FM///xTP//8Uz///GM///xTP/fUg8/4pWPP+JVjri/8Yz///GM///xjP//8Yz///GM///xjP//8Yz///GM///xjP//8Yz///HM///yzL/sX46/4RRPP+MWD3/h1I4sf/GMvT/xjL0/8Yy9P/FMvP4wDP99740//e+NP/3vjT/9740//O6NP/dpzf/m2k7/4JOPP+KVjz/iVY8/wAAAAAAAAAAAAAAAAAAAAAAAAAAd0A74X5IPf98Rzz/fEc8/3xHPP99SDz/gEw8/4dTPP+LVz3/jFg9/4FKL2sAAAAAAAAAAAAAAAAAAAAAAAAAAIhVOuaMWD7/i1c9/4tXPf+LVz3/i1c9/4xYPf+OWT//iFQ75XxDJ0gAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACHUji2iFQ6zohTOc2HVDnMh1M5yodUOsaGUjerf0YqVAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA//8AAPAPAADwAwAA8AEAAAABAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQAA8AEAAPADAADwDwAA//8AAA=='/>";
    s += "<meta content='utf-8' http-equiv='encoding'>";
    s += "<link rel='stylesheet' href='bootstrap.css'>";
    s += "<link rel='stylesheet' href='main.css'>";
    s += "</head><body>";
    s += "<div id='top_bar' style='position:absolute;top:0px;height:5px;width:100%;background-color:#33C5FF;'></div>";
    s += "<div class='container py-3'>";
    s += "<header>";
    s += "  <div class='d-flex flex-column flex-md-row align-items-center pb-3 mb-4 border-bottom'>";
    s += "    <a href='/' class='d-flex align-items-center text-dark text-decoration-none'><span class='fs-4 logo'></span></a><span class='fs-4'>Configuration</span>";
    s += "    <nav class='d-inline-flex mt-2 mt-md-0 ms-md-auto'>";
    s += "      <a class='me-3 py-2 text-dark text-decoration-none' href='/rota_enable' onclick='return enableReceiverOTA();'><svg xmlns='http://www.w3.org/2000/svg' width='16' height='16' fill='currentColor' class='bi bi-upload' viewBox='0 0 16 16'><path d='M.5 9.9a.5.5 0 0 1 .5.5v2.5a1 1 0 0 0 1 1h12a1 1 0 0 0 1-1v-2.5a.5.5 0 0 1 1 0v2.5a2 2 0 0 1-2 2H2a2 2 0 0 1-2-2v-2.5a.5.5 0 0 1 .5-.5z'/><path d='M7.646 1.146a.5.5 0 0 1 .708 0l3 3a.5.5 0 0 1-.708.708L8.5 2.707V11.5a.5.5 0 0 1-1 0V2.707L5.354 4.854a.5.5 0 1 1-.708-.708l3-3z'/></svg></a>";
    s += "      <a class='me-3 py-2 text-dark text-decoration-none' href='/' onclick='return reboot();'><svg xmlns='http://www.w3.org/2000/svg' width='16' height='16' fill='currentColor' class='bi bi-power' viewBox='0 0 16 16'><path d='M7.5 1v7h1V1h-1z'/><path d='M3 8.812a4.999 4.999 0 0 1 2.578-4.375l-.485-.874A6 6 0 1 0 11 3.616l-.501.865A5 5 0 1 1 3 8.812z'/></svg></a>";
    s += "    </nav>";
    s += "  </div>";
    s += "</header>";
    s += "<body><main>";
    s += "<div class='row justify-content-between'>";
    if (ideSize != realSize) s += "<div class='alert alert-danger' role='alert'>Your flash size (" + String(ideSize) + ") is configured incorrectly. It should be " + String(realSize) + ".</div>";
    s += "<div class='col-md-6 col-lg-7'>";
    s += "<form action='/config' method='post'>";
    s += "    <div class='col-12 mt-3'><label class='form-label'>Wifi Name</label><input type='text' name='ssid' class='form-control' value='" + String(user_setting.ssid) + "'></div>";
    s += "    <div class='col-12 mt-3'><label class='form-label'>Password</label><input type='password' name='password' class='form-control' value='" + String(user_setting.password) + "'></div>";
    s += "    <div class='col-12 mt-3'><label class='form-label mt-4'>MQTT Server</label><input type='text' name='mqttServer' class='form-control' value='" + String(user_setting.mqttServer) + "'></div>";
    s += "    <div class='col-12 mt-3'><label class='form-label'>MQTT Username</label><input type='text' name='mqttUserName' class='form-control' value='" + String(user_setting.mqttUserName) + "'></div>";
    s += "    <div class='col-12 mt-3'><label class='form-label'>MQTT Password</label><input type='password' name='mqttUserPassword' class='form-control' value='" + String(user_setting.mqttUserPassword) + "'></div>";
    s += "    <div class='col-12 mt-3'><label class='form-label mt-4'>NTP Server</label><input type='text' name='ntpServer' class='form-control' value='" + String(user_setting.ntpServer) + "'></div>";

    s += "    <div class='col-12 mt-3'>";
    s += "          <label class='form-label me-4 mt-4 col-4 col-md-3'>Temperature</label>";
    s += "          <div class='form-check form-check-inline col-2 col-md-3'><input class='form-check-input' type='radio' name='unit' value='0'" + c + "><label class='form-check-label'>celsius</label></div>";
    s += "          <div class='form-check form-check-inline col-2 col-md-3'><input class='form-check-input' type='radio' name='unit' value='1'" + f + "><label class='form-check-label'>fahrenheit</label></div>";
    s += "    </div>";
    s += "    <div class='col-12'>";
    s += "          <label class='form-label me-4 mt-4 col-4 col-md-3'>Display</label>";
    s += "          <div class='form-check form-check-inline col-2 col-md-3'><input class='form-check-input' type='radio' name='oled_enabled' value='1'" + e + "><label class='form-check-label'>always on</label></div>";
    s += "          <div class='form-check form-check-inline col-2 col-md-3'><input class='form-check-input' type='radio' name='oled_enabled' value='0'" + d + "><label class='form-check-label'>sleep</label></div>";
    s += "    </div>";
    s += "    <div class='form-floating'><br/><button class='btn btn-primary btn-lg' type='submit'>Save</button><br /><br /><br /></div>";
    s += " </form>";
    s += "</div>";
    s += "<div class='col-md-5 col-lg-4 order-md-last bg-light p-4 rounded-3 border border-1 text-dark'>";
    s += "<p class='fs-4 border-bottom'>MQTT Integration</p>";
    s += "<p class='text-break'><strong>Topic<br /></strong>stat/mrdiy_sensor_XXXXXXXXXXXX/status</p>";
    s += "<p class='text-break'><strong>Switch type load</strong><br />{\"data\": {\"category\":\"switch\",\"status\":x,\"battery\":xx.x }}</p>";
    s += "<p class='text-break'><strong>Climate type load</strong><br />{\"data\": {\"category\":\"climate\",\"temperature\":xx.x,\"humidity\":xx.x,\"battery\":xx.x }}</p>";
    s += "<p class='fs-4 border-bottom mt-5'>HTTP Integration</p>";
    s += "<p class='text-break'><strong>URL<br /></strong>/json?id=XXXXXXXXXXXX</p>";
    s += "<p class='text-break'><strong>Load</strong><br />simular to the MQTT loads above</p>";
    s += "</div>";
    s += "</div></main></body><script src='main.js'></script><script></script></html>";
    server.send(200, "text/html", s );
  }
}

void handleStatus() {

  // sd card, mqtt, receiver ota, ntp
  bool mqtt_is_good = true;
  if ( String(user_setting.mqttServer) != "" && mqtt_connected != true ) mqtt_is_good = false;

  server.send(200, "text/plain", String(sd_card_found) + "," + String( mqtt_is_good ) + "," + String(receiver_status) + "," + String( timeClient.getEpochTime() > 1635652800 ));
}

void handleRawFile() {

  /*  FORMAT: timestamp, category, status, temperature, humidty, battery  */
  if ( sd_card_found == false ) server.send ( 404, "text/html",  "No SD card found." );

  if ( SD.exists("/data/" + server.arg("id")) ) {
    File sensor = SD.open("/data/" + server.arg("id"));
    int fsize = sensor.size();
    server.sendHeader("Content-Length", (String)(fsize));
    size_t fsizeSent = server.streamFile(sensor, "text/plain");
    sensor.close();
  }  else  server.send ( 404, "text/html",  "Invalid request." );

}

void handleNames() {

  if ( sd_card_found == false ) server.send ( 200, "text/html",  "" );

  if ( SD.exists("/data/config.txt") ) {
    File sensor = SD.open("/data/config.txt");
    int fsize = sensor.size();
    server.sendHeader("Content-Length", (String)(fsize));
    size_t fsizeSent = server.streamFile(sensor, "text/plain");
    sensor.close();
  }  else  server.send ( 200, "text/html",  "" );
}

void handleDeleteSensor() {

  if ( SD.exists("/data/" + server.arg("id")) ) {
    SD.remove("/data/" + server.arg("id"));
    server.send(200, "text/plain", "1");
  } else {
    server.send(200, "text/plain", "0");
  }
}

void handleRetrySD() {

  if (sd_card_found == true) {
    server.send(200, "text/plain", "1");
    return;
  } else {

    if (!SD.begin(16)) {
      sd_card_found = false;
      showInfo("SDCard", "error: not found", 5);
      server.send(200, "text/plain", "0");
    } else {
      showInfo("SD Card", "loading sensors", 3);
      server.send(200, "text/plain", "1");
      sd_card_found = true;
      readMemoryFromFile();
    }
  }
}

void handleNotFound() {

  server.send(404, "text/html", "404 - Page Not Found");
}

void html_css() {

  server.sendHeader("Cache-Control", "public, max-age=604800, immutable");
  server.send_P(200, "text/css", css);
}

void html_bootstrap() {

  server.sendHeader("Cache-Control", "public, max-age=604800, immutable");
  server.send_P(200, "text/css", bootstrap);
}


void html_js() {

  server.sendHeader("Cache-Control", "public, max-age=604800, immutable");
  server.send_P(200, "text/javascript", js);
}

/* ################################# MQTT ########################################### */

void mqttReconnect() {

  mqtt_connected = false;
  if ( String(user_setting.mqttServer) == "" || WiFi.status() != WL_CONNECTED) return;
  if ( mqttClient.connected() ) {
    mqtt_connected = true; return;
  }
  if ( millis() - next_mqtt_connection_attempt_timestamp < 0 ) return;  // using a delay stops OTA uploads

  next_mqtt_connection_attempt_timestamp = millis() + 5 * 60 * 1000;  // retry every 5 minutes
  if (mqttClient.connect( thingName.c_str(), user_setting.mqttUserName, user_setting.mqttUserPassword, "stat/mrdiy_sensors/LWT", willQoS, willRetain, willMessage)) {
    mqttClient.publish("stat/mrdiy_sensors/status", "online");
    mqttClient.publish("stat/mrdiy_sensors/LWT", "online");
    debugln(F("stat/mrdiy_sensors/status ... online "));
    showInfo("MQTT", "connected", 3);
    mqtt_connected = true;
  } else {
    showInfo("MQTT", "connection failed", 3);
    mqtt_connected = false;
  }
}

void mqttPublish(char macAdress[], String payload,  size_t len ) {

  strcpy (mqttTopic, "stat/mrdiy_sensor_");
  strcat (mqttTopic, macAdress);
  strcat (mqttTopic, "/status");
  debug(mqttTopic);
  debug(' ');
  debugln(payload);
  mqttClient.publish(mqttTopic, payload.c_str() , len);
}

/* ################################# OLED ########################################### */

void showInfo(String title, String msg, int idle_timeout) {

  if (!oled_available) return;
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(title);
  display.setTextSize(1);
  display.setCursor(0, 24);
  display.println(msg);
  display.display();
  if ( idle_timeout > 0 ) last_activity_timestamp = millis() + idle_timeout * 1000;
}

void showMsg(String title, String mac, String status, String battery, int idle_timeout) {

  if (!oled_available || !user_setting.oled_enabled) return;
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(title);
  if (status == "0" or status == "1") display.setCursor(116, 0);
  else display.setCursor(104, 0);
  display.print(status);
  display.setTextSize(1);
  display.setCursor(0, 24);
  display.print(mac);
  display.setCursor(98, 24);
  display.print(battery);
  display.print("v");
  display.display();
  if ( idle_timeout > 0 ) last_activity_timestamp = millis() + idle_timeout * 1000;
}

void showFirmwareProgress(int progress) {

  if (!oled_available) return;
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Firmware");
  progress = progress > 100 ? 100 : progress;
  progress = progress < 0 ? 0 : progress;
  float bar = ((float)(128 - 2) / 100) * progress;
  display.drawRect(0, 24, 128, 7, WHITE);
  display.fillRect(2, 24 + 2, bar - 2, 6 - 3, WHITE);
  display.display();
  last_activity_timestamp = millis() + 10 * 1000;
}

void showIdle() {

  if (!oled_available) return;

  if ( ap_mode == true) {
    showInfo("Setup", "http://192.168.4.1", 30000);
    display.drawBitmap(110, 0, wifi1_icon16x16, 16, 16, WHITE);
    display.display();
    return;
  }
  if (!user_setting.oled_enabled) {
    display.clearDisplay();
    display.display();
    return;
  }
  String msg = "";
  int count = 0;
  float lowest_battery = 4.3;
  for (int i = 0; i < sensors_saved; i++) {
    if ( sensors[i].battery < battery_cutoff_volt && sensors[i].battery > 2) count++;
    if ( sensors[i].battery < lowest_battery && sensors[i].battery > 2) lowest_battery = sensors[i].battery;
  }

  if (count == 0) {
    if (WiFi.status() == WL_CONNECTED) msg = WiFi.localIP().toString();
    else msg = "Wifi not connected";
  } else if ( count >= 1 ) {
    msg = "low battery warning";
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(sensors_saved);
  display.setCursor(20, 0);
  if ( sensors_saved == 1) display.print("Sensor");
  else if ( sensors_saved >= 10)  display.print(" Sensors");
  else display.print("Sensors");
  display.setTextSize(1);
  display.setCursor(0, 24);
  display.print(msg);
  if ( sd_card_found == false) display.drawBitmap(110, 16, warning_icon16x16, 16, 16, WHITE);
  display.display();
  last_activity_timestamp = millis() + 5 * 60 * 1000;
}

void showLogo() {

  if (!oled_available) return;
  display.drawBitmap(0, 0, logo, 128, 32, WHITE);
  display.display();
  delay(3000);
}


/* ################################# Tools ########################################### */

float getUserUnitTemperature(float t) {

  if ( user_setting.unit == 1  ) return t * 1.8 + 32;
  return t;
}

String uptime() {

  int duration = time(nullptr);
  if (duration >= 86400) return String(duration / 86400) + " days";
  if (duration >= 3600)  return String(duration / 3600) + + " hours";
  if (duration >= 60)    return String(duration / 60) + " minutes";
  return String(duration) + " seconds";
}

void saveMemoryToFile() {

  if (sd_card_found == true) {
    if (SD.exists("/data/memory.bin")) SD.remove("/data/memory.bin");
    File file = SD.open("/data/memory.bin", FILE_WRITE);
    if (file) {
      file.write(sensors_saved);
      for (int i = 0; i < sensors_saved; i++) file.write((uint8_t *)&sensors[i], sizeof(struct sensor_data));
      file.close();
    }
  }
}

void readMemoryFromFile() {

  if (sd_card_found == true) {
    if (SD.exists("/data/memory.bin")) {
      File file = SD.open("/data/memory.bin", FILE_READ);
      sensors_saved = file.read();
      for (int i = 0; i < sensors_saved; i++) file.read((uint8_t *)&sensors[i], sizeof(struct sensor_data));
      file.close();
    }
  }
}
