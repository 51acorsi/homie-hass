#include <Homie.h>
#include <ArduinoOTA.h>

#define FW_NAME "hh-esp8266"
#define FW_VERSION "0.0.1"

/* Magic sequence for Autodetectable Binary Upload */
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";
/* End of magic sequence for Autodetectable Binary Upload */

//Pins
const int IN_01 = 5;
const int IN_02 = 4;
const int IN_03 = 15;
const int OUT_01 = 14;
const int OUT_02 = 12;
const int OUT_03 = 13;

Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce();
Bounce debouncer3 = Bounce();

//Parameters
const String PAYLOAD_ON = "ON";
const String PAYLOAD_OFF = "OFF";

const int ON_SIGNAL = LOW;
const int OFF_SIGNAL = HIGH;

const int DEBOUNCER_INTERVAL = 40;

//Nodes
HomieNode light01Node("light1", "light");
HomieNode light02Node("light2", "light");
HomieNode light03Node("light3", "light");

//Statuses
int in01LastState = -1;
int in02LastState = -1;
int in03LastState = -1;

bool mqttDisconnected = false;

void onHomieEvent(HomieEvent event) {
  switch(event) {
    case HOMIE_MQTT_CONNECTED:
      if (mqttDisconnected){
        mqttDisconnected = false;
        //Update all statuses in case it has changed in the meantime
        Homie.setNodeProperty(light01Node, "state", ((digitalRead(OUT_01) == ON_SIGNAL) ? PAYLOAD_ON : PAYLOAD_OFF), true);
        Homie.setNodeProperty(light02Node, "state", ((digitalRead(OUT_02) == ON_SIGNAL) ? PAYLOAD_ON : PAYLOAD_OFF), true);
        Homie.setNodeProperty(light03Node, "state", ((digitalRead(OUT_03) == ON_SIGNAL) ? PAYLOAD_ON : PAYLOAD_OFF), true);
      }
      break;
    case HOMIE_MQTT_DISCONNECTED:
      mqttDisconnected = true;
      break;
    case HOMIE_WIFI_DISCONNECTED:
      mqttDisconnected = true;
    break;
  }
}

bool light01Handler(String value) {
  return lightHandler(light01Node, OUT_01, value);
}

bool light02Handler(String value) {
  return lightHandler(light02Node, OUT_02, value);
}

bool light03Handler(String value) {
  return lightHandler(light03Node, OUT_03, value);
}

bool lightHandler(HomieNode lightNode, int lightPin, String value)
{
  if (value != PAYLOAD_ON && value != PAYLOAD_OFF) return false;

  if (value == PAYLOAD_ON) {
    digitalWrite(lightPin, ON_SIGNAL);
  } else if (value == PAYLOAD_OFF) {
    digitalWrite(lightPin, OFF_SIGNAL);
  }
  Homie.setNodeProperty(lightNode, "state", value, true);
  return true;
}

void loopInputHandler() {
  debouncer1.update();
  debouncer2.update();
  debouncer3.update();

  int input01 = debouncer1.read();
  int input02 = debouncer2.read();
  int input03 = debouncer3.read();

  if (in01LastState != input01) {
    in01LastState = input01;
    light01Handler( (digitalRead(OUT_01) == ON_SIGNAL ) ? PAYLOAD_OFF : PAYLOAD_ON );
  }
  else if (in02LastState != input02) {
    in02LastState = input02;
    light02Handler( (digitalRead(OUT_02) == ON_SIGNAL ) ? PAYLOAD_OFF : PAYLOAD_ON );
  }
  else if (in03LastState != input03) {
    in03LastState = input03;
    light03Handler( (digitalRead(OUT_03) == ON_SIGNAL ) ? PAYLOAD_OFF : PAYLOAD_ON );
  }
}

void setup() {
  Serial.begin(115200);

  //--------------------------------------------------
  //Configure OTA Update
  //--------------------------------------------------

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
  Serial.println("OTA Ready");

  //Configure Outputs
  pinMode(OUT_01, OUTPUT);
  pinMode(OUT_02, OUTPUT);
  pinMode(OUT_03, OUTPUT);

  //Setup Outputs
  digitalWrite(OUT_01, OFF_SIGNAL);
  digitalWrite(OUT_02, OFF_SIGNAL);
  digitalWrite(OUT_03, OFF_SIGNAL);

  //Configure Inpunts
  pinMode(IN_01, INPUT);
  pinMode(IN_02, INPUT);
  pinMode(IN_03, INPUT);

  //Setup Inputs
  debouncer1.attach(IN_01);
  debouncer2.attach(IN_02);
  debouncer3.attach(IN_03);
  debouncer1.interval(DEBOUNCER_INTERVAL);
  debouncer2.interval(DEBOUNCER_INTERVAL);
  debouncer3.interval(DEBOUNCER_INTERVAL);
  in01LastState = digitalRead(IN_01);
  in02LastState = digitalRead(IN_02);
  in03LastState = digitalRead(IN_03);

  //Setup Homie
  Homie.setFirmware(FW_NAME, FW_VERSION);
  light01Node.subscribe("state", light01Handler);
  light02Node.subscribe("state", light02Handler);
  light03Node.subscribe("state", light03Handler);
  Homie.registerNode(light01Node);
  Homie.registerNode(light02Node);
  Homie.registerNode(light03Node);
  Homie.onEvent(onHomieEvent);
  Homie.setup();
}

void loop() {
  ArduinoOTA.handle();
  loopInputHandler();
  Homie.loop();
}
