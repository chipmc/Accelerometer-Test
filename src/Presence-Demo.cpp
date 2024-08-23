/*	Accelerometer-Interrupt-Test.cpp
	Chip McClelland (chip@seeinsights.com)
	May 12th 2024
	https://github.com/sparkfun/SparkFun_MMA8452Q_Particle_Library

	This sketch is for development of occupancy for the LoRA node 
    Occupancy sensors only state whether a space is occupired or not
    Currently, the two sensors we will use are the PIR sensor and the Accelerometer

	Adafruit Feather m0
	Distributed as-is; no warranty is given. 

	The concept is that this program will keep track of the number of seconds the device has detected occupancy.
	Resetting the device resets the count
	Occupancy is tracked in seconds.
*/

// v1.0 - Initial setup to validate approach - hard wired for the accelerometer sensor initially
// v2.0	- Working to build the propper behaviour including sleep

//Include Libraries:
#include <arduino.h>
#include <ArduinoLowPower.h>
#include <ArduinoLog.h>     // https://github.com/thijse/Arduino-Log
#include <Wire.h>

//Include program structure
#include "pinout.h"
#include "timing.h"
#include "stsLED.h"
#include "take_measurements.h"
#include "MyData.h"
#include "Config.h"

const uint8_t firmwareRelease = 1;

// State Machine Variables
enum State { ERROR_STATE, IDLE_STATE, SLEEPING_STATE, LOW_BATTERY};
char stateNames[4][16] = {"Error", "Idle", "Sleeping", "Low Battery"};
volatile State state = IDLE_STATE;
State oldState = IDLE_STATE;

// Initialize Functions
void userSwitchISR();
void sensorISR();
void publishStateTransition(void);
void wakeUp_Timer();

// Program Variables	
volatile uint8_t IRQ_Reason = 0; 										// 0 - Invalid, 1 - AB1805, 2 - RFM95 DIO0, 3 - RFM95 IRQ, 4 - User Switch, 5 - Sensor
time_t lastEventTime = 0;		

void setup()
{
	Wire.begin(); 														// Establish Wire.begin for I2C communication
	Serial.begin(115200);												// Establish Serial connection if connected for debugging
	delay(2000);

	gpio.setup(); 														// Setup the pins
 	LED.setup(gpio.STATUS);												// Led used for status
	LED.on();                           								// Turn on the LED to show the program is running

	// Log.begin(LOG_LEVEL_SILENT, &Serial);
	Log.begin(LOG_LEVEL_INFO, &Serial);
	Log.infoln("PROGRAM: See Insights LoRa Node - Accelerometer Test");

	if (sysData.setup()) {												// Set up the system data
		Log.infoln("System data set up");
		sysStatus.debounceMin = 1;										// Set the debounce time
		lastEventTime = timeFunctions.getTime();                    // Record the time of the event
		currentData.setup();											// Set up current storage objects if the system data is set up
	}
	else {
		Log.infoln("System data failed to set up");
		state = ERROR_STATE;
	}

	if (timeFunctions.setup()) {										// Set up the timing functions
		Log.infoln("Time functions set up");
	}
	else {
		Log.infoln("Time functions failed to set up");
		state = ERROR_STATE;
	}

	if (!measure.setup()) {												// Set up the sensor
		Log.infoln("Sensor failed to set up");
		state = ERROR_STATE;
	}

	if (!digitalRead(gpio.USER_SW)) {
		uint32_t startTesting = millis();
		Log.infoln("User switch detected - 20 seconds to test tap sensitivity");
		while (millis() - startTesting < 20000UL) {
			if (digitalRead(gpio.I2C_INT)) {
				Log.infoln("Tap detected");
				measure.clearPresenceInterrupt();
				delay(100);    
			}
		}
		Log.infoln("Time is up - resuming startup");
	}

	LowPower.attachInterruptWakeup(gpio.I2C_INT, sensorISR, RISING);                  	// Accelerometer interrupt from low to high
	LowPower.attachInterruptWakeup(gpio.USER_SW, userSwitchISR, FALLING);             	// User switch interrupt from high to low
	LowPower.attachInterruptWakeup(gpio.WAKE, wakeUp_Timer, FALLING);               	// RTC Alarm interrupt from low to high

	Log.infoln("Setup process completed exiting in state %s", stateNames[state]);

	LED.off();                                                     		//  End of the setup routine

}

