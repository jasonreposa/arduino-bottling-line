#include "FillingProcess.h"

FillingProcess::FillingProcess() {
  fillingButton = Bounce(); // pushbutton for Filling task
}

void FillingProcess::setup() {
  while(!Serial) {}
  Serial.begin(9600);

  // air cylinder that moves filler heads up and down
  pinMode(FILLER_AIR_CYLINDER_RELAY_1, OUTPUT);
  digitalWrite(FILLER_AIR_CYLINDER_RELAY_1, HIGH);
  pinMode(START_FILLING_BUTTON, INPUT_PULLUP);
  digitalWrite(START_FILLING_BUTTON, HIGH);
  fillingButton.attach(START_FILLING_BUTTON);

  // water valve that turns to allow transfer of beverage to bottle
  pinMode(BEVERAGE_FILLING_RELAY_3, OUTPUT);
  digitalWrite(BEVERAGE_FILLING_RELAY_3, HIGH);
  
  // CO2 gas valve that turns to allow purging of oxygen from bottles by filling them with CO2 gas
  pinMode(CO2_PURGING_RELAY_4, OUTPUT);
  digitalWrite(CO2_PURGING_RELAY_4, HIGH);
}

void FillingProcess::loop() {
  // FILLING
  fillingButton.update();

  // cool down emergency
  if (emergencyStopFillingTriggered && elapsedFillingTimer > EMERGENCY_STOP_FILLING_COOL_DOWN) {
    fillingReset();
  }

  if (inFillingProcess) {
    if (fillingButton.fell()) {
      emergencyStopFilling();
    } else {
      // continue to monitor the process
      monitorFillingProcess();
    }

  } else if (fillingButton.fell() && !emergencyStopFillingTriggered) {
    // if we haven't started filling, but the user pressed the fillingButton, start filling
    // unless it was pressed due to emergency, then don't start the process again
    startFillingProcess();
  }
}


void FillingProcess::startFillingProcess() {  
  // we've started the filling task
  inFillingProcess = true;

  // reset the elapsed time
  elapsedFillingTimer = 0;

  // so start by lowering the filler heads
  startLowering();
}

void FillingProcess::startLowering() {
  startedLowering = true;

  // start lowering
  digitalWrite(FILLER_AIR_CYLINDER_RELAY_1, LOW);
  
  Serial.println("------------------------------------");
  Serial.print("Start Lowering Filler for "); Serial.print(fillerLoweringTimeInMilliseconds); Serial.println(" ms");
}

void FillingProcess::startPurging() {
  startedPurging = true;

  // turn on the CO2 gas valve to purge the oxygen from the bottle
  digitalWrite(CO2_PURGING_RELAY_4, LOW);

  Serial.println("------------------------------------");
  Serial.print("Start CO2 Purging for "); Serial.print(purgingTimeInMilliseconds); Serial.println(" ms");
}

void FillingProcess::startFilling() {
  // we've started the beverage filling
  startedFilling = true;

  // turn on the water valve to release the beverage
  digitalWrite(BEVERAGE_FILLING_RELAY_3, LOW);

  Serial.println("------------------------------------");
  Serial.print("Start Beverage Filling for "); Serial.print(fillingTimeInMilliseconds); Serial.println(" ms");
}

void FillingProcess::stopPurging() {
  startedPurging = false;

  // turn off the CO2 gas valve to stop the flow of the CO2
  digitalWrite(CO2_PURGING_RELAY_4, HIGH);
  Serial.println("DONE PURGING OXYGEN!");
  Serial.println("------------------------------------");
}

void FillingProcess::stopFilling() {
  startedFilling = false;

  // turn off the water valve to stop the flow of the beverage
  digitalWrite(BEVERAGE_FILLING_RELAY_3, HIGH);
  Serial.println("DONE BEVERAGE FILLING!");
  Serial.println("------------------------------------");
}

void FillingProcess::startRaising() {
  startedRaising = true;

  digitalWrite(FILLER_AIR_CYLINDER_RELAY_1, HIGH);
  Serial.println("DONE FILLING PROCESS! Entering cool down.");
  Serial.println("------------------------------------");
}

// after the cooldown period we reset everything
void FillingProcess::fillingReset() {
  inFillingProcess = false;

  // turn all the flags back off now that we're done the process
  startedLowering = false;
  startedPurging = false;
  startedFilling = false;
  startedRaising = false;

  emergencyStopFillingTriggered = false;

  Serial.println("FILLING COOL.");
  Serial.println("------------------------------------");
}

void FillingProcess::monitorFillingProcess() {
  // determine, in reverse timing, which event to fire

  // elapsed > lowering, purging, filling and cooldown, reset
  // elapsed > lowering, purging and filling, start raising
  // elapsed > lowering and purging, start filling
  // elapsed > lowering, start purging

  // if we've gone past the user controlled...

  // elapsed > loweringTime, purgingTime, fillingTime and cool down, allow the user to press the button again
  if (elapsedFillingTimer > fillerLoweringTimeInMilliseconds + purgingTimeInMilliseconds + fillingTimeInMilliseconds + FILLING_COOL_DOWN) {
    fillingReset();

  // elapsed > loweringTime, purgingTime and fillingTime, stop filling and start raising filler heads
  } else if (elapsedFillingTimer > fillerLoweringTimeInMilliseconds + purgingTimeInMilliseconds + fillingTimeInMilliseconds) {

    // gaurd these actions so they only occur once
    if (startedFilling && !startedRaising) {
      stopFilling();
      startRaising();
    }

  // elapsed > loweringTime and purgingTime, stop purging and start filling
  } else if (elapsedFillingTimer > fillerLoweringTimeInMilliseconds + purgingTimeInMilliseconds) {

    // gaurd these actions so they only occur once
    if (startedPurging && !startedFilling) {
      stopPurging();
      startFilling();
    }

  // elapsed > loweringTime, start CO2 purge
  // which just means that loweringTime is just the amount of time to delay before opening up the ball valve to let in the CO2
  } else if (elapsedFillingTimer > fillerLoweringTimeInMilliseconds) {

    // gaurd this action so it only occurs once
    if (startedLowering && !startedPurging) {
      startPurging();
    }

  // we're still in the middle of the filling process
  } else if (!startedRaising) {
//     Serial.print("Filling - Elapsed Time: "); Serial.println(elapsedFillingTimer);
  }
}

void FillingProcess::emergencyStopFilling() {
  Serial.println("EMERGENCY STOP FILLING!");
  Serial.println("------------------------------------");

  stopPurging();
  stopFilling();
  startRaising();

  // kick us out of the process
  inFillingProcess = false;
  emergencyStopFillingTriggered = true;

  // reset the elapsed time
  elapsedFillingTimer = 0;
}