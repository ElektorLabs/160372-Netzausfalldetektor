/*
 * GSM_SMS_Partser.c
 *
 * Created: 23.04.2018 16:15:39
 *  Author: mathiasc
 */ 

#include <avr/pgmspace.h>
#include <string.h>
#include "..\EEPROM_FS\EEPFS.h"
#include "GSM_PrivateTypes.h"
#include "GSM_SMS_Parser.h"
/* This will be our SMS Parser in simple parts */
/* We just expect to have "Config Remote [ID]" */
const char SMS_Parser_Config_Remote[] PROGMEM ="Config Remote ";
const char SMS_Parser_Req_Status[] PROGMEM ="Request Status";

volatile bool ConfigHasChanged = false;
volatile bool RequestStatus = false;

void voGSM_ParseSMS( GSM_SMS_MSG_t* Message ){
    bool boInvalidID=false;
    EEPFS_RemoteLineID_t RemoteID;
    memset(RemoteID.LineID,0,sizeof(RemoteID.LineID));
    RemoteID.CRC=0;
    /* We need to get the Token we can process here */
    /* This is not pretty done but shall work        */
    
    /* First check is if we have more than 15 chars in the message */
    if( (Message->Messagelen>=15) || ( Message->Messagelen <= 35 ) ){
        if( 0 == strncmp_P((const char*)&Message->Message[0],SMS_Parser_Config_Remote, 14 ) ){
            /* Okay we can now do the parsing of the no form the SMS */
            /* For this we will take the new start address and try to find invalid elements */
            /* valid elements are +,0,1,2,3,4,5,6,7,8,9 , we don't support spaces for now */
            for(uint8_t i=0;i<( (Message->Messagelen)-14 );i++){
                if( ( '+' == Message->Message[i+14] ) || ( ( Message->Message[i+14] >= '0' ) && ( Message->Message[i+14] <= '9') ) ){
                        RemoteID.LineID[i]=Message->Message[i+14];
                    } else{
                        boInvalidID=true;
                        break;
                    }                     
                } /* End of for loop Message */
                    /* If we have a valid ID we put that the the EEPROM */
                if(false == boInvalidID){
                        
                    EEPFS_WriteRemoteLineID(&RemoteID); /* Write the ID to EEPROM */
                    /* We need to signal that we need to send a SMS for the new Config */
                    ConfigHasChanged = true;
                }
            
            }  /* End of MSG check */
            
            if( 0 == strncmp_P((const char*)&Message->Message[0],SMS_Parser_Req_Status, 14 ) ){
                RequestStatus=true;
            }                
            
       }  /* End of Lenghtcheck */
} /* End of function */


/**************************************************************************************************
 *    Function      : GSM_ConfigParsedOkay
 *    Description   : This will get if the Config has Changed
 *    Input         : none
 *    Output        : bool
 *....Remarks       : This will be one shot and reset after a read
 **************************************************************************************************/
bool GSM_ConfigParsedOkay( void ){
    bool Result = ConfigHasChanged;
    ConfigHasChanged = false;
    return Result;

}

/**************************************************************************************************
 *    Function      : GSM_RequestParsedOkay
 *    Description   : This will get if the Status is Requested
 *    Input         : none
 *    Output        : bool
 *....Remarks       : This will be one shot and reset after a read
 **************************************************************************************************/
bool GSM_RequestParsedOkay( void ){
    
    bool Result = RequestStatus;
    RequestStatus = false;
    return Result;
    
}