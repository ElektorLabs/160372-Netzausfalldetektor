/*
 * EEPFS.c
 *
 * Created: 09.04.2018 16:19:27
 *  Author: mathiasc
 */ 

#include <stdbool.h>
#include <stdio.h>
#include <util/crc16.h>
#include <avr/eeprom.h>
#include "EEPFS.h"
/* This will be our EEPFS */
/* We use the 512 bytes as follows */
/* The first bytes will be for the RemotLineID */
/* 00..31    RemoteLineID            ( 32 Byte )
   32..33    RemoteLineID CRC        ( 2  Byte )
   34..63   Reserved                ( 30 Byte )
   62..93   RemoteLineID Backup     ( 32 Byte )
   94..95   RemoteLineID Backup CRC ( 2  Byte )
   96..127  Reserved                ( 30 Byte )
   128..195 Reserved for Sysconfig  ( 64 Byte )
   195..255 Reserved for Sysconfig  ( 64 Byte )     
 */
/* We define the EEPROM Layout here */
#define RemoteLineIDAddr ( 0 )
#define RemoteLineIDSize ( 32 )

#define RemoteLineIDBackupAddr ( 62 )
#define RemoteLineIDBackupSize ( 32 )

#define SysconfigAddr ( 128 )
#define SysconfigSize ( 64 )

#ifdef E2END
    #define E2SIZE E2END
#endif
/* Function prototypes */

void EEPFS_WriteBackupLineID(EEPFS_RemoteLineID_t* RemoteID);
void EEPFS_WriteLineID(EEPFS_RemoteLineID_t* RemoteID);

void EEPFS_ReadBackupLineID(EEPFS_RemoteLineID_t* RemoteID);
void EEPFS_ReadLineID(EEPFS_RemoteLineID_t* RemoteID);

bool LineIDCRCCheck( uint16_t* ptru16CRC );
bool LineIDBackupCRCCheck( uint16_t* ptru16CRC );


/**************************************************************************************************
 *    Function      : EEPFS_EraseAll
 *    Description   : Set the EEPROM to completly 0xFF
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void EEPFS_EraseAll( void ){
    /* We can do a crc check for the eeprom if we like to, but is not requiered now */
    for(uint16_t i=0;i<E2SIZE;i++){
       eeprom_update_byte((uint8_t*)(i),0xFF); 
    }
};


/**************************************************************************************************
 *    Function      : EEPFS_ReadRemoteLineID
 *    Description   : Reads the RemoteLineID for the SMS Target
 *    Input         : EEPFS_RemoteLineID_t*  
 *    Output        : bool
 *....Remarks       : Returns false if EEPROM has invalid data 
 **************************************************************************************************/
bool EEPFS_ReadRemoteLineID(EEPFS_RemoteLineID_t* RemoteID){
     struct {
        uint8_t ValueOK:1;
        uint8_t BackupValueOK:1;
        uint8_t Reserved:6;
    } Statusflags;
    
    Statusflags.BackupValueOK=false;
    Statusflags.ValueOK = false; 
    bool boReturnval = false;
        
    uint16_t ValueIDCRC=0;
    uint16_t BackupValueIDCRC=0;
    /* We first check the CRC of both the RemoteID and the BackupID */
    if(true == LineIDCRCCheck( &ValueIDCRC ) ) {
        /* Value is okay */
        Statusflags.ValueOK=1;
    } else {
        /* Value is broken */
        Statusflags.ValueOK=0;
    }
    
    if(true == LineIDBackupCRCCheck( &BackupValueIDCRC) ){
        Statusflags.BackupValueOK=1;
    } else {
        Statusflags.BackupValueOK=0;
    }
    
    /* Next is to decide what to load */
    if( ( 1 == Statusflags.ValueOK) && ( 1 == Statusflags.BackupValueOK ) ){
        /* First, if both values are okay */
        EEPFS_ReadLineID(RemoteID);
        /* We compare the CRC and if both don't match we write the Backup again */
        if(ValueIDCRC != BackupValueIDCRC){
            EEPFS_WriteBackupLineID(RemoteID);
        }
        boReturnval=true;
        
    } else if( ( 1 != Statusflags.ValueOK) && ( 1 == Statusflags.BackupValueOK ) ){
        /* Second, if the LineID is not okay  */
        EEPFS_ReadBackupLineID(RemoteID);
        if(ValueIDCRC != BackupValueIDCRC){
            EEPFS_WriteBackupLineID(RemoteID);
        }
        boReturnval=true;
        
    } else if( ( 1 == Statusflags.ValueOK) && ( 1 != Statusflags.BackupValueOK ) ){
       /* Third, if the BackupID is broken */ 
        EEPFS_ReadLineID(RemoteID);
        EEPFS_WriteBackupLineID(RemoteID);
        boReturnval=true;
        
    } else {
        /* Last, if both IDs are broken */
        boReturnval=false;
    }
    
    
    return boReturnval;
}

