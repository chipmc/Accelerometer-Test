#include "timing.h"

AB1805 ab1805(Wire); // Class instance for the the AB1805 RTC

timing *timing::_instance;

// [static]
timing &timing::instance() {
    if (!_instance) {
        _instance = new timing();
    }
    return *_instance;
}

timing::timing() {
}

timing::~timing() {
}

bool timing::setup() {
  // ab1805.resetConfig();
  ab1805.withFOUT(gpio.WAKE).setup();
  ab1805.setWDT(ab1805.WATCHDOG_MAX_SECONDS);

  ab1805.setRtcFromTime(1722075394+60); // Set the time to 1722075394 + 60 seconds

  bool isRTCSet = ab1805.getRtcAsTime(time_cv,hundrths_cv);
  if (isRTCSet) {
    Log.infoln("AB1805 is set to %l", time_cv);
    return true;
  }
  else {
    Log.infoln("AB1805 is not set");
    return false;
  }
}

void timing::loop() {
    // static uint32_t lastTime = 0;

    // Put your code to run during the application thread loop here
    ab1805.loop(); 

/*
    if (millis() - lastTime > 10000) {
        ab1805.getRtcAsTime(time_cv,hundrths_cv);
        lastTime = millis();
        Log.infoln("Time: %u.%u", (uint32_t)time_cv, hundrths_cv);
    }   
    */
    
}

/*******************************************************************************
 * Method Name: setTime()
 *******************************************************************************/
bool timing::setTime(time_t UnixTime, uint8_t hundredths){
  ab1805.setRtcFromTime(UnixTime,hundredths);
  if (ab1805.isRTCSet()) {
    Log.infoln("AB1805 is set to %l", UnixTime);
    return true;
  }
  else {
    Log.infoln("AB1805 is not set");
    return false;
  }
}

time_t timing::getTime() {

  time_t time_seconds;
  uint8_t hundredths;
  ab1805.getRtcAsTime(time_seconds, hundredths);

  return time_seconds;
}



/*******************************************************************************
 * Method Name: update()
 *******************************************************************************/
bool timing::update(){
 // Need to fill this in once I have the clock working

    return true;
}

/*******************************************************************************
 * Method Name: InterruptAtEvent()
 *******************************************************************************/
void timing::interruptAtEvent(uint8_t eventType){
  
 // Need to fill this in once I have the clock working

}

void timing::interruptAtTime(time_t UnixTime, uint8_t hundredths){
  ab1805.interruptAtTime(UnixTime,hundredths);
}

void timing::clearRepeatingInterrupt(){
  ab1805.clearRepeatingInterrupt();
}

void timing::stopWDT(){
  ab1805.stopWDT();
}

void timing::resumeWDT(){
  ab1805.resumeWDT();
}

void timing::setWDT(int seconds){
  ab1805.setWDT(seconds);
}

bool timing::isRTCSet(){
  return ab1805.isRTCSet();
}

void timing::deepPowerDown(uint16_t seconds){
  ab1805.deepPowerDown(seconds);
}