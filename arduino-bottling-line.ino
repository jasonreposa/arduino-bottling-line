/*
  Author: Jason Reposa
  Created: 11-25-2020 (?)
*/

// for buttons and rotary encoders
#include <Wire.h>
#include <IoAbstraction.h>
#include <IoAbstractionWire.h>
#include <TaskManagerIO.h>

// custom constants for expansion boards and pin layout
#define FIRST_EXPANSION_BOARD_ADDRESS 0x20
#define SECOND_EXPANSION_BOARD_ADDRESS 0x21
#define THIRD_EXPANSION_BOARD_ADDRESS 0x22

// physical buttons to start the capping and filling process
#define START_CAPPING_BUTTON_PIN 0 // first expansion board, blue button
#define START_FILLING_BUTTON_PIN 1 // first expansion board, red button

// custom libraries to support the bottling process
#include "FillingProcess.h"
FillingProcess fillingProcess = FillingProcess();

#include "CappingProcess.h"
CappingProcess cappingProcess = CappingProcess();

// ROTARY ENCODER
IoAbstractionRef ioExpander1 = ioFrom8574(SECOND_EXPANSION_BOARD_ADDRESS);
IoAbstractionRef ioExpander2 = ioFrom8574(THIRD_EXPANSION_BOARD_ADDRESS);

#include "RotaryEncoderPCF8574.h"
RotaryEncoderPCF8574 cappingTimeEncoder = RotaryEncoderPCF8574(ioExpander1, 0, 1, 2);
RotaryEncoderPCF8574 fillingTimeEncoder = RotaryEncoderPCF8574(ioExpander1, 3, 4, 5);
RotaryEncoderPCF8574 loweringTimeEncoder = RotaryEncoderPCF8574(ioExpander2, 0, 1, 2);
RotaryEncoderPCF8574 purgingTimeEncoder = RotaryEncoderPCF8574(ioExpander2, 3, 4, 5);

AvrEeprom avrEeprom;
#define CAPPING_TIME_EEPROM_ADDRESS 0
#define FILLING_TIME_EEPROM_ADDRESS 4
#define LOWERING_TIME_EEPROM_ADDRESS 8
#define PURGING_TIME_EEPROM_ADDRESS 12

void onCappingStartPressed(uint8_t pin, bool heldDown) {
  cappingProcess.onStartButtonPress(heldDown);
}

void onFillingStartPressed(uint8_t pin, bool heldDown) {
  fillingProcess.onStartButtonPress(heldDown);
}

void onCappingTimeChange(uint32_t newTime) {
  Serial.print("New Capping Time: "); Serial.println(newTime);
  cappingProcess.setCappingTime(newTime);
  // write it to memory
  avrEeprom.write16(CAPPING_TIME_EEPROM_ADDRESS, newTime);
  // update interface
  HMI_setTimer("x0", newTime);
}

void onFillingTimeChange(uint32_t newTime) {
  Serial.print("New Filling Time: "); Serial.println(newTime);
  fillingProcess.setFillingTime(newTime);
  // write it to memory
  avrEeprom.write16(FILLING_TIME_EEPROM_ADDRESS, newTime);
  // update interface
  HMI_setTimer("x1", newTime);
}

void onLoweringTimeChange(uint32_t newTime) {
  Serial.print("New Lowering Time: "); Serial.println(newTime);
  fillingProcess.setLoweringTime(newTime);
  // write it to memory
  avrEeprom.write16(LOWERING_TIME_EEPROM_ADDRESS, newTime);
  // update interface
  HMI_setTimer("x2", newTime);
}

void onPurgingTimeChange(uint32_t newTime) {
  Serial.print("New Purging Time: "); Serial.println(newTime);
  fillingProcess.setPurgingTime(newTime);
  // write it to memory
  avrEeprom.write16(PURGING_TIME_EEPROM_ADDRESS, newTime);
  // update interface
  HMI_setTimer("x3", newTime);
}

// HMI functions
void HMI_setTimer(String hmiVariable, uint32_t newCappingTime) {
  Serial1.print(hmiVariable);
  Serial1.print(".val=");
  Serial1.print(newCappingTime);
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
}