/**************************************************************************************************
 *    Function      : EEPFS_WriteRemoteLineID
 *    Description   : Writes the RemoteLineID for the SMS Target
 *    Input         : EEPFS_RemoteLineID_t*  
 *    Output        : bool
 *....Remarks       : Returns false if EEPROM has write errors
 **************************************************************************************************/
bool EEPFS_WriteRemoteLineID(EEPFS_RemoteLineID_t* RemoteID){

    //Okay now we need to Build our CRC
    uint8_t* ptr=(uint8_t *)(RemoteID);
    uint16_t crc_calc=0xFFFF;
    uint8_t size=sizeof(EEPFS_RemoteLineID_t);
    if(size>2){
        size-=2; /* substract 2, as we don't want to have the crc in there */
    }
    for(uint8_t i=0;i<size;i++)
    {
        crc_calc = _crc_ccitt_update(crc_calc,*ptr);
        ptr++;
    }
    RemoteID->CRC=crc_calc;
    //And now everything must back to the EEPROM
    EEPFS_WriteLineID(RemoteID);
    EEPFS_WriteBackupLineID(RemoteID);
    
    return true;
}

/**************************************************************************************************
 *    Function      : EEPFS_WriteBackupLineID
 *    Description   : Writes the Backup-LineID
 *    Input         : EEPFS_RemoteLineID_t*
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void EEPFS_WriteBackupLineID(EEPFS_RemoteLineID_t* RemoteID)
{
    uint8_t* ptr=(uint8_t *)(RemoteID);
    /* This is internal and will pain wirte the config to the Backup location */
    for(uint16_t i=0;i<sizeof(EEPFS_RemoteLineID_t);i++)
    {
        eeprom_update_byte((uint8_t *)i+(RemoteLineIDBackupAddr) , *ptr);
        ptr++;
    }
}

/**************************************************************************************************
 *    Function      : EEPFS_WriteLineID
 *    Description   : Writes the LineID
 *    Input         : EEPFS_RemoteLineID_t*
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void EEPFS_WriteLineID(EEPFS_RemoteLineID_t* RemoteID)
{
    uint8_t* ptr=(uint8_t *)(RemoteID);
    /* This is internal and will pain wirte the config to the Backup location */
    for(uint16_t i=0;i<sizeof(EEPFS_RemoteLineID_t);i++)
    {
        eeprom_update_byte((uint8_t *)i+(RemoteLineIDAddr) , *ptr);
        ptr++;
    }
}

/**************************************************************************************************
 *    Function      : EEPFS_ReadLineID
 *    Description   : Reads the content of the backup linieid from eeprom
 *    Input         : uint16_t*
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void EEPFS_ReadBackupLineID(EEPFS_RemoteLineID_t* RemoteID){
    
    uint16_t crc_calc=0xFFFF;
    uint8_t* ptr=(uint8_t *)(RemoteID);
    
    for(uint16_t i=0;i<(sizeof(EEPFS_RemoteLineID_t));i++)
    {
        *ptr=eeprom_read_byte((uint8_t *)i+(RemoteLineIDBackupAddr));
        crc_calc = _crc_ccitt_update(crc_calc,*ptr);
        ptr++;
    }
    
    /* We now plain copied the data to the given struct */
}

/**************************************************************************************************
 *    Function      : EEPFS_ReadLineID
 *    Description   : Reads the content of the linieid from eeprom
 *    Input         : uint16_t*
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void EEPFS_ReadLineID(EEPFS_RemoteLineID_t* RemoteID){
    
    uint16_t crc_calc=0xFFFF;
    uint8_t* ptr=(uint8_t *)(RemoteID);
    
    for(uint8_t i=0;i<(sizeof(EEPFS_RemoteLineID_t));i++)
    {
        *ptr=eeprom_read_byte((uint8_t *)(i+RemoteLineIDAddr));
        crc_calc = _crc_ccitt_update(crc_calc,*ptr);
        ptr++;
    }
    
    /* We now plain copied the data to the given struct */
}

/**************************************************************************************************
 *    Function      : LineIDCRCCheck
 *    Description   : Checks if the CRC in EEPROM matches the Content in EEPROM
 *    Input         : uint16_t*
 *    Output        : bool
 *....Remarks       : Returns false if the CRC dosn't match
 **************************************************************************************************/
