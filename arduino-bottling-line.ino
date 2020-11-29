/*

*/
#include <Bounce2.h>
#include <elapsedMillis.h>

// RELAYS
#define FILLER_ENGAGEMENT_RELAY_1 A3
#define CAPPER_RELAY_2 A2
#define FILLING_RELAY_3 A1
#define CO2_RELAY_4 A0

// BUTTONS
#define START_FILLING_BUTTON 7 // red
#define START_CAPPING_BUTTON 4 // blue
// could have more buttons to separate each function of the filling proccess,
// but i think that would become tedious at some point

// the time allowed in restarting a process
#define FILLING_COOL_DOWN 5000 // make these bigger in production, or make them editable, or both
#define CAPPING_COOL_DOWN 5000
#define EMERGENCY_STOP_COOL_DOWN 5000

Bounce fillingButton = Bounce(); // pushbutton for Filling task
Bounce cappingButton = Bounce(); // pushbutton for Capping task

elapsedMillis elapsedFillingTimer;
elapsedMillis elapsedCappingTimer;

unsigned long fillerLoweringTimeInMilliseconds = 1000;
unsigned long purgingTimeInMilliseconds = 1000;
unsigned long fillingTimeInMilliseconds = 1000;
unsigned long cappingTimeInMilliseconds = 1000;

// in order:
bool startedLowering = false;
bool startedPurging = false;
bool startedFilling = false;
bool startedRaising = false;

bool startedCapping = false;

bool inFillingProcess = false;
bool inCappingProcess = false;

bool emergencyStopFillingTriggered = false;
bool emergencyStopCappingTriggered = false;

void setup() {
  while(!Serial) {}

  Serial.begin(9600);

  Serial.println("------------------------------------");
  Serial.println("START");

  // FILLING

  // air cylinder that moves filler heads up and down
  pinMode(FILLER_ENGAGEMENT_RELAY_1, OUTPUT);
  digitalWrite(FILLER_ENGAGEMENT_RELAY_1, HIGH);
  pinMode(START_FILLING_BUTTON, INPUT_PULLUP);
  digitalWrite(START_FILLING_BUTTON, HIGH);
  fillingButton.attach(START_FILLING_BUTTON);

  // water valve that turns to allow transfer of beverage to bottle
  pinMode(FILLING_RELAY_3, OUTPUT);
  digitalWrite(FILLING_RELAY_3, HIGH);
  
  // CO2 gas valve that turns to allow purging of oxygen from bottles by filling them with CO2 gas
  pinMode(CO2_RELAY_4, OUTPUT);
  digitalWrite(CO2_RELAY_4, HIGH);

  // CAPPING
  pinMode(CAPPER_RELAY_2, OUTPUT);
  digitalWrite(CAPPER_RELAY_2, HIGH);
  pinMode(START_CAPPING_BUTTON, INPUT_PULLUP);
  digitalWrite(START_CAPPING_BUTTON, HIGH);
  cappingButton.attach(START_CAPPING_BUTTON);
}

void startFillingProcess() {  
  // we've started the filling task
  inFillingProcess = true;

  // reset the elapsed time
  elapsedFillingTimer = 0;

  // so start by lowering the filler heads
  startLowering();
}

void startLowering() {
  startedLowering = true;

  // start lowering
  digitalWrite(FILLER_ENGAGEMENT_RELAY_1, LOW);
  
  Serial.println("------------------------------------");
  Serial.print("Start Lowering Filler for "); Serial.print(fillerLoweringTimeInMilliseconds); Serial.println(" ms");
}

void startPurging() {
  startedPurging = true;

  // turn on the CO2 gas valve to purge the oxygen from the bottle
  digitalWrite(CO2_RELAY_4, LOW);

  Serial.println("------------------------------------");
  Serial.print("Start CO2 Purging for "); Serial.print(purgingTimeInMilliseconds); Serial.println(" ms");
}

