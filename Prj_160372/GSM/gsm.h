/*
 * gsm.h
 *
 * Created: 03.04.2018 13:58:27
 *  Athor: mathiasc
 */ 


#ifndef GSM_H_
#define GSM_H_

#include <stdint.h>
#include <stdbool.h>
#include "GSM_PublicTypes.h"

/* This is the public API witch is to be used */ 

/**************************************************************************************************
 *    Function      : GSM_Init
 *    Description   : GSM Initalisation of the M95
 *    Input         : none
 *    Output        : bool
 *....Remarks       : Returns false if we have a Init Timout
 **************************************************************************************************/
bool GSM_Init( void );

/**************************************************************************************************
 *    Function      : GSM_Shutdown
 *    Description   : This will issue a GSM Shutdown command to power down the modem
 *    Input         : none
 *    Output        : bool
 *....Remarks       : none
 **************************************************************************************************/
void GSM_Shutdown( void );

/**************************************************************************************************
 *    Function      : GetGSMPowerDown
 *    Description   : This will return if the modem is in shutdown or not
 *    Input         : none
 *    Output        : bool
 *....Remarks       : none
 **************************************************************************************************/
bool GetGSMPowerDown ( void );


/**************************************************************************************************
 *    Function      : GSM_Task
 *    Description   : This is the GSM Task and needs to be called as frequent as possible 
 *    Input         : none
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void GSM_Task( void );

/**************************************************************************************************
 *    Function      : GSM_RxData
 *    Description   : This will process incomming bytes from the modem 
 *    Input         : uint8_t
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void GSM_RxData( uint8_t ch );

/**************************************************************************************************
 *    Function      : GSM_Send_SMS_P
 *    Description   : This will send an SMS from Flash
 *    Input         : GSM_SMS_MESSAGE_P
 *    Output        : none
 *....Remarks       : Sends one of the predefined messages
 **************************************************************************************************/
GSMCMDResult_t GSM_Send_SMS_P( GSMSMSMESSAGE_t Msg );

/**************************************************************************************************
 *    Function      : GSM_CMD_Send
 *    Description   : This will send a command to the modem 
 *    Input         : GSM_AT_COMMAND_t
 *    Output        : none
 *....Remarks       : Sends one of the predefined commands
 **************************************************************************************************/
GSMCMDResult_t GSM_CMDSend( GSMATCOMMAND_t Command );

/**************************************************************************************************
 *    Function      : GSM_GetLastCommandResult
 *    Description   : This will get the result of the last executed command
 *    Input         : none
 *    Output        : GSM_CMD_LastResult_t
 *....Remarks       : none
 **************************************************************************************************/
GSMCMDLastResult_t GSM_GetLastCommandResult ( void );

/**************************************************************************************************
 *    Function      : GSM_GetLastCME_Error
 *    Description   : This will get the last code of a recognised CME error
 *    Input         : none
 *    Output        : int16_t
 *....Remarks       : if code is 65535 no error so far has been captured
 **************************************************************************************************/
uint16_t GSM_GetLastCME_Error( void );

/**************************************************************************************************
 *    Function      : GSM_GetLastCMS_Error
 *    Description   : This will get the last code of a recognised CMS error
 *    Input         : none
 *    Output        : int16_t
 *....Remarks       : if code is 65535 no error so far has been captured
 **************************************************************************************************/
uint16_t GSM_GetLastCMS_Error( void );

/**************************************************************************************************
 *    Function      : GSM_GetConfigChanged
 *    Description   : This will get if the Config has Changed
 *    Input         : none
 *    Output        : bool
 *....Remarks       : This will be one shot and reset after a read
 **************************************************************************************************/
bool GSM_GetConfigChanged( void );

/**************************************************************************************************
 *    Function      : GSM_GetStatusRequested
 *    Description   : This will get if the Status is requested
 *    Input         : none
 *    Output        : bool
 *....Remarks       : This will be one shot and reset after a read
 **************************************************************************************************/
bool GSM_GetStatusRequested( void );

/**************************************************************************************************
 *    Function      : GSM_GetNetworkStatus
 *    Description   : This will get the Networkstatus
 *    Input         : none
 *    Output        : uint8
 *....Remarks       : none
 **************************************************************************************************/
uint8_t GSM_GetNetworkStatus( void );
#endif /* GSM_H_ */