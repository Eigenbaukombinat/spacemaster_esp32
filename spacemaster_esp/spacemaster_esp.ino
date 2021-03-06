#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "config.h"

////////////////////////////////////////////////////
//                                                //
//   DON'T FORGET TO EDIT YOUR WIFI CREDENTIALS:  //
//   > cp config.h.example config.h               //
//   > vim config.h                               //
//                                                //
////////////////////////////////////////////////////

//*** MQTT ***
WiFiClient espClient;
PubSubClient client(espClient);

// *** Switch Status ***
int spaceSwitchState = 1;
int lastSpaceSwitchState = 1;
// *** Door Status ***
int doorSwitchState = 1;
int lastDoorSwitchState = 1;

// *** Autoopen State ***
int autoopenStatus = 0;

// *** define the GPIO Pins Door***
int spaceSwitchPin = 3;  //D9 (RX)
int doorSwitchPin = 4;   //D2

// *** define the GPIO Autoopen PIN ***
int RelaisPin = 0;      //D3
int ldrPin = A0;        //A0
int autoopenPin = 14;   //D5

// *** count Doorbell ***
int count = 0;


void setup() {
  WiFi.hostname(hname);
  Serial.begin(115200);
  while (!Serial) {   //waiting for the serial connection
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  client.publish(topicdebugwifi, "reconnect" + WiFi.localIP());
  Serial.println("");
  Serial.println("Wifi connected");
  Serial.print("IP address ");
  Serial.println(WiFi.localIP());

  // *** Space and Door Switch ***
  digitalWrite(RelaisPin, LOW);
  pinMode(spaceSwitchPin, INPUT_PULLUP);
  pinMode(doorSwitchPin, INPUT_PULLUP);
  pinMode(ldrPin, INPUT_PULLUP);
  pinMode(RelaisPin, OUTPUT);
  pinMode(autoopenPin, INPUT_PULLUP);

  // *** MQTT ***
  client.setServer(mqtt_server, 1883);
}

// *** MQTT reconnect ***
void reconnect() {    // Loop until we're reconnected
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");    // Attempt to connect
    if (client.connect(hname)) {
      Serial.println("connected");                    // Once connected, publish an announcement...  
      client.publish(topicdebugmqtt, "reconnect");    // ... and resubscribe
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again next Time");         // Wait before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // *** CHECK SPACESWITCH ***
  
  spaceSwitchState = digitalRead(spaceSwitchPin);
  doorSwitchState = digitalRead(doorSwitchPin);
  if (doorSwitchState != lastDoorSwitchState) {              //Statusabgleich zum letzen Durchlauf
    if (doorSwitchState == 0) { // tür abgeschlossen
      String spacestatus = "closed";
      Serial.println(spacestatus);
      client.publish(topicopen, "false"); //MQTT Space closed
      client.publish(topicdoor, "lock"); //MQTT tür abgeschlossen 
    }
    if (doorSwitchState == 1) {//MQTT tür unabgeschlossen
      client.publish(topicdoor, "unlock"); //MQTT tür offen 
    }
  }
   if (spaceSwitchState != lastSpaceSwitchState) {
    if (spaceSwitchState == 1  ) {  //schalter aus
      String spacestatus = "closed";
      client.publish(topicopen, "false"); //MQTT Space closed
      Serial.println(spacestatus);
      Serial.println(count);
      String countstr = String(count);
      char attributes[100];
      countstr.toCharArray(attributes, 100);
      client.publish(topicklingelclose, attributes);
      count = 0;   // reset counter on Switchstate
    }
    if ((spaceSwitchState == 0)  && (doorSwitchState == 1)) { // schalter ein und tür unabgeschlossen
        String spacestatus = "open";
        Serial.println(spacestatus);
        Serial.println(count);
        String countstr = String(count);
        client.publish(topicopen, "true");   //MQTT Space open
        char attributes[100];
        countstr.toCharArray(attributes, 100);
        client.publish(topicklingelopen, attributes);
        count = 0;   // reset counter on Switchstate
    }
  
  }
    delay(500);                                        //delay 
    lastSpaceSwitchState = spaceSwitchState;
    lastDoorSwitchState = doorSwitchState;
  
 Serial.println("####START");
 Serial.print("spaceSwitchState = " );
 Serial.println(spaceSwitchState);
 Serial.print("doorSwitchState = " );
 Serial.println(doorSwitchState);
 
 // *** CHECK AUTOOPENSWITCH ***
 
  sensorWert = analogRead(ldrPin);
  autoopenStatus = digitalRead(autoopenPin);
  Serial.print("autoopenStatus = " );
  Serial.println(autoopenStatus);
  Serial.print("Sensorwert = " );
  Serial.println(sensorWert);
  Serial.print("Count = " );
  Serial.println(count);
  Serial.println("####END");
  digitalWrite(RelaisPin, LOW);

  if ((sensorWert > schwellWert) && (autoopenStatus == 0) && (spaceSwitchState == 0) && (doorSwitchState == 1)){    // autoopen ein und tür unabgeschlossen und schalter ein
    delay(1500);
    digitalWrite(RelaisPin, HIGH);
    count = count +1;  // count autoopen on Spaceopen
    client.publish(topicklingel, "Klingel-OPEN");  //MQTT - TÜRKLINGEL OFFEN
    delay(3500);
    digitalWrite(RelaisPin, LOW);
    Serial.println("####GOING-COOL-DOWN");
    delay(86000);  // Cooldown ca. 1.3min     83000
  }
  else {
    digitalWrite(RelaisPin, LOW);
  }
  if ((sensorWert > schwellWert) && (autoopenStatus == 0) && (spaceSwitchState == 1) ){
    count = count +1;  // count autoopen on Spaceclosed
      client.publish(topicklingel, "Klingel-CLOSE");   //MQTT - TÜRKLINGEL ZU
    Serial.println("####GOING-COOL-DOWN");
    delay(86000);  // Cooldown ca. 1.3min
  }
  delay(1000);     
}
