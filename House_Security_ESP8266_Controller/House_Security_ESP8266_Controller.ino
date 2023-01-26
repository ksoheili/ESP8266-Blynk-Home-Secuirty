///////////////////////// FILE HEADER  ////////////////////////////////
//
// Title: Arduino ESP8266 home security connector with Blynk IoT.
//
// Author: Kian Soheili
//
// Date: 01/24/2023
//
///////////////////////// OUTSIDE RESOURCES //////////////////////////
// IFTT
// Blynk.co
///////////////////////////////////////////////////////////////////////

//ENSURE BOARD IS SET TO ESP8266 GENERIC

/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example runs directly on ESP8266 chip.

  Note: This requires ESP8266 support package:
    https://github.com/esp8266/Arduino

  Please be sure to select the right ESP8266 module
  in the Tools -> Board menu!

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/

//Gather libs, don't change the order
#include <Wire.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WidgetRTC.h>

//Definitions
#define BLYNK_PRINT Serial
//Auth tokens
#define BLYNK_TEMPLATE_ID "####"
#define BLYNK_DEVICE_NAME "Home Security System"
#define BLYNK_AUTH_TOKEN "####"
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = BLYNK_AUTH_TOKEN;
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "####";
char pass[] = "####";

#define LED_PIN 16
//Arming code, set your own
#define PASS_KE "####"
//Set output pin to control garage bit on the nano
#define G_PIN 12

//Initialize rtc widget to get time in future. 
WidgetRTC rtc;
//Setup Blynk widgets 
WidgetLED led_fault(V16); //register to virtual pin 1
WidgetLED ledstai_m(V9); //register to virtual pin 1
//Array of virtual Pins needed
int pins[] = {V0, V1, V2, V3, V4, V5, V6, V7, V8, V9, V10, V11, V12};
//Array of names mapped to virttual pins
String names[] = {"PlayWinR", "AllyDoor", "OfficeDoor", "GarageDoor", 
                  "PlayWinL", "FrontWinR", "FrontDoor", "FrontWinL", 
                  "PlayRoomDoor", "OfficeWin", "LivingMotion", 
                   "PlayMotion", "StairsMotion"};
