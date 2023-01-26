///////////////////////// FILE HEADER  ////////////////////////////////
//
// Title: Arduino Nano GPIO expander for home security.
//
// Author: Kian Soheili
//
// Date: 01/24/2023
//
///////////////////////// OUTSIDE RESOURCES //////////////////////////
//
//
///////////////////////////////////////////////////////////////////////

//ENSURE COMPILER IS SET TO NANO FOR THIS CODE OR PROPER BOARD

//Include wire library to transmit extra GPIO data to ESP through I2C
#include <Wire.h>
//Setup variables for monitoring
#define front_door 2
#define garage_door 3
#define office_door 4
#define ally_door 5
#define playroom_door 6
#define playroom_window_L 7
#define playroom_window_R 8
#define front_window_L 9
#define front_window_R 10
#define office_window 11
#define motion_living 13
#define motion_playroom A0
#define motion_stairs A1
#define extra_1 12
#define extra_2 A2
#define extra_3 A7
//Output trigger
#define G_DOOR A3

//Set char array to send data to other Arduino.
char str[16] = "";

//Setup
void setup() {
  //Start wire with address 8
  Wire.begin(8);                /* join i2c bus with address 8 */
  Wire.onRequest(requestEvent); /* register request event */

  //Set Input Pins
  pinMode(front_door, INPUT);
  pinMode(garage_door, INPUT);
  pinMode(office_door, INPUT);
  pinMode(ally_door, INPUT);
  pinMode(playroom_door, INPUT);
  pinMode(playroom_window_L, INPUT);
  pinMode(playroom_window_R, INPUT);
  pinMode(front_window_L, INPUT);
  pinMode(front_window_R, INPUT);
  pinMode(office_window, INPUT);
  pinMode(motion_living, INPUT);
  pinMode(motion_playroom, INPUT);
  pinMode(motion_stairs, INPUT);
  pinMode(extra_1, INPUT);
  pinMode(extra_2, INPUT);
  pinMode(extra_3, INPUT);
  //Set output for opening door.
  pinMode(G_DOOR, OUTPUT);
  digitalWrite(G_DOOR,LOW);
    
  
}


//Loop
void loop() {
  //Boolean array to capture all values directly from pin read method.
  boolean data[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  //Digital read all the pins.
  data[0] = digitalRead(front_door);
  data[1] = digitalRead(garage_door);
  data[2] = digitalRead(office_door);
  data[3] = digitalRead(ally_door);
  data[4] = digitalRead(playroom_door);
  data[5] = digitalRead(playroom_window_L);
  data[6] = digitalRead(playroom_window_R);
  data[7] = digitalRead(front_window_L);
  data[8] = digitalRead(front_window_R);
  data[9] = digitalRead(office_window);
  //Motion sensors do not 'trip' like magnetic ones, they go 'High' when
  //there is motion, to fix this in the program we flip the sign. 
  data[10] = !digitalRead(motion_living);
  data[11] = !digitalRead(motion_playroom);
  data[12] = !digitalRead(motion_stairs);
  data[13] = digitalRead(extra_1);
  data[14] = digitalRead(extra_2);
  data[15] = digitalRead(extra_3);
  //Small delay
  delay(100);
  //16 unused
  str[16] = "";
  //If G_Door pin, which is hardwired to ESP is triggered, then the nano 
  //simulates a garage door open click.
  if(data[13]){
    digitalWrite(G_DOOR,HIGH);
    delay(400);
    digitalWrite(G_DOOR,LOW);
  }
  //Iterate through boolean, and convert to char.
  for (int i =0; i<sizeof(data); i++){
    if(data[i]){
      str[i]='1';
    }else{
      str[i]='0';
    }
  }
}


//Function that executes whenever data is requested from ESP, writes all the charvals.
void requestEvent() {
  Wire.write(str,sizeof(str));
}
