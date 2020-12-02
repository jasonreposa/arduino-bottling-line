#include "CappingProcess.h"

CappingProcess::CappingProcess() {
  ioExpander = ioFrom8574(0x20);
}

void CappingProcess::onPressed(uint8_t pin, bool held) {
  // we wait to see if the operator is holding down the button
  // to make sure we really should be doing the task
  if (!emergencyStopCappingTriggered && held) {
    if (inCappingProcess) {
      emergencyStopCapping();
    } else {
      startCappingProcess();
    }
  }
}

// when a key is released this is called.
void CappingProcess::onReleased(uint8_t pin, bool held) {
  /* nothing */
}

void CappingProcess::setup() {
  while(!Serial);
  Serial.begin(9600);

  Wire.begin();
  switches.initialise(ioFrom8574(0x20, 0), true); // pull up logic is optional, defaults to PULL_DOWN buttons.
  switches.addSwitchListener(0, this);

  pinMode(CAPPER_AIR_CYLINDER_RELAY_2, OUTPUT);
  digitalWrite(CAPPER_AIR_CYLINDER_RELAY_2, HIGH);
  pinMode(START_CAPPING_BUTTON, INPUT_PULLUP);
  digitalWrite(START_CAPPING_BUTTON, HIGH);
}

void CappingProcess::loop() {
  taskManager.runLoop();

  // cool down emergency
  if (emergencyStopCappingTriggered && elapsedCappingTimer > EMERGENCY_STOP_CAPPING_COOL_DOWN) {
    cappingReset();
  }

  if (inCappingProcess) {
    monitorCappingProcess();
  }
}

void CappingProcess::startCappingProcess() {
  // we've started the capping task
  inCappingProcess = true;

  // reset the elapsed time
  elapsedCappingTimer = 0;

  // so start capping!
  startCapping();
}

void CappingProcess::startCapping() {
  startedCapping = true;

  // turn on the solenoid that will lower the capping heads
  digitalWrite(CAPPER_AIR_CYLINDER_RELAY_2, LOW);

  Serial.println("------------------------------------");
  Serial.print("Start Capping for "); Serial.print(cappingTimeInMilliseconds); Serial.println(" ms");
}

void CappingProcess::stopCapping() {
  startedCapping = false;

  // raise capper heads
  digitalWrite(CAPPER_AIR_CYLINDER_RELAY_2, HIGH);
  Serial.println("DONE CAPPING! Entering cool down.");
  Serial.println("------------------------------------");
}

// after the cooldown period we reset everything
void CappingProcess::cappingReset() {
  inCappingProcess = false;

  // turn all the flags back off now that we're done the process
  startedCapping = false;

  emergencyStopCappingTriggered = false;

  Serial.println("CAPPING COOL.");
  Serial.println("------------------------------------");
}

void CappingProcess::monitorCappingProcess() {
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

void CappingProcess::emergencyStopCapping() {
  Serial.println("EMERGENCY STOP CAPPING!");
  Serial.println("------------------------------------");
  stopCapping();

  // kick us out of the process
  inCappingProcess = false;
  emergencyStopCappingTriggered = true;

  // reset the elapsed time
  elapsedCappingTimer = 0;
}