//Switch array                   
boolean data[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//Fault list array
String fault_lst[16] = {""};
//Flags
int armed = 0;
int pschck = 0;
long errtime = 0;
boolean ff = false;

void setup()
{
  //Wire between two devices to get data string
  Wire.begin(5, 4); /* join i2c bus with SDA=D1 and SCL=D2 of NodeMCU */
  Blynk.begin(auth, ssid, pass);

  //Initialize pins for data output
  pinMode(LED_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  digitalWrite(G_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  //Set garage door stattus label
  Blynk.setProperty(V3, "color", "#F28123");
  Blynk.setProperty(V3, "label", "Garage Door Status");
  Blynk.virtualWrite(V3, "Closed!");
  String color = "#F28123";
  //Set all LED indicators to specified color
  Blynk.setProperty(V0, "color", color);
  Blynk.setProperty(V1, "color", color);
  Blynk.setProperty(V2, "color", color);
  Blynk.setProperty(V4, "color", color);
  Blynk.setProperty(V5, "color", color);
  Blynk.setProperty(V6, "color", color);
  Blynk.setProperty(V7, "color", color);
  Blynk.setProperty(V8, "color", color);
  Blynk.setProperty(V9, "color", color);
  Blynk.setProperty(V10, "color", color);
  Blynk.setProperty(V11, "color", color);
  Blynk.setProperty(V12, "color", color);
  Blynk.setProperty(V14, "color", "#38726C");
  Blynk.setProperty(V17, "color", "#563F1B");
  Blynk.setProperty(V15, "color", color);
  //Start time widget with sync interval
  rtc.begin();
  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)
}




void loop()
{
  //Always make sure garage door pin does not somehow get triggered for safety.
  digitalWrite(LED_PIN, LOW);
  digitalWrite(G_PIN, LOW);
  //Setup blynk startt
  Blynk.run();

  //data retrieval of door states
  Wire.beginTransmission(8); // begin with device address 8 
  Wire.write("g");  // Send trigger char 
  Wire.endTransmission();    // stop transmitting 
  Wire.requestFrom(8, 16); // request & read data of size 16
  //counter to 0
  int i = 0;
  //While each bit is received, set the respective virtual pin in 
  //the vpin array at the start of file. Both devices pin mapping must
  //be aligned for this to work properly, or rename labels in blynk. 
  while (Wire.available()) {
    char c = Wire.read();
    if (i < 13) {
      if (i == 3) {
        //Special case, garage door has a text output unlike LEDS
        if (c == '1') {
          Blynk.setProperty(pins[i], "color", "#F28123");
          Blynk.virtualWrite(pins[i], "GARAGE CLOSED!");
          data[i] = true;
        } else {
          Blynk.setProperty(pins[i], "color", "#D34E24");
          Blynk.virtualWrite(pins[i], "GARAGE OPENED!");
          data[i] = false;
        }
      } else {
        WidgetLED led(pins[i]);
        if (c == '1') {
          Blynk.setProperty(pins[i], "color", "#F7F052");
          led.on();
          data[i] = true;
        } else {
          Blynk.setProperty(pins[i], "color", "#D34E24");
          led.on();
          data[i] = false;
        }
      }
    }
    i++;
  }
  //Armed check
  if (armed == 1) {
    Blynk.setProperty(V13, "color", "#D34E24");
    Blynk.setProperty(V15, "color", "#D34E24");
    Blynk.virtualWrite(V13, "SYS ARMED!");
    if (alllock()) {
      //no warning.
      led_fault.off();
    } else {
      //Warning fault
      led_fault.on();
    }
  } else {
    //Disarmed
    led_fault.off();
    Blynk.setProperty(V13, "color", "#F28123");
    Blynk.setProperty(V15, "color", "#F28123");
    Blynk.virtualWrite(V13, ".......");
  }
  //delay
  delay(10);
}




//Command Line
BLYNK_WRITE(V18)
{
  //Receive command string from user in app
  String value = param.asString();
  String a = param[0].asString();
  //Clear the panel each time
  Blynk.virtualWrite(V18, "clr");
  //Check that the system is not faulted while trying to be disabled
  //Check paskey and arm/disarm respectively
  if (!ff && (a == PASS_KE)) {
    if ((pschck == 1) && (armed == 0)) {
      armed = 1;
      pschck = 2;
      errtime = 0;
      Blynk.virtualWrite(V18, "System Armed Successfully\n");
    }
    if ((pschck == 0) && (armed == 1)) {
      armed = 0;
      pschck = 0;
      errtime = 0;
      Blynk.virtualWrite(V18, "System Disarmed Successfully\n");
    }
  } else {
    //When disarming, there is 45 seconds to do so, otherwise it is faulted.
    if ((pschck == 0) && (armed == 0)) {
      armed = 0;
      pschck = 0;
      errtime = 0;
      Blynk.virtualWrite(V18, "No Action Taken.\n");
    }
    if (errtime == 0) {
      Blynk.virtualWrite(V18, "Error, passkey incorrect, 45 seconds to try again: \n");
      errtime = millis();
    } else if ((millis() - errtime) < 45 * 1000) {
      long t = 45000 - (millis() - errtime);
      float p = t / 1000;
      Blynk.virtualWrite(V18, "Error, passkey incorrect, " + String(p) + " seconds to try again: \n");
    }
    if ((millis() - errtime) > 45 * 1000) {
      Blynk.virtualWrite(V18, "Error too much time taken, notifying admin of obstruction. \n");
      errtime = 0;
      ff = true;
    }
  }
}



//gpio open close garage door
//This is directly controlled here, but can be passed to
//The secondary arduino as well to be triggered there. 
void opgarage() {
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(G_PIN, HIGH);
  delay(200);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(G_PIN, LOW);

}


//open close blynk
BLYNK_WRITE(V14)
{
  //If the button to open or close is clicked then toggle the door
  if (armed == 1) {
    Blynk.virtualWrite(V18, "Error, system is currently armed, please un-arm.\n");
  } else {
    opgarage();
    Blynk.virtualWrite(V18, "clr");
    Blynk.virtualWrite(V18, "Garage Door Opening... \n");
  }
}
//Simple copied method, just for gui.
BLYNK_WRITE(V17)
{
  if (armed == 1) {
    Blynk.virtualWrite(V18, "Error, system is currently armed, please un-arm.\n");
  } else {
    opgarage();
    Blynk.virtualWrite(V18, "clr");
    Blynk.virtualWrite(V18, "Garage Door Closing... \n");
  }
}


//Armed system, alllock checks the entire system to catch any faults, and returns a safe/unsafe status. 
static boolean alllock() {
  int fg = 0;
  //Iterate through the security points
  for (int i = 0; i < 13; i++) {
    //If any circuit is broken it will be false, closed circuit is high, open is low. 
    if (!data[i]) {
      //Circuit broken return false, not all locked
      //Use time widget to get real time for printing to user app
      Blynk.virtualWrite(V18, "Fault detected in system at " + names[i] + " " + (String(hour()) + ":" + minute() + ":" + second()) + "\n");
      //Short code that connects to IFTT to send text to phone number with message. 
      if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
        WiFiClient client;
        HTTPClient http;
        //IFTTT API Used with HTTTP requests, and voice phone call. 
        if (http.begin(client, "http://maker.ifttt.com/trigger/alarm/with/key/##API_KEY###?value1=" + names[i])) {  // HTTP
          int httpCode = http.GET();
          // httpCode will be negative on error
          if (httpCode > 0) {
            //success code
          }
          http.end();
        }
      }
      //Message sent, stem fault is high. 
      fg = 1;
    }
  }
  //Write new line
  Blynk.virtualWrite(V18, "");
  if (fg == 1) {
    return false;
  }
  //no breaks, no alarm to be triggered all locked
  return true;
}

//Arm System
BLYNK_WRITE(V15)
{
  //If the arm slider is flipped in the app, then we need to initialize the arming procedure, next is passcode. 
  int value = param.asInt();
  int a = param[0].asInt();
  pschck  = a;
  //Clear LCD
  Blynk.virtualWrite(V18, "clr");
  if (pschck == 1) {
    Blynk.virtualWrite(V18, "To arm, please enter 4 digit passkey: \n");
  } else {
    Blynk.virtualWrite(V18, "To disarm, please enter 4 digit passkey: \n");
  }
}
