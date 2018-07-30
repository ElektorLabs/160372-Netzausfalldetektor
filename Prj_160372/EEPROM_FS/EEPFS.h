/*
 * EEPFS.h
 *
 * Created: 09.04.2018 16:19:41
 *  Author: mathiasc
 */ 


#ifndef EEPFS_H_
#define EEPFS_H_

#include <stdbool.h>
#include "..\BATTERY_MON\battery_mon.h"

typedef struct {
    char LineID[20]; /* This will be the ID to send the SMS to */
    uint16_t CRC;    /* Needs to be the last element in struct */
} EEPFS_RemoteLineID_t;

typedef struct {
    BATTERYSTATUS_t BatteryStatus;
    uint8_t PowerLoss:1;
    uint8_t LastStatusReported:1;
    uint8_t NewConfig:1;
    uint8_t Reserved:5;
    uint16_t CRC;
} EEPFS_Systemstatus_t;

/**************************************************************************************************
 *    Function      : EEPFS_EraseAll
 *    Description   : Set the EEPROM to completly 0xFF
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void EEPFS_EraseAll( void );

/**************************************************************************************************
 *    Function      : EEPFS_ReadRemoteLineID
 *    Description   : Reads the RemoteLineID for the SMS Target
 *    Input         : EEPFS_RemoteLineID_t*  
 *    Output        : bool
 *....Remarks       : Returns false if EEPROM has invalid data 
 **************************************************************************************************/
bool EEPFS_ReadRemoteLineID(EEPFS_RemoteLineID_t* RemoteID);

/**************************************************************************************************
 *    Function      : EEPFS_WriteRemoteLineID
 *    Description   : Writes the RemoteLineID for the SMS Target
 *    Input         : EEPFS_RemoteLineID_t*  
 *    Output        : bool
 *....Remarks       : Returns false if EEPROM has write errors
 **************************************************************************************************/
bool EEPFS_WriteRemoteLineID(EEPFS_RemoteLineID_t* RemoteID);

/**************************************************************************************************
 *    Function      : EEPFS_ReadSystemstatus
 *    Description   : Reads the Systemstatus
 *    Input         : EEPFS_Systemstatus_t*  
 *    Output        : bool
 *....Remarks       : Returns false if EEPROM has invalid data 
 **************************************************************************************************/
bool EEPFS_ReadSystemstatus( EEPFS_Systemstatus_t* SysStatus );


/**************************************************************************************************
 *    Function      : EEPFS_ReadSystemstatus
 *    Description   : Writes the Systemstatus
 *    Input         : EEPFS_Systemstatus_t*  
 *    Output        : bool
 *....Remarks       : Returns false if EEPROM has write errors
 **************************************************************************************************/
bool EEPFS_WriteSystemstatus( EEPFS_Systemstatus_t* SysStatus );

#endif /* EEPFS_H_ */