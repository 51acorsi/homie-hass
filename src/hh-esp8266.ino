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

const String PAYLOAD_ON = "ON";
const String PAYLOAD_OFF = "OFF";

HomieNode light01Node("light1", "light");
HomieNode light02Node("light2", "light");
HomieNode light03Node("light3", "light");

int in01Status = -1;
int in02Status = -1;
int in03Status = -1;

Bounce debouncer1 = Bounce();

bool light01Handler(String value) {
   if (value == PAYLOAD_ON) {
     digitalWrite(OUT_01, HIGH);
     Homie.setNodeProperty(light01Node, "state", PAYLOAD_ON);
   } else if (value == PAYLOAD_OFF) {
     digitalWrite(OUT_01, LOW);
     Homie.setNodeProperty(light01Node, "state", PAYLOAD_OFF);
   } else {
     return false;
   }
  return true;
}

bool light02Handler(String value) {
  if (value == PAYLOAD_ON) {
    digitalWrite(OUT_02, HIGH);
    Homie.setNodeProperty(light02Node, "state", PAYLOAD_ON);
  } else if (value == PAYLOAD_OFF) {
    digitalWrite(OUT_02, LOW);
    Homie.setNodeProperty(light02Node, "state", PAYLOAD_OFF);
  } else {
    return false;
  }
 return true;
}

bool light03Handler(String value) {
  if (value == PAYLOAD_ON) {
    digitalWrite(OUT_03, HIGH);
    Homie.setNodeProperty(light03Node, "state", PAYLOAD_ON);
  } else if (value == PAYLOAD_OFF) {
    digitalWrite(OUT_03, LOW);
    Homie.setNodeProperty(light03Node, "state", PAYLOAD_OFF);
  } else {
    return false;
  }
 return true;
}

void loopInputHandler() {
  int input_01 = debouncer1.read();

  if (in01Status != input_01) {
    in01Status = input_01;
    light01Handler( digitalRead(OUT_01) ? PAYLOAD_ON : PAYLOAD_OFF );
  }
}

void setup() {
  //Configure Inpunts
  pinMode(IN_01, INPUT);
  pinMode(IN_02, INPUT);
  pinMode(IN_03, INPUT);

  //Configure Outputs
  pinMode(OUT_01, OUTPUT);
  pinMode(OUT_02, OUTPUT);
  pinMode(OUT_03, OUTPUT);

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

  //Setup Inputs
  debouncer1.attach(IN_01);
  debouncer1.interval(40);
  Homie.setLoopFunction(loopInputHandler);

  //Setup Homie
  Homie.setFirmware(FW_NAME, FW_VERSION);
  light01Node.subscribe("state", light01Handler);
  light02Node.subscribe("state", light02Handler);
  light03Node.subscribe("state", light03Handler);
  Homie.registerNode(light01Node);
  Homie.registerNode(light02Node);
  Homie.registerNode(light03Node);
  Homie.setup();
}

void loop() {
  ArduinoOTA.handle();
  Homie.loop();
}
