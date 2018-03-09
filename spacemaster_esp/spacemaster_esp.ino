#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include "config.h"


////////////////////////////////////////////////////
//                                                //
//   DON'T FORGET TO EDIT YOUR WIFI CREDENTIALS:  //
//   > cp config.h.example config.h               //
//   > vim config.h                               //
//                                                //
////////////////////////////////////////////////////

// *** Switch Status ***
int spaceSwitchState = 1;
int lastSpaceSwitchState = 1;
// *** Door Status ***
int doorSwitchState = 1;

int lastDoorSwitchState = 1;

// *** define the GPIO Pins ***
int spaceSwitchPin = 3; //D9 (RX)
int doorSwitchPin = 4; //D2

// *** report to which Server ***
char * spaceapiServer = "putin.lan";

// *** HTTP POST with the actually state ***
void publishStatus(char * spaceapiServer, uint16_t port, String spacestatus) {
  String spacestatusPost = "";
  spacestatusPost = "status=" + spacestatus;
  HTTPClient http;
  http.begin(spaceapiServer, port, spacestatus);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.POST(spacestatusPost);
  http.writeToStream(&Serial);
  http.end();
}

void setup() {

  Serial.begin(115200);
  while (!Serial) {
    ; //waiting for the serial connection
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    // wait for the WiFi connection
    delay(500);
    Serial.print(".");

  }

  Serial.println("");
  Serial.println("Wifi connected");
  Serial.print("IP address ");
  Serial.println(WiFi.localIP());

  // *** Space and Door Switch ***
  pinMode(spaceSwitchPin, INPUT_PULLUP);
  pinMode(doorSwitchPin, INPUT_PULLUP);


}

void loop() {

  spaceSwitchState = digitalRead(spaceSwitchPin);
  doorSwitchState = digitalRead(doorSwitchPin);
  if ((spaceSwitchState != lastSpaceSwitchState) || (doorSwitchState != lastDoorSwitchState)) {

    if ((doorSwitchState == 0) || (spaceSwitchState == 1)) { // tür abgeschlossen oder schalter aus
      String spacestatus = "closed";
      Serial.println(spacestatus);
      publishStatus(spaceapiServer, 8888, spacestatus);
      publishStatus(spaceapiServer, 8889, spacestatus);
    } else if (spaceSwitchState == 0) { // schalter ein
      String spacestatus = "open";
      Serial.println(spacestatus);
      publishStatus(spaceapiServer, 8888, spacestatus);
      publishStatus(spaceapiServer, 8889, spacestatus);

    }
    delay(200);
    lastSpaceSwitchState = spaceSwitchState;
    lastDoorSwitchState = doorSwitchState;
  }

}
