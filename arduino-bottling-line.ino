/*
  Author: Jason Reposa
  Created: 11-25-2020 (?)
*/
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
IoAbstractionRef ioExpander = ioFrom8574(SECOND_EXPANSION_BOARD_ADDRESS);

#include "RotaryEncoderPCF8574.h"
RotaryEncoderPCF8574 cappingTimeEncoder = RotaryEncoderPCF8574(ioExpander, 0, 1, 2);
RotaryEncoderPCF8574 fillingTimeEncoder = RotaryEncoderPCF8574(ioExpander, 3, 4, 5);

AvrEeprom avrEeprom;
#define CAPPING_TIME_EEPROM_ADDRESS 0
#define FILLING_TIME_EEPROM_ADDRESS 1

void onCappingStartPressed(uint8_t pin, bool heldDown) {
  cappingProcess.onStartButtonPress(heldDown);
}

void onFillingStartPressed(uint8_t pin, bool heldDown) {
  fillingProcess.onStartButtonPress(heldDown);
}

void onCappingTimeChange(uint8_t newTime) {
  Serial.print("New Capping Time: "); Serial.println(newTime);
  cappingProcess.setCappingTime(newTime);
  // write it to memory
  avrEeprom.write8(CAPPING_TIME_EEPROM_ADDRESS, newTime);
}

void onFillingTimeChange(uint8_t newTime) {
  Serial.print("New Filling Time: "); Serial.println(newTime);
  fillingProcess.setFillingTime(newTime);
  // write it to memory
  avrEeprom.write8(FILLING_TIME_EEPROM_ADDRESS, newTime);
}

void setup() {
  while(!Serial) {}

  Serial.begin(9600);

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

  uint8_t fillingTimeValue = avrEeprom.read8(FILLING_TIME_EEPROM_ADDRESS);
  if (fillingTimeValue) {
    onFillingTimeChange(fillingTimeValue);
    Serial.print("Found a filling time value: "); Serial.println(fillingTimeValue);
  }

  fillingTimeEncoder.setup(fillingTimeValue);
  fillingTimeEncoder.setBounds(2, 50);
  fillingTimeEncoder.setCallback(onFillingTimeChange);

  // CAPPING
  cappingProcess.setup();

  uint8_t cappingTimeValue = avrEeprom.read8(CAPPING_TIME_EEPROM_ADDRESS);
  if (cappingTimeValue) {
    onFillingTimeChange(cappingTimeValue);
    Serial.print("Found a capping time value: "); Serial.println(cappingTimeValue);
  }

  // rotary encoder
  cappingTimeEncoder.setup(cappingTimeValue);
  cappingTimeEncoder.setBounds(1, 25);
  cappingTimeEncoder.setCallback(onCappingTimeChange);
}

void loop() {
  taskManager.runLoop();

  // FILLING
  fillingProcess.loop();
  fillingTimeEncoder.loop();

  // CAPPING
  cappingProcess.loop();
  cappingTimeEncoder.loop();
}
