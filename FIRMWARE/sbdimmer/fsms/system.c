/*
 * system.c
 *
 * Created: 01.04.2019 13:39:23
 *  Author: fly8r
 */
#include "include/system.h"

/************************************************************************/
/* VARS                                                                 */
/************************************************************************/
volatile	static		uint8_t			FSM_state;
static					uint8_t			delta=FSM_SYSTEM_OCR_DEFAULT_DELTA;
static					uint8_t			flash_count;


/* EEPROM data */
static					settings_data_t	EEMEM	ee_settings;

/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/
void FSM_SYSTEM_OutputTrigger(void);
void FSM_SYSTEM_LoadSettingsFromEEPROM(void);
void FSM_SYSTEM_SaveSettingsToEEPROM(void);

void FSM_SYSTEM_Init(void)
{
	// Set default FSM state
	FSM_state = FSM_SYSTEM_STATE_IDLE;
	// Load settings from EEPROM
	FSM_SYSTEM_LoadSettingsFromEEPROM();
	// Load current value from PWM table
	if(device.settings.level==0) device.settings.level=16;
}

void FSM_SYSTEM_Process(void)
{
	uint8_t tmp=0;
	
	switch(FSM_state) {

		case FSM_SYSTEM_STATE_IDLE: {
			/************************************************************************/
			/* Button processing                                                    */
			/************************************************************************/
			if(GetMessage(MSG_BTN_KEY_PRESSED)) { // <- Press message processing
				// Getting press type
				uint8_t *press_type = GetMessageParam(MSG_BTN_KEY_PRESSED);
				// Processing button press by press time
				if(*press_type == BUTTON_EVENT_LONG_PRESS) {
					if(device._state) { // <- If device state is ON
						// Save settings
						FSM_SYSTEM_SaveSettingsToEEPROM();
						// Goto indicate saved state
						FSM_state = FSM_SYSTEM_STATE_FLASHING_LIGHT;
						// Set flashing count
						flash_count = FSM_SYSTEM_FLASH_COUNT * 2;
						// Flush FSM timer
						ResetTimer(TIMER_SYSTEM);
						return;
					} else {
						// Load saved values
						FSM_SYSTEM_LoadSettingsFromEEPROM();
						// Change device state
						FSM_SYSTEM_OutputTrigger();
					}
				} else {
					// Change device state
					FSM_SYSTEM_OutputTrigger();
				}
			} // End MSG_BTN_KEY_PRESSED message processing

			/************************************************************************/
			/* Encoder processing                                                   */
			/************************************************************************/
			if(GetMessage(MSG_ENC_ROTATE)) {
				// Get rotate direction
				int8_t *rotate = GetMessageParam(MSG_ENC_ROTATE);
				//
				if(device.settings.level > (FSM_SYSTEM_OCR_MAX_VALUE-64)) {
					delta = FSM_SYSTEM_OCR_MAX_DELTA;
				} else {
					delta = FSM_SYSTEM_OCR_DEFAULT_DELTA;
				}
				// Rotation processing
				if(*rotate > 0) {
					tmp = device.settings.level + delta;
					if(tmp >= FSM_SYSTEM_OCR_MAX_VALUE) {
						device.settings.level = FSM_SYSTEM_OCR_MAX_VALUE;
					} else {
						device.settings.level = tmp;
					}
				} else {
					tmp = device.settings.level - delta;
					if(tmp < FSM_SYSTEM_OCR_MIN_VALUE) {
						device.settings.level = FSM_SYSTEM_OCR_MIN_VALUE;
					} else {
						device.settings.level = tmp;
					}
				}
				// Set value from PWM table by level value
				PWM_SET_LEVEL(device.settings.level);
			}
			return;
		}

		case FSM_SYSTEM_STATE_FLASHING_LIGHT: {
			if(GetTimer(TIMER_SYSTEM) > FSM_SYSTEM_FLASH_HALF_PERIOD) {
				if(!--flash_count) {
					// Set PWM to ON state
					PWM_ON(device.settings.level);
					// Set ON state
					device._state=1;
					// Set default FSM state
					FSM_state = FSM_SYSTEM_STATE_IDLE;
					return;
				}
				// Change device state
				FSM_SYSTEM_OutputTrigger();
				// Flush FSM timer
				ResetTimer(TIMER_SYSTEM);
			}
			return;
		}

		default: break;
	}

}

void FSM_SYSTEM_OutputTrigger(void)
{
	if(device._state) { // <- Device state is ON
		// Turn OFF PWM IO
		PWM_OFF();
		// Set current device state
		device._state=0;
	} else { // <- Device state is OFF
		// Set value from PWM table by level value
		PWM_ON(device.settings.level);
		// Set current device state
		device._state=1;
	}
}

void FSM_SYSTEM_LoadSettingsFromEEPROM(void)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Waiting for EEPROM ready
		while(!eeprom_is_ready());
		// Read settings from EEPROM
		eeprom_read_block((void *)&device.settings, (void *)&ee_settings, sizeof(ee_settings));
	}
}

void FSM_SYSTEM_SaveSettingsToEEPROM(void)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Waiting for EEPROM ready
		while(!eeprom_is_ready());
		// Store settings to EEPROM
		eeprom_write_block((void *)&device.settings, (void *)&ee_settings, sizeof(device.settings));
	}
}