bool LineIDCRCCheck( uint16_t* CRC ){
    bool Returnval = false;
    
    uint8_t data=0;
    
    uint8_t lenght=sizeof(EEPFS_RemoteLineID_t);
    uint16_t crc_calc=0xFFFF;
    uint16_t stored_crc=0;
    if(lenght>2){
        lenght=lenght-2;
    }
    for(uint8_t i=0;i<(lenght);i++)
    {
        data=eeprom_read_byte((uint8_t *)(i+RemoteLineIDAddr));
        crc_calc = _crc_ccitt_update(crc_calc,data);
    }
    
    stored_crc=eeprom_read_word((uint16_t *)(lenght+(RemoteLineIDAddr)));
    
    /* Next is to compare the items */
    if(stored_crc != crc_calc){
        Returnval = false;
    } else {
        Returnval = true;
    }
    
    if(CRC!=NULL){
        *CRC = crc_calc;     
    }
    
    return Returnval;
}

/**************************************************************************************************
 *    Function      : LineIDBackupCRCCheck
 *    Description   : Checks if the CRC in EEPROM matches the Content in EEPROM
 *    Input         : uint16_t*
 *    Output        : bool
 *....Remarks       : Returns false if the CRC dosn't match
 **************************************************************************************************/
bool LineIDBackupCRCCheck( uint16_t* CRC ){
        bool Returnval = false;
        
        uint8_t data=0;
        
        uint8_t lenght=sizeof(EEPFS_RemoteLineID_t);
        uint16_t crc_calc=0xFFFF;
        uint16_t stored_crc=0;
        if(lenght>2){
            lenght=lenght-2;
        }
        for(uint8_t i=0;i<(lenght);i++)
        {
            data=eeprom_read_byte((const uint8_t *)((uint16_t)+(RemoteLineIDBackupAddr)));
            crc_calc = _crc_ccitt_update(crc_calc,data);
        }
        
        stored_crc=eeprom_read_word((const uint16_t *)(lenght+(RemoteLineIDBackupAddr)));
        
        /* Next is to compare the items */
        if(stored_crc != crc_calc){
            Returnval = false;
            } else {
            Returnval = true;
        }
        
        if(CRC!=NULL){
            *CRC = crc_calc;
        }
        
        return Returnval;
}

/**************************************************************************************************
 *    Function      : EEPFS_ReadSystemstatus
 *    Description   : Reads the Systemstatus
 *    Input         : EEPFS_Systemstatus_t*  
 *    Output        : bool
 *....Remarks       : Returns false if EEPROM has invalid data 
 **************************************************************************************************/
bool EEPFS_ReadSystemstatus( EEPFS_Systemstatus_t* SysStatus ){

    bool Result = false;
    uint16_t crc_calc=0xFFFF;
    uint8_t* ptr=(uint8_t *)(SysStatus);
    uint8_t size=sizeof(EEPFS_Systemstatus_t);
    if(size>2){
        size-=2; /* substract 2, as we don't want to have the crc in there */
    }
    for(uint8_t i=0;i<(size);i++)
    {
        *ptr=eeprom_read_byte((const uint8_t *)((uint16_t)i+(SysconfigAddr)));
        crc_calc = _crc_ccitt_update(crc_calc,*ptr);
        ptr++;
    }
    
    for(uint8_t i=size;i<(sizeof(EEPFS_Systemstatus_t));i++)
    {
        *ptr=eeprom_read_byte((const uint8_t *)(i+(SysconfigAddr)));
        ptr++;
    }    

    if(crc_calc != SysStatus->CRC){
        Result = false;
    } else {
        Result = true;
    }
    return Result;
}

/**************************************************************************************************
 *    Function      : EEPFS_ReadSystemstatus
 *    Description   : Writes the Systemstatus
 *    Input         : EEPFS_Systemstatus_t*  
 *    Output        : bool
 *....Remarks       : Returns false if EEPROM has write errors
 **************************************************************************************************/
bool EEPFS_WriteSystemstatus( EEPFS_Systemstatus_t* SysStatus ){

    bool Result=false;
    EEPFS_Systemstatus_t StatusReadData;
    //Okay now we need to Build our CRC
    uint8_t* ptr=(uint8_t *)(SysStatus);
    uint16_t crc_calc=0xFFFF;
    uint8_t size=sizeof(EEPFS_Systemstatus_t);
    if(size>2){
        size-=2; /* substract 2, as we don't want to have the crc in there */
    }
    for(uint8_t i=0;i<size;i++)
    {
        crc_calc = _crc_ccitt_update(crc_calc,*ptr);
        ptr++;
    }
    SysStatus->CRC=crc_calc;

    ptr=(uint8_t *)(SysStatus);
    /* This is internal and will plain write the config to the backup location */
    for(uint16_t i=0;i<sizeof(EEPFS_Systemstatus_t);i++)
    {
        eeprom_update_byte((uint8_t *)i+(SysconfigAddr) , *ptr);
        ptr++;
    }
    if(false == EEPFS_ReadSystemstatus(&StatusReadData) ){
        Result=false;
    } else {
        Result = true;
    }
    return Result;
}

