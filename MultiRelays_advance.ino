#define BLYNK_TEMPLATE_ID "TMPL39xU7NI-l"
#define BLYNK_TEMPLATE_NAME "DHT"
#define BLYNK_AUTH_TOKEN "IansFrfydm4MhMnLJwkZcR6jbH_UDUl3"
//#define ENABLE_DEBUG
#define BLYNK_PRINT Serial

#ifdef ENABLE_DEBUG
#define DEBUG_ESP_PORT Serial
#define NODEBUG_WEBSOCKETS
#define NDEBUG
#endif

#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
#include <WiFi.h>
#endif

#include <SinricPro.h>
#include <SinricProSwitch.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>


#if defined(ESP8266)
#define RELAYPIN_1 D0
#define RELAYPIN_2 D4
#define RELAYPIN_3 D7
#define dhtPin D8
// #define RELAYPIN_4 D4
// #define RELAYPIN_5 D5
// #define RELAYPIN_6 D6
// #define RELAYPIN_7 D7
// #define RELAYPIN_8 D8
#elif defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
#define RELAYPIN_1 16
#define RELAYPIN_2 17
#define RELAYPIN_3 18
#define RELAYPIN_4 19
#define RELAYPIN_5 21
#define RELAYPIN_6 22
#define RELAYPIN_7 23
#define RELAYPIN_8 25
#endif

/*****************
 * Configuration *
 *****************/

struct RelayInfo {
  String deviceId;
  String name;
  int pin;
};

std::vector<RelayInfo> relays = {
  { "662e53c27c9e6c6fe876bf0c", "Relay 1", RELAYPIN_1 },
  { "662e54073019d22c419913e1", "Relay 2", RELAYPIN_2 },
  { "662e54203019d22c41991420", "Relay 3", RELAYPIN_3 }
  // {"5fxxxxxxxxxxxxxxxxxxxxxx", "Relay 4", RELAYPIN_4},
  // {"5fxxxxxxxxxxxxxxxxxxxxxx", "Relay 5", RELAYPIN_5},
  // {"5fxxxxxxxxxxxxxxxxxxxxxx", "Relay 6", RELAYPIN_6},
  // {"5fxxxxxxxxxxxxxxxxxxxxxx", "Relay 7", RELAYPIN_7},
  // {"5fxxxxxxxxxxxxxxxxxxxxxx", "Relay 8", RELAYPIN_8}
};


#define WIFI_SSID "TFKioT"
#define WIFI_PASS "00000000"
#define APP_KEY "2ecc4359-3fa0-42fa-90d8-429520245ee5"                                          // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET "3027c453-fa79-4398-9652-8c983e65f04a-29148b4f-c67f-4edc-ab44-903fc413bead"  // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"

#define BAUD_RATE 115200  // Change baudrate to your need

bool onPowerState(const String &deviceId, bool &state) {
  for (auto &relay : relays) {                                                             // for each relay configuration
    if (deviceId == relay.deviceId) {                                                      // check if deviceId matches
      Serial.printf("Device %s turned %s\r\n", relay.name.c_str(), state ? "on" : "off");  // print relay name and state to serial
      digitalWrite(relay.pin, state);                                                      // set state to digital pin / gpio
      return true;                                                                         // return with success true
    }
  }
  return false;  // if no relay configuration was found, return false
}

void setupRelayPins() {
  for (auto &relay : relays) {   // for each relay configuration
    pinMode(relay.pin, OUTPUT);  // set pinMode to OUTPUT
  }
}

void setupWiFi() {
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

#if defined(ESP8266)
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
#elif defined(ESP32)
  WiFi.setSleep(false);
#endif

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}

void setupSinricPro() {
  for (auto &relay : relays) {                              // for each relay configuration
    SinricProSwitch &mySwitch = SinricPro[relay.deviceId];  // create a new device with deviceId from relay configuration
    mySwitch.onPowerState(onPowerState);                    // attach onPowerState callback to the new device
  }

  SinricPro.onConnected([]() {
    Serial.printf("Connected to SinricPro\r\n");
  });
  SinricPro.onDisconnected([]() {
    Serial.printf("Disconnected from SinricPro\r\n");
  });

  SinricPro.begin(APP_KEY, APP_SECRET);
}
#define DHTTYPE DHT11
DHT dht(dhtPin, DHTTYPE);

void setup() {
  Serial.begin(BAUD_RATE);
  setupRelayPins();
  setupWiFi();
  Blynk.config(BLYNK_AUTH_TOKEN);
  setupSinricPro();
  dht.begin();
}

void loop() {
  Blynk.run();
  SinricPro.handle();

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Serial.print("Temperature: ");
  // Serial.print(temperature);
  // Serial.print(" °C, Humidity: ");
  // Serial.print(humidity);
  // Serial.println(" %");

  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);
}