#ifndef FILLING_PROCESS_H
#define FILLING_PROCESS_H

#include <Bounce2.h>
#include <elapsedMillis.h>

// the time allowed in restarting a process
#define FILLING_COOL_DOWN 5000

// Air cylinder - relay for lowering filling heads
#define FILLER_AIR_CYLINDER_RELAY_1 A3
// Water solenoid - relay for beverage filling
#define BEVERAGE_FILLING_RELAY_3 A1
// CO2 solenoid - relay for purging bottles of CO2
#define CO2_PURGING_RELAY_4 A0
// physical button to start the filling process
#define START_FILLING_BUTTON 9 // red

// if the physical button is pressed again, emergency stop, then wait the alloted time below (ms)
#define EMERGENCY_STOP_FILLING_COOL_DOWN 5000


class FillingProcess {
  private:
    Bounce fillingButton;
    elapsedMillis elapsedFillingTimer;

    unsigned long fillerLoweringTimeInMilliseconds = 1000;
    unsigned long purgingTimeInMilliseconds = 1000;
    unsigned long fillingTimeInMilliseconds = 1000;


    // in order:
    bool startedLowering = false;
    bool startedPurging = false;
    bool startedFilling = false;
    bool startedRaising = false;

    bool inFillingProcess = false;

    bool emergencyStopFillingTriggered = false;

    void startFillingProcess();
    void startLowering();
    void startPurging();
    void startFilling();
    void stopPurging();
    void stopFilling();
    void startRaising();
    void fillingReset();
    void monitorFillingProcess();
    void emergencyStopFilling();
  public:
    FillingProcess();
    void setup();
    void loop();
};

#endif