#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ArduinoOTA.h>
#include <ThingSpeak.h>
#include <DHT.h>


#include "settings.h"

const char* ssid = SETTINGS_SSID;
const char* password = SETTINGS_PASS;

float f;
float h;

#define DHTPIN 2
#define DHTTYPE DHT11

WiFiClient client;
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready OTA");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  setupDht();
  delay(2000);
  ThingSpeak.begin(client);
  delay(100);
}

int counter;

void loop() {
  counter++;
  if (counter > 200) {
    readTemp();
    postValues();
    counter = 0;
  }
  ArduinoOTA.handle();
  delay(100);
}

void postValues(void) {
  if (!isnan(f)) {
    Serial.println("posting: f: " + convertFloatToString(f) + " h: " + convertFloatToString(h));
    ThingSpeak.setField(1,f);
    ThingSpeak.setField(3,h);
    ThingSpeak.writeFields(SETTINGS_THINGSPEAK_CHANNEL, SETTINGS_THINGSPEAK_KEY);
    // ThingSpeak.writeField(SETTINGS_THINGSPEAK_CHANNEL, 1, f, SETTINGS_THINGSPEAK_KEY);
    // delay(500);
    // ThingSpeak.writeField(SETTINGS_THINGSPEAK_CHANNEL, 3, h, SETTINGS_THINGSPEAK_KEY);
  } 
  else {
    Serial.println("dht read error");    
  }
  delay(500);
}

void readTemp(void) {
  f = dht.readTemperature(true);
  delay(2000);
  h = dht.readHumidity();
  delay(2000);
}

void setupDht(void) {
  dht.begin();
  delay(100);
}

String convertFloatToString(float n) {
  char buf2[16];
  return dtostrf(n, 5, 2, buf2);
}






