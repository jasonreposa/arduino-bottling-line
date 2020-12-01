/*

*/
// #include <IoAbstractionWire.h>

// custom libraries to support the bottling process
#include "FillingProcess.h"
FillingProcess fillingProcess = FillingProcess();

#include "CappingProcess.h"
CappingProcess cappingProcess = CappingProcess();


// ROTARY ENCODER
#define CLK 4
#define DT 5
#define SW 6
int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir ="";
unsigned long lastButtonPress = 0;
#define CAPPING_UPPER_LIMIT_IN_SECONDS 25

// PCF8574
// IoAbstractionRef ioUsingArduino();
// IoAbstractionRef ioExpander = ioFrom8574(0x20);

void setup() {
  while(!Serial) {}

  Serial.begin(9600);

  Serial.println("------------------------------------");
  Serial.println("START");

  // FILLING
  fillingProcess.setup();

  // CAPPING
  cappingProcess.setup();

  // control length of filling and capping

  // Set encoder pins as inputs
  pinMode(CLK,INPUT);
  pinMode(DT,INPUT);
  pinMode(SW, INPUT_PULLUP);
  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);
  counter = 0;//cappingTimeInMilliseconds / 1000;

//  Wire.begin();
//  ioDevicePinMode(ioExpander, 0, INPUT);

}

//int counterToTimeInMilliseconds(counter) {
//  return counter * 1000;
//}
//
//int timeInMillisecondsToCounter(timeInMilliseconds) {
//  return timeInMilliseconds / 1000;
//}



// void checkKnobs() {
//   // Read the current state of CLK
//   currentStateCLK = digitalRead(CLK);
//
//   // If last and current state of CLK are different, then pulse occurred
//   // React to only 1 state change to avoid double count
//   if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){
//
//     // If the DT state is different than the CLK state then
//     // the encoder is rotating CCW so decrement
//     if (digitalRead(DT) != currentStateCLK) {
//       counter --;
//       currentDir ="CCW";
//     } else {
//       // Encoder is rotating CW so increment
//       counter ++;
//       currentDir ="CW";
//     }
//
//     // normalize by putting in a range
//     if (counter > CAPPING_UPPER_LIMIT_IN_SECONDS) {
//       counter = CAPPING_UPPER_LIMIT_IN_SECONDS;
//     } else if (counter < 0) {
//       counter = 0;
//     }
//
//     cappingTimeInMilliseconds = counter * 1000;
//     // map(counter, 0, CAPPING_UPPER_LIMIT_IN_SECONDS, 0, CAPPING_UPPER_LIMIT_IN_SECONDS * 1000); //Map value 0-25 to 0-100000
//     Serial.print("Capping time: "); Serial.println(cappingTimeInMilliseconds);
//
// //    Serial.print("Direction: ");
// //    Serial.print(currentDir);
// //    Serial.print(" | Counter: ");
// //    Serial.println(counter);
//   }
//
//   // Remember last CLK state
//   lastStateCLK = currentStateCLK;
//
//   // Read the button state
//   int btnState = digitalRead(SW);
//
//   //If we detect LOW signal, button is pressed
//   if (btnState == LOW) {
//     //if 50ms have passed since last LOW pulse, it means that the
//     //button has been pressed, released and pressed again
//     if (millis() - lastButtonPress > 50) {
//       Serial.println("Button pressed!");
//     }
//
//     // Remember last button press event
//     lastButtonPress = millis();
//   }
//
//   delay(1);
// }

void loop() {
  // taskManager.runLoop();
//  checkKnobs();

  fillingProcess.loop();
  cappingProcess.loop();
}
