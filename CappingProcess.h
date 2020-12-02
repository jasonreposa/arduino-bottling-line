#ifndef CAPPING_PROCESS_H
#define CAPPING_PROCESS_H

#include <elapsedMillis.h>

#define CAPPING_COOL_DOWN 5000
#define CAPPER_AIR_CYLINDER_RELAY_2 A2

// if the physical button is pressed again, emergency stop, then wait the alloted time below (ms)
#define EMERGENCY_STOP_CAPPING_COOL_DOWN 5000


class CappingProcess {
  private:
    elapsedMillis elapsedCappingTimer;
    // sane defaults(?) - override with EEPROM
    unsigned long cappingTimeInMilliseconds = 1000;
    bool startedCapping = false;
    bool inCappingProcess = false;
    bool emergencyStopCappingTriggered = false;

    // in order of operation
    void startCappingProcess();
    void startCapping();
    void stopCapping();

    void cappingReset();
    void monitorCappingProcess();
    void emergencyStopCapping();
  public:
    CappingProcess();
    void setup();
    void loop();

    void onStartButtonPress(bool);
};

#endif
