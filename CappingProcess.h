#ifndef CAPPING_PROCESS_H
#define CAPPING_PROCESS_H

#include <Bounce2.h>
#include <elapsedMillis.h>

#include <Wire.h>
#include <IoAbstraction.h>
#include <IoAbstractionWire.h>
#include <TaskManagerIO.h>

#define CAPPING_COOL_DOWN 5000
#define CAPPER_AIR_CYLINDER_RELAY_2 A2
#define START_CAPPING_BUTTON 8 // blue
#define EMERGENCY_STOP_CAPPING_COOL_DOWN 5000


class CappingProcess : public SwitchListener {
  private:
    Bounce cappingButton;

    // to handle button presses
    IoAbstractionRef ioExpander;

    elapsedMillis elapsedCappingTimer;
    unsigned long cappingTimeInMilliseconds = 1000;
    bool startedCapping = false;
    bool inCappingProcess = false;
    bool emergencyStopCappingTriggered = false;

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

    // SwitchListener callbacks
    void onPressed(uint8_t, bool);
    void onReleased(uint8_t, bool);
};

#endif