void startFilling() {
  // we've started the beverage filling
  startedFilling = true;

  // turn on the water valve to release the beverage
  digitalWrite(FILLING_RELAY_3, LOW);

  Serial.println("------------------------------------");
  Serial.print("Start Beverage Filling for "); Serial.print(fillingTimeInMilliseconds); Serial.println(" ms");
}

void stopPurging() {
  startedPurging = false;

  // turn off the CO2 gas valve to stop the flow of the CO2
  digitalWrite(CO2_RELAY_4, HIGH);
  Serial.println("DONE PURGING OXYGEN!");
  Serial.println("------------------------------------");
}

void stopFilling() {
  startedFilling = false;

  // turn off the water valve to stop the flow of the beverage
  digitalWrite(FILLING_RELAY_3, HIGH);
  Serial.println("DONE BEVERAGE FILLING!");
  Serial.println("------------------------------------");
}

void startRaising() {
  startedRaising = true;

  digitalWrite(FILLER_ENGAGEMENT_RELAY_1, HIGH);
  Serial.println("DONE FILLING PROCESS! Entering cool down.");
  Serial.println("------------------------------------");
}

// after the cooldown period we reset everything
void fillingReset() {
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

void monitorFillingProcess() {
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

void emergencyStopFilling() {
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

void startCappingProcess() {
  // we've started the capping task
  inCappingProcess = true;

  // reset the elapsed time
  elapsedCappingTimer = 0;

  // so start capping!
  startCapping();
}

void startCapping() {
  startedCapping = true;

  // turn on the solenoid that will lower the capping heads
  digitalWrite(CAPPER_RELAY_2, LOW);

  Serial.println("------------------------------------");
  Serial.print("Start Capping for "); Serial.print(cappingTimeInMilliseconds); Serial.println(" ms");
}

void stopCapping() {
  startedCapping = false;

  // raise capper heads
  digitalWrite(CAPPER_RELAY_2, HIGH);
  Serial.println("DONE CAPPING! Entering cool down.");
  Serial.println("------------------------------------");
}

// after the cooldown period we reset everything
void cappingReset() {
  inCappingProcess = false;

  // turn all the flags back off now that we're done the process
  startedCapping = false;

  emergencyStopCappingTriggered = false;

  Serial.println("CAPPING COOL.");
  Serial.println("------------------------------------");
}

void monitorCappingProcess() {
  // determine, in reverse timing, which event to fire

  // if we've gone past the user controlled...
  // cappingTime and cool down, allow the user to press the button again
  if (elapsedCappingTimer > cappingTimeInMilliseconds + CAPPING_COOL_DOWN) {
    cappingReset();

  // cappingTime, stop capping
  } else if (startedCapping && elapsedCappingTimer > cappingTimeInMilliseconds) {
    stopCapping();

  // we're still in the middle of the capping process
  } else if (startedCapping) {
    Serial.print("Capping: "); Serial.println(elapsedCappingTimer);
  }
}

void emergencyStopCapping() {
  Serial.println("EMERGENCY STOP CAPPING!");
  Serial.println("------------------------------------");
  stopCapping();

  // kick us out of the process
  inCappingProcess = false;
  emergencyStopCappingTriggered = true;

  // reset the elapsed time
  elapsedCappingTimer = 0;
}

void loop() { 
  // FILLING
  fillingButton.update();

  // cool down emergency
  if (emergencyStopFillingTriggered && elapsedFillingTimer > EMERGENCY_STOP_COOL_DOWN) {
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

  // CAPPING
  cappingButton.update();

  // cool down emergency
  if (emergencyStopCappingTriggered && elapsedCappingTimer > EMERGENCY_STOP_COOL_DOWN) {
    cappingReset();
  }

  if (inCappingProcess) {
    if (cappingButton.fell()) {
      emergencyStopCapping();
    } else {
      monitorCappingProcess();
    }
  } else if (cappingButton.fell() && !emergencyStopCappingTriggered) {
    // if we haven't started capping, but the user pressed the cappingButton, start capping
    // unless it was pressed due to emergency, then don't start the process again
    startCappingProcess();
  }
}
