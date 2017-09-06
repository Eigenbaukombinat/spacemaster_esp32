#include <WiFi.h>
#include <WiFiMulti.h>

#include <RotaryEncoder.h>

//#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F,16,2); //ESP32 I2C is IO21 and IO22

#include <HTTPClient.h>
// *** Switch Status ***
int spaceSwitchState = 0;
int spaceSwitchCounter = 0;
int lastSpaceSwitchState = 0;
// *** Door Status ***
int doorSwitchState = 0;
int doorSwitchCounter = 0;
int lastDoorSwitchState = 0;

// *** define the GPIO Pins ***
int spaceSwitchPin = 12; //IO12
int doorSwitchPin = 14; //IO14
// *** Rotary Encoder ***
RotaryEncoder encoder(4, 2); //IO4 and IO2
int encoderButton = 15; //IO15

#define ROTARYSTEPS 15 // 15 minutes steps
#define ROTARYMIN 0
#define ROTARYMAX 4320 // 72 hours

int n = LOW;

// *** Define the Wlan to connect ***
char ssid[] = "SSID";
const char* password = "PASSWORD";

// *** report to whitch Server ***
char * spaceapiServer = "putin.lan";

// *** HTTP POST with the actually state ***
void publishStatus(char * spaceapiServer, uint16_t port, String spacestatus){
  String spacestatusPost = "";
  spacestatusPost = "status=" + spacestatus;
  HTTPClient http;
  http.begin(spaceapiServer, port, spacestatus);
  //http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.POST(spacestatusPost);
  http.writeToStream(&Serial);
  http.end();
}

void setup() {

    Serial.begin(115200);
    while (!Serial){
      ; //waiting for the serial connection
    }
    Serial.println();
    Serial.print("SSID: ");
    Serial.println(ssid);
  
    WiFi.begin(ssid, password);

    while(WiFi.status() != WL_CONNECTED) {
      // wait for the WiFi connection
      delay(500);
      Serial.print(".");
      
    }

    lcd.backlight();
    lcd.init();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(WiFi.localIP());
    Serial.println("");
    Serial.println("Wifi connected");
    Serial.print("IP address ");
    Serial.println(WiFi.localIP());
  
  // *** Space and Door Switch ***
  pinMode(spaceSwitchPin, INPUT_PULLUP);
  pinMode(doorSwitchPin, INPUT_PULLUP);

  pinMode(encoderButton, INPUT);


}

void loop() {

  spaceSwitchState = digitalRead(spaceSwitchPin);
  doorSwitchState = digitalRead(doorSwitchPin);

  int newPos = encoder.getPosition() * ROTARYSTEPS;

  if (newPos < ROTARYMIN) {
    encoder.setPosition(ROTARYMIN / ROTARYSTEPS);
    newPos = ROTARYMIN;

  } else if (newPos > ROTARYMAX) {
    encoder.setPosition(ROTARYMAX / ROTARYSTEPS);
    newPos = ROTARYMAX;
  } // 
  
  int pos = 0;
  encoder.tick();
  if (pos != newPos) {
    int minutes = newPos;
    String mns = " Minuten       ";
    if (minutes <= 59){
      lcd.setCursor(0,1);
      String lcdMinutes = minutes + mns; 
      lcd.print(lcdMinutes);
 //     lcd.print(" Minuten ");
    }else {
      int hours = minutes / 60;
      int lcdMinutes = minutes - (hours * 60);
      String hrs = " Stunden       ";
      String sign = ":";
      String lcdHours = hours + sign + lcdMinutes + hrs;
      lcd.setCursor(0,1);
      lcd.print(lcdHours);
    }

    pos = newPos;
  }

  if(digitalRead(encoderButton) == HIGH){
    //lcd.clear();
    //lcd.home();
    delay(200);
  }
  
  if ((spaceSwitchState != lastSpaceSwitchState) || (doorSwitchState != lastDoorSwitchState)) {
    if (lastSpaceSwitchState != spaceSwitchState){
      spaceSwitchCounter++;
    }
      else{
        doorSwitchCounter++;
      }
    if((spaceSwitchState == LOW) || (doorSwitchState == LOW)){
      String spacestatus = "closed";
      Serial.println(spacestatus);
      publishStatus(spaceapiServer, 8888, spacestatus);
      publishStatus(spaceapiServer, 8889, spacestatus);
      Serial.print("Door: ");
      Serial.println(doorSwitchCounter);
      Serial.print("Space: ");
      Serial.println(spaceSwitchCounter);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Space: ");
      lcd.print(spacestatus); 
      }
          else{     
            String spacestatus = "open";
            Serial.println(spacestatus);
            publishStatus(spaceapiServer, 8888, spacestatus);
            publishStatus(spaceapiServer, 8889, spacestatus);
            Serial.print("Door: ");
            Serial.println(doorSwitchCounter);
            Serial.print("Space: ");
            Serial.println(spaceSwitchCounter);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Space: ");
            lcd.print(spacestatus);
            }
  delay(500);
  lastSpaceSwitchState = spaceSwitchState;
  lastDoorSwitchState = doorSwitchState;
  }

}