void setup() {
  // RX and TX on the pro micro
  Serial1.begin(9600);
  // wait 1 second for Serial1
  while (!Serial1 && millis() < 1000);

  // Set up a faster baud for HMI -- DOESN'T WORK
  //  Serial1.print("baud=115200");
  //  Serial1.write(0xff);
  //  Serial1.write(0xff);
  //  Serial1.write(0xff);
  //  Serial1.end();
  //  Serial1.begin(115200);

  // these will kill non-computer connected loading
  Serial.begin(9600);
  // wait another 1 second for Serial
  // if you wait forever, the program won't start unless it's connected to a serial monitor
  while (!Serial && millis() < 2000);

  Serial.println("------------------------------------");
  Serial.println("START");

  Wire.begin();

  // First we set up the switches library, giving it the task manager and tell it where the pins are located
  // the second parameter is a flag to use pull up switching, (true is pull up).
  switches.initialise(ioFrom8574(FIRST_EXPANSION_BOARD_ADDRESS, 0), true);

  switches.addSwitch(START_CAPPING_BUTTON_PIN, onCappingStartPressed);
  switches.addSwitch(START_FILLING_BUTTON_PIN, onFillingStartPressed);

  // FILLING
  fillingProcess.setup();

  // rotary encoders
  uint32_t fillingTimeValue = avrEeprom.read16(FILLING_TIME_EEPROM_ADDRESS);
  if (fillingTimeValue) {
    onFillingTimeChange(fillingTimeValue);
    Serial.print("Found a filling time value: "); Serial.println(fillingTimeValue);
  }

  // in tenths of a second - two seconds is 20
  fillingTimeEncoder.setBounds(20, 1000);
  fillingTimeEncoder.setCallback(onFillingTimeChange);
  fillingTimeEncoder.setup(fillingTimeValue);

  // Filling Process - lowering
  uint32_t loweringTimeValue = avrEeprom.read16(LOWERING_TIME_EEPROM_ADDRESS);
  if (loweringTimeValue) {
    onLoweringTimeChange(loweringTimeValue);
    Serial.print("Found a lowering time value: "); Serial.println(loweringTimeValue);
  }

  // in tenths of a second - two seconds is 20
  loweringTimeEncoder.setBounds(10, 100);
  loweringTimeEncoder.setCallback(onLoweringTimeChange);
  loweringTimeEncoder.setup(loweringTimeValue);

  // Filling Process - purging
  uint32_t purgingTimeValue = avrEeprom.read16(PURGING_TIME_EEPROM_ADDRESS);
  if (purgingTimeValue) {
    onPurgingTimeChange(purgingTimeValue);
    Serial.print("Found a purging time value: "); Serial.println(purgingTimeValue);
  }

  // in tenths of a second - two seconds is 20
  purgingTimeEncoder.setBounds(5, 50);
  purgingTimeEncoder.setCallback(onPurgingTimeChange);
  purgingTimeEncoder.setup(purgingTimeValue);

  // CAPPING
  cappingProcess.setup();

  uint32_t cappingTimeValue = avrEeprom.read16(CAPPING_TIME_EEPROM_ADDRESS);
  if (cappingTimeValue) {
    onCappingTimeChange(cappingTimeValue);
    Serial.print("Found a capping time value: "); Serial.println(cappingTimeValue);
  }

  // rotary encoder
  // in tenths of a second - one second is 10
  cappingTimeEncoder.setBounds(10, 100);
  cappingTimeEncoder.setCallback(onCappingTimeChange);
  cappingTimeEncoder.setup(cappingTimeValue);
}

void loop() {
  // 65 0 2 0 FF FF FF - Capping Start Button
  // 65 0 8 0 FF FF FF - Filling Start Button
  while (Serial1.available() > 0) {
    int serial_datum = Serial1.read();
    //    Serial.print(serial_datum);
    //    Serial.print(" - ");
    //    Serial.println(serial_datum, HEX);

    // really unsafe. the data comes over one int at a time in the loop(),
    // so if we ever have a conflict with a number like 101, 255, 0 or a
    // button with the same id, this would need to be rewritten.
    if (serial_datum == 2) {
      Serial.println("Start Capping");
      cappingProcess.onStartButtonPress(true);
    } else if (serial_datum == 8) {
      Serial.println("Start Filling");
      fillingProcess.onStartButtonPress(true);
    }
  }

  taskManager.runLoop();

  // FILLING
  fillingProcess.loop();
  fillingTimeEncoder.loop();
  loweringTimeEncoder.loop();
  purgingTimeEncoder.loop();

  // CAPPING
  cappingProcess.loop();
  cappingTimeEncoder.loop();
}