void loop()
{
  switch (state) {
    case IDLE_STATE:
    	if (state != oldState) publishStateTransition();             	// We will apply the back-offs before sending to ERROR state - so if we are here we will take action
		if (timeFunctions.getTime() - lastEventTime > (sysStatus.debounceMin * 60UL)) {		// If we have been in IDLE for more than the debounce period, go to sleep
			// state = SLEEPING_STATE; 
			Log.infoln("In the idle state longer than the debounce period (%d mins)- going to sleep", sysStatus.debounceMin);
			lastEventTime = timeFunctions.getTime();                    // Record the time of the event
			LED.off();
			break;	
		}
    break;

    case SLEEPING_STATE: {
		if (state != oldState) publishStateTransition();              	// We will apply the back-offs before sending to ERROR state - so if we are here we will take action
		IRQ_Reason = IRQ_Invalid;

		if (digitalRead(gpio.I2C_INT)){ 
			Log.infoln("Sensor pin(line2) still high - delaying sleep"); 
			break;
		}
		
		time_t currentTime = timeFunctions.getTime();					// How long to sleep

    	unsigned long sleepTime = 60UL;                      			// Sleep for 60 seconds

		Log.infoln("Going to sleep for %u seconds", sleepTime);

		timeFunctions.stopWDT();  										// No watchdogs interrupting our slumber

		Serial.flush();													// Ensure all serial data is sent
		Serial.end();													// Close the serial port
		delay(100);
		timeFunctions.interruptAtTime(currentTime + sleepTime, 0);      // Set the interrupt for the next event
		LowPower.sleep((sleepTime + 1) * 1000UL);						// Set the sleep time in milliseconds 
		Serial.begin(115200);											// Reopen the serial port
		unsigned long wakeStartTime = millis();
		while (!Serial) {
			if (millis() - wakeStartTime > 5000) break;
		}												// Wait for the serial port to open
		timeFunctions.resumeWDT();                          			// Wakey Wakey - WDT can resume
	
		if (IRQ_Reason == IRQ_Sensor) {
			Log.infoln("Woke up for Sensor"); 							// Interrupt from the PIR Sensor
			lastEventTime = timeFunctions.getTime();                    // Record the time of the event
			state = IDLE_STATE;
		}
		else if (IRQ_Reason == IRQ_UserSwitch) {
			Log.infoln("Woke up for User Switch");
			lastEventTime = timeFunctions.getTime();                    // Record the time of the event
			state = IDLE_STATE;
		}
		else if (IRQ_Reason == IRQ_AB1805) {
			Log.infoln("Woke up for RTC Alarm");
			lastEventTime = timeFunctions.getTime();                    // Record the time of the event
			state = IDLE_STATE;
		}
		else {
			Log.infoln("Woke up without an interrupt - going to IDLE");
			state = IDLE_STATE;
		}
    } break;

    case LOW_BATTERY:
    	if (state != oldState) publishStateTransition();
		state = IDLE_STATE;												// Logs the low battery state don't get stuck here
    break;

	case ERROR_STATE:
    	if (state != oldState) publishStateTransition();

		if (state == ERROR_STATE) {
			Log.infoln("Error state reached - exiting");
			while (1) {
				LED.on();
				delay(1000);
				LED.off();
				delay(1000);
			}
		}
    break;
  }

  measure.loop();                                                   	// Check the sensor
  currentData.loop();													// Ensure data is stored when needed.
  sysData.loop();
  timeFunctions.loop();													// Keep the time up to date
}

/**
 * @brief Publishes a state transition to the Log Handler and to the Particle monitoring system.
 *
 * @details A good debugging tool.
 */
void publishStateTransition(void)
{
	char stateTransitionString[256];
	bool publish = true;
	snprintf(stateTransitionString, sizeof(stateTransitionString), "From %s to %s", stateNames[oldState],stateNames[state]);
	oldState = state;
	if (publish) Log.infoln(stateTransitionString);
}

void wakeUp_Timer() {
    IRQ_Reason = IRQ_AB1805;
    // Something more here?
}

void userSwitchISR() {
  	IRQ_Reason = IRQ_UserSwitch;
}

void sensorISR() {	
	IRQ_Reason = IRQ_Sensor;      // and write to IRQ_Reason in order to wake the device up
}