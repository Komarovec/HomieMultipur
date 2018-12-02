#include <Homie.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "Timer.h"

#define DHTTYPE DHT11
#define DHTPIN 4

const int PIN_RELAY = 16;
const int PIN_BUZZER = 5;
const int PIN_RGBLED_R = 15;
const int PIN_RGBLED_G = 13;
const int PIN_RGBLED_B = 12;
const int PIN_LEDW = 14;

void DHTmeasure();

DHT_Unified dht(DHTPIN, DHTTYPE);
boolean DHTsendFlag = false;

HomieNode DHTNode("dht","dht");
HomieNode relayNode("relay", "relay");
HomieNode ledNode("led", "led");

Timer DHTtimer;

float temp = 0, hum = 0;

bool relaySetHandler(const HomieRange& range, const String& value) {
  if (value != "ON" && value != "OFF") return false;

  bool on = (value == "ON");
  digitalWrite(PIN_RELAY, on ? HIGH : LOW);
  relayNode.setProperty("state").send(value);
  Homie.getLogger() << "Relay is " << (on ? "ON" : "OFF") << endl;

  return true;
}

bool RLedSetHandler(const HomieRange& range, const String& value) {
  if(value != "ON" && value != "OFF") return false;

  bool on = (value == "ON");
  digitalWrite(PIN_RGBLED_R, on ? HIGH : LOW);
  ledNode.setProperty("red").send(value);
  Homie.getLogger() << "RED Led is " << (on ? "ON" : "OFF") << endl;

  return true;
}

bool GLedSetHandler(const HomieRange& range, const String& value) {
  if(value != "ON" && value != "OFF") return false;

  bool on = (value == "ON");
  digitalWrite(PIN_RGBLED_G, on ? HIGH : LOW);
  ledNode.setProperty("green").send(value);
  Homie.getLogger() << "GREEN Led is " << (on ? "ON" : "OFF") << endl;

  return true;
}

bool BLedSetHandler(const HomieRange& range, const String& value) {
  if(value != "ON" && value != "OFF") return false;

  bool on = (value == "ON");
  digitalWrite(PIN_RGBLED_B, on ? HIGH : LOW);
  ledNode.setProperty("blue").send(value);
  Homie.getLogger() << "BLUE Led is " << (on ? "ON" : "OFF") << endl;

  return true;
}

void DHTHandler() {
  Homie.getLogger() << "Temperature: " << temp << " °C | humidity: " << hum << "%" << endl;
  DHTNode.setProperty("temperature").send(String(temp));
  DHTNode.setProperty("humidity").send(String(hum));
}

void DHTmeasure() {
  sensors_event_t event;  

  //Meření teploty
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
  }
  else {
    temp = event.temperature;
  }

  //Meření relativní vlhkosti
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
  }
  else {
    hum = event.relative_humidity;
  }

  DHTsendFlag = true;
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  Serial << endl << endl;

  pinMode(PIN_RGBLED_R, OUTPUT);
  pinMode(PIN_RGBLED_G, OUTPUT);
  pinMode(PIN_RGBLED_B, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);

  DHTtimer.every(20000, DHTmeasure);
  DHTNode.setProperty("temperature/unit").send("°C");
  DHTNode.setProperty("humidity/unit").send("%");

  Homie_setFirmware("multipurFW", "1.0.0");

  relayNode.advertise("state").settable(relaySetHandler);
  ledNode.advertise("red").settable(RLedSetHandler);
  ledNode.advertise("blue").settable(BLedSetHandler);
  ledNode.advertise("green").settable(GLedSetHandler);

  Homie.disableLedFeedback();
  Homie.setup();
}

void loop() {
  if(DHTsendFlag) {
    DHTHandler();
    DHTsendFlag = false;
  }
  DHTtimer.update();
  Homie.loop();
}