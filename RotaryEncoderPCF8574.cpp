#include "RotaryEncoderPCF8574.h"

RotaryEncoderPCF8574::RotaryEncoderPCF8574(IoAbstractionRef _ioExpander, uint8_t _sw, uint8_t _dt, uint8_t _clk) {
  ioExpander = _ioExpander;
  sw = _sw;
  dt = _dt;
  clk = _clk;
}

void RotaryEncoderPCF8574::setBounds(uint32_t _lowerBound, uint32_t _upperBound) {
  lowerBound = _lowerBound;
  upperBound = _upperBound;  
  normalizeBounds();
}

void RotaryEncoderPCF8574::setCallback(TimeCallbackFn _callback) {
  callback = _callback;
}

void RotaryEncoderPCF8574::normalizeBounds() {
  // normalize by putting in a range
  if (counter > upperBound) {
    counter = upperBound;
  } else if (counter < lowerBound) {
    counter = lowerBound;
  }
}

void RotaryEncoderPCF8574::setup(uint32_t counterTimeValueFromEEPROM) {
  ioDevicePinMode(ioExpander, sw, INPUT_PULLUP);
  ioDevicePinMode(ioExpander, dt, INPUT_PULLUP);
  ioDevicePinMode(ioExpander, clk, INPUT_PULLUP);

  ioDeviceSync(ioExpander);
  lastStateCLK = ioDeviceDigitalRead(ioExpander, clk);

  counter = counterTimeValueFromEEPROM;
  normalizeBounds();
}

// Goes through the rotary encoders and buttons
void RotaryEncoderPCF8574::loop() {
  ioDeviceSync(ioExpander);

  uint8_t read1 = ioDeviceDigitalRead(ioExpander, sw);
  uint8_t read2 = ioDeviceDigitalRead(ioExpander, dt);
  uint8_t read3 = ioDeviceDigitalRead(ioExpander, clk);

   // Read the current state of CLK
   currentStateCLK = read2;//digitalRead(CLK);
   currentStateDT = read3; //digitalRead(DT);

   // If last and current state of CLK are different, then pulse occurred
   // React to only 1 state change to avoid double count
   if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){

     // If the DT state is different than the CLK state then
     // the encoder is rotating CCW so decrement
     if (currentStateDT != currentStateCLK) {
       counter -= resolution;
       currentDir ="CCW";
     } else {
       // Encoder is rotating CW so increment
       counter += resolution;
       currentDir ="CW";
     }

     normalizeBounds();

     Serial.print("Counter time: "); Serial.println(counter);

 //    Serial.print("Direction: ");
 //    Serial.print(currentDir);
 //    Serial.print(" | Counter: ");
 //    Serial.println(counter);

     callback(counter);
   }

   // Remember last CLK state
   lastStateCLK = currentStateCLK;

   // Read the button state
   uint8_t btnState = read1;//digitalRead(SW);

   //If we detect LOW signal, button is pressed
   if (btnState == LOW) {
     //if 50ms have passed since last LOW pulse, it means that the
     //button has been pressed, released and pressed again
     if (millis() - lastButtonPress > 50) {
       Serial.println("Button pressed!");
     }

     // Remember last button press event
     lastButtonPress = millis();
   }

//   delay(1);
 }
