// Presence Class
// Author: Chip McClelland
// Date: May 2024
// License: GPL3
// In this class, we are using a sensor to determine whether a space is occupied or not (binary)

#ifndef __PRESENCE_H
#define __PRESENCE_H

#include <arduino.h>
#include <ArduinoLog.h>
#include "MyData.h"
#include "Config.h"
#include "stsLED.h"
#include "ErrorCodes.h"
#include "stsLED.h"
#include "TapSensor.h"
#include "timing.h"


/**
 * This class is a singleton; you do not create one as a global, on the stack, or with new.
 * 
 * From global application setup you must call:
 * Presence::instance().setup();
 * 
 * From global application loop you must call:
 * Presence::instance().loop();
 */
class Presence {
public:
    /**
     * @brief Gets the singleton instance of this class, allocating it if necessary
     * 
     * Use Presence::instance() to instantiate the singleton.
     */
    static Presence &instance();

    /**
     * @brief Perform setup operations; call this from global application setup()
     * 
     * You typically use Presence::instance().setup();
     */
    bool setup();

    /**
     * @brief Perform application loop operations; call this from global application loop()
     * 
     * You typically use Presence::instance().loop();
     */
    bool loop();

protected:
    /**
     * @brief The constructor is protected because the class is a singleton
     * 
     * Use Presence::instance() to instantiate the singleton.
     */
    Presence();

    /**
     * @brief The destructor is protected because the class is a singleton and cannot be deleted
     */
    virtual ~Presence();

    /**
     * This class is a singleton and cannot be copied
     */
    Presence(const Presence&) = delete;

    /**
     * This class is a singleton and cannot be copied
     */
    Presence& operator=(const Presence&) = delete;

    /**
     * @brief Singleton instance of this class
     * 
     * The object pointer to this class is stored here. It's NULL at system boot.
     */
    static Presence *_instance;

    int count = 0;

};
#endif  /* __Presence_H */
