#ifndef ROTARY_ENCODER_PCF8574_H
#define ROTARY_ENCODER_PCF8574_H

#include <IoAbstraction.h>

/** 
 * The signature for a callback function that is registered with addSwitch
 * @param key the pin associated with the pin
 * @param heldDown if the button has been held down
 */ 
typedef void(*TimeCallbackFn)(uint32_t newTime);

class RotaryEncoderPCF8574 {
  private:
    uint8_t currentStateCLK;
    uint8_t currentStateDT;
    uint8_t lastStateCLK;
    String currentDir = "";
    unsigned long lastButtonPress = 0;
    uint32_t lowerBound = 0;
    uint32_t upperBound = 100;

    // in tenths of a second
    uint32_t counter = 10;  // one second
    uint8_t resolution = 5;  // half a second

    uint8_t sw;
    uint8_t dt;
    uint8_t clk;
    IoAbstractionRef ioExpander;
    TimeCallbackFn callback;

    void normalizeBounds();
  public:
    RotaryEncoderPCF8574(IoAbstractionRef /* ioExpander */, uint8_t /* sw */, uint8_t /* dt */, uint8_t /* clk */);
    void setup(uint32_t /* counterTimeValueFromEEPROM */);
    void loop();
    void setBounds(uint32_t /* lowerBound */, uint32_t /* upperBound */);
    void setCallback(TimeCallbackFn /* callback */);
  };

#endif
