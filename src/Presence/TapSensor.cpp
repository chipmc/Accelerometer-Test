// People Counter Class
// Author: Chip McClelland
// Date: May 2023
// License: GPL3
// In this class, we look at the occpancy values and determine what the occupancy count should be 
// Note, this code is limited to a single sensor with two zones
// Note, this code assumes that Zone 1 is the inner (relative to room we are measureing occupancy for) and Zone 2 is outer

#include "TapSensor.h"

// Create an MMA8452Q object, used throughout the rest of the sketch.
MMA8452Q accel; // Default constructor, SA0 pin is HIGH

// The above works if the MMA8452Q's address select pin (SA0) is high.
// If SA0 is low (if the jumper on the back of the SparkFun MMA8452Q breakout
// board is closed), initialize it like this:
// MMA8452Q accel(MMA8452Q_ADD_SA0_);

TapSensor *TapSensor::_instance;

// [static]
TapSensor &TapSensor::instance() {
    if (!_instance) {
        _instance = new TapSensor();
    }
    return *_instance;
}

TapSensor::TapSensor() {
}

TapSensor::~TapSensor() {
}

bool TapSensor::setup() {

    // Initialize the accelerometer with begin():
	// begin can take two parameters: full-scale range, and output data rate (ODR).
	// Full-scale range can be: SCALE_2G, SCALE_4G, or SCALE_8G (2, 4, or 8g)
	// ODR can be: ODR_800, ODR_400, ODR_200, ODR_100, ODR_50, ODR_12, ODR_6 or ODR_1
    if (!accel.begin(SCALE_2G, ODR_100)) {
        Log.infoln("Communication with accelerometer failed");     // Set up accel with +/-2g range, and 100Hz ODR
        return false;
    }

    sysStatus.sensitivity = 1;

    accel.setupTapIntsLatch(sysStatus.sensitivity);                        // Set up the tap interrupt

    accel.clearTapInts();

    // To update acceleration values from the accelerometer, call accel.read();
    accel.read();

	// After reading, six class variables are updated: x, y, z, cx, cy, and cz.
	// Those are the raw, 12-bit values (x, y, and z) and the calculated
	// acceleration's in units of g (cx, cy, and cz).

    Log.infoln("Tap Sensor initialized with sensitivity %d and (%F,%F,%F)g's",sysStatus.sensitivity, accel.cx, accel.cy, accel.cz);
    return true;
}

void TapSensor::clearTapInts() {
    accel.clearTapInts();
}

bool TapSensor::loop() {                                            // Called by Presence.loop which is in the main loop - checks on occupancy state
    /*
    if (millis() - lastTime > 1000) {                               // This is optional for testing 
        lastTime = millis();
        accel.read();
        Log.infoln("Acceleration: (%F,%F,%F)g's", accel.cx, accel.cy, accel.cz);
    }
    */

    if (digitalRead(gpio.I2C_INT)) {                                // All we are doing here is passing back the occupancy to the presence function
        // Log.infoln("Occupancy detected");
        TapSensor::clearTapInts();                                  // Clear the interrupt
        return true;
    }
    else {
        return false;
    }
}
