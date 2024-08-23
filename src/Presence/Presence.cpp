// People Counter Class
// Author: Chip McClelland
// Date: May 2023
// License: GPL3
// In this class, we look at the occpancy values and determine what the occupancy count should be 
// Note, this code is limited to a single sensor with two zones
// Note, this code assumes that Zone 1 is the inner (relative to room we are measureing occupancy for) and Zone 2 is outer

#include "Presence.h"

Presence *Presence::_instance;

// [static]
Presence &Presence::instance() {
    if (!_instance) {
        _instance = new Presence();
    }
    return *_instance;
}

Presence::Presence() {
}

Presence::~Presence() {
}

bool Presence::setup() {

    if (!TapSensor::instance().setup()) {
        return false;
    }
    else {
        return true;
    }
}

bool Presence::loop() {
    static bool lastOccupancyState = false;
    static time_t occupancyPeriodStart = 0;
    bool occupancyState = TapSensor::instance().loop();
    time_t now = timeFunctions.getTime();
    if (occupancyState) {                                       // Occupancy detected
        if (!lastOccupancyState) {                              // This is a new occupancy period
            lastOccupancyState = true;                          // Set the last state to true  
            occupancyPeriodStart = timeFunctions.getTime();		// Begin a new period of occupancy  
            Log.infoln("Starting a new occupancy period at %d", occupancyPeriodStart);
            LED.on();                                           // Turn on the indicator LED - take out for production
        }
        else {
            Log.infoln("Continue current occupancy period");
        }
    }
    else if (lastOccupancyState && ((now - occupancyPeriodStart) > sysStatus.debounceMin * 60UL)) {                                                   // Occupancy is no longer detected
        lastOccupancyState = false;                            // End the period of occupancy
        current.occupancyNet += timeFunctions.getTime() - occupancyPeriodStart;          // calculate the net occupancy time
        current.occupancyGross = current.occupancyNet +1 ;     // Gross occupancy is bigger by one for testing memory storage
        currentData.currentDataChanged = true;                 // Set the flag to save the data
        Log.infoln("Occupancy period has ended - total occupancy today is currently %d seconds", current.occupancyNet);
        LED.off();                                              // Turn off the LED now that occupancy is over
    }
    else {
       // Log.infoln("No occupancy detected"); // This will run every loop if no occupancy is detected
    }

    return true;
}
