/*
 * GSM_SMS_Parser.h
 *
 * Created: 23.04.2018 16:15:54
 *  Author: mathiasc
 */ 


#ifndef GSM_SMS_PARSER_H_
#define GSM_SMS_PARSER_H_

void voGSM_ParseSMS( GSM_SMS_MSG_t* Message );

/**************************************************************************************************
 *    Function      : GSM_ConfigParsedOkay
 *    Description   : This will get if the Config has Changed
 *    Input         : none
 *    Output        : bool
 *....Remarks       : This will be one shot and reset after a read
 **************************************************************************************************/
bool GSM_ConfigParsedOkay( void );

/**************************************************************************************************
 *    Function      : GSM_RequestParsedOkay
 *    Description   : This will get if the Status is requested
 *    Input         : none
 *    Output        : bool
 *....Remarks       : This will be one shot and reset after a read
 **************************************************************************************************/
bool GSM_RequestParsedOkay( void );

#endif /* GSM_SMS_PARSER_H_ */