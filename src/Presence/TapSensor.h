// TapSensor Class
// Author: Chip McClelland
// Date: May 2024
// License: GPL3
// In this class, we are using a sensor to determine whether a space is occupied or not (binary)

#ifndef __TAPSENSOR_H
#define __TAPSENOSR_H

#include <arduino.h>
#include <ArduinoLog.h>
#include "MyData.h"
#include "Config.h"
#include "stsLED.h"
#include "ErrorCodes.h"
#include "stsLED.h"
#include "ModMMA8452Q.h"


/**
 * This class is a singleton; you do not create one as a global, on the stack, or with new.
 * 
 * From global application setup you must call:
 * TapSensor::instance().setup();
 * 
 * From global application loop you must call:
 * TapSensor::instance().loop();
 */
class TapSensor {
public:
    /**
     * @brief Gets the singleton instance of this class, allocating it if necessary
     * 
     * Use TapSensor::instance() to instantiate the singleton.
     */
    static TapSensor &instance();

    /**
     * @brief Perform setup operations; call this from global application setup()
     * 
     * You typically use TapSensor::instance().setup();
     */
    bool setup();

    /**
     * @brief Perform application loop operations; call this from global application loop()
     * 
     * You typically use TapSensor::instance().loop();
     */
    bool loop();

    /**
     * @brief Call this to clear the interrupt once a tap is detected
    */
    void clearTapInts();

protected:
    /**
     * @brief The constructor is protected because the class is a singleton
     * 
     * Use TapSensor::instance() to instantiate the singleton.
     */
    TapSensor();

    /**
     * @brief The destructor is protected because the class is a singleton and cannot be deleted
     */
    virtual ~TapSensor();

    /**
     * This class is a singleton and cannot be copied
     */
    TapSensor(const TapSensor&) = delete;

    /**
     * This class is a singleton and cannot be copied
     */
    TapSensor& operator=(const TapSensor&) = delete;

    /**
     * @brief Singleton instance of this class
     * 
     * The object pointer to this class is stored here. It's NULL at system boot.
     */
    static TapSensor *_instance;

    int count = 0;

};
#endif  /* __TapSensor_H */
