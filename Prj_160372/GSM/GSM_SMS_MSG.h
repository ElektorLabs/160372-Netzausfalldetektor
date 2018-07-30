/*
 * GSM_SMS_MSG.h
 *
 * Created: 10.04.2018 11:58:06
 *  Author: mathiasc
 */ 


#ifndef GSM_SMS_MSG_H_
#define GSM_SMS_MSG_H_

/* Here are the Messages predefined that we will send out in case of events */
/* Text is short and ugly as flash is run out                               */
const char GSM_SMS_POWERLOSS_MSG[] PROGMEM = "Alert:Power loss";
const char GSM_SMS_POWERRESTORED_MSG[] PROGMEM = "Alert:Power restored";
const char GSM_SMS_BATTERY_LOW_MSG[] PROGMEM = "Warning:Backup cells are low";
const char GSM_SMS_BATTERY_EMPTY_MSG[] PROGMEM = "Warning:Backup cells are empty";
const char GSM_SMS_NEW_CONFIG_MSG[] PROGMEM = "Saved new number in EEPROM";

const char GSM_SMS_STATUS_P0[] PROGMEM = "Mains frequency is ";
const char GSM_SMS_STATUS_P1[] PROGMEM = " with ";
const char GSM_SMS_STATUS_P2[] PROGMEM = "Hz. Battery has ";
const char GSM_SMS_STATUS_P3[] PROGMEM = "mV and is ";
const char GSM_SMS_STATUS_P_OKAY[] PROGMEM = "OKAY";
const char GSM_SMS_STATUS_P_FAULTY[] PROGMEM = "FAULTY";
const char GSM_SMS_STATUS_P_LOW[] PROGMEM = "LOW";
const char GSM_SMS_STATUS_P_EMPTY[] PROGMEM = "EMPTY";





#endif /* GSM_SMS_MSG_H_ */