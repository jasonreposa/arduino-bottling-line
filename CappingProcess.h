#ifndef CAPPING_PROCESS_H
#define CAPPING_PROCESS_H

#include <elapsedMillis.h>

#include <Wire.h>
#include <IoAbstraction.h>
#include <IoAbstractionWire.h>
#include <TaskManagerIO.h>

#define CAPPING_COOL_DOWN 5000
#define CAPPER_AIR_CYLINDER_RELAY_2 A2
#define START_CAPPING_BUTTON_ADDRESS 0x20
#define START_CAPPING_BUTTON_PIN 0 // blue

// if the physical button is pressed again, emergency stop, then wait the alloted time below (ms)
#define EMERGENCY_STOP_CAPPING_COOL_DOWN 5000


class CappingProcess : public SwitchListener {
  private:
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
