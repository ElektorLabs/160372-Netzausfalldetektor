/*
 * battery_mon.h
 *
 * Created: 11.04.2018 08:23:52
 *  Author: mathiasc
 */ 

#ifndef BATTERY_H_
#define BATTERY_H_

typedef enum {
  VBAT_OK,
  VBAT_LOW,
  VBAT_EMPTY
} BATTERYSTATUS_t;

typedef enum {
  SUPPLY_OK,
  SUPPLY_FAIL    
}POWERSUPPLYSTATUS_t;

typedef enum{
    BATTERYIN=0,
    POWERSUPPLY
} POWERSOURCE_t;

typedef    struct {
        uint16_t VBat; /* Batteryvoltage in mV */
        BATTERYSTATUS_t Batterystatus;
        POWERSUPPLYSTATUS_t Supplystatus;
        POWERSOURCE_t Powersource;
}BatteryStatus_t;

/**************************************************************************************************
 *    Function      : voBATTERY_MON_init
 *    Description   : Init of the battery monitor
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void BATTERY_MON_init( );

/**************************************************************************************************
 *    Function      : voBATTERY_MON_VBatUpdate
 *    Description   : Processes the ADC Value and updates the flags for the battery level
 *    Input         : uint16_t  
 *    Output        : none
 *....Remarks       : To be called from the ADC ISR with fresh value
 **************************************************************************************************/
void BATTERY_MON_VBatUpdate( uint16_t u16mVolt);

/**************************************************************************************************
 *    Function      : strBATTERY_MON_GetInfo
 *    Description   : Returns the current battery information
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
BatteryStatus_t BATTERY_MON_GetInfo( );

#endif