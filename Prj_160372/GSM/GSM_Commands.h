/*
 * GSM_Commands.h
 *
 * Created: 10.04.2018 10:31:59
 *  Author: mathiasc
 */ 


#ifndef GSM_COMMANDS_H_
#define GSM_COMMANDS_H_

/* We define here the token we have for the SMS Commands */
#include "GSM_PublicTypes.h"

typedef struct{
    GSMATCOMMAND_t Command;                 /* The command enum */
    PGM_P CommandStrPtr;                       /* Pointer to the string in Flash */
    uint8_t Timout;                           /* Timeout for the command */
    uint8_t (*CallbackPostCondition)(void );   /* Callback to check if a certain condition is met */
} PGM_GSM_Command_Tab_t;




/* This is done as extern to safe includes and avoid some mess with the header  */
/* This will be used as callbacks inside the table                              */
extern uint8_t CheckIfSIMisReady( void );
extern uint8_t CheckIfNetLoginIsReady( void );

/* This are the commandstrings defined as const in the FLASH */
const char GSMSETATE[] PROGMEM    = "ATE0";                                /* GSM_AT_ATE */
const char GSMSETIFC[] PROGMEM    = "AT+IFC=2,2";                          /* GSM_AT_AT_IFC */
const char GSMSETCPMS[] PROGMEM   = "AT+CPMS=\"SM\",\"SM\",\"SM\"";        /* GSM_AT_AT_CPMS */
const char GSMSETCNMI[] PROGMEM   = "AT+CNMI=2,2";                         /* GSM_AT_AT_CNMI */
const char GSMSETCMFG[] PROGMEM   = "AT+CMGF=1";                           /* GSM_AT_AT_CMGF */
const char GSMPWRDOWN[] PROGMEM   = "AT+QPOWD=0";                          /* GSM_AT_AT_QPOWD */
const char GSMQINITSTAT[] PROGMEM = "AT+QINISTAT";                         /* GSM__AT_QINITSTAT */
const char GSMQSTATUS[] PROGMEM   = "AT+QNSTATUS";                         /* GSM_AT_QNSTATUS */

/* The command table */
const PGM_GSM_Command_Tab_t GSM_COMMAND_TAB[]={
    {.Command = GSM_QINITSTAT,   .CommandStrPtr = GSMQINITSTAT, .Timout = 4, .CallbackPostCondition = CheckIfSIMisReady},
    {.Command = GSM_AT_QPOWD,    .CommandStrPtr = GSMPWRDOWN,   .Timout = 4, .CallbackPostCondition = NULL},
    {.Command = GSM_AT_CMGF,     .CommandStrPtr = GSMSETCMFG,   .Timout = 4, .CallbackPostCondition = NULL},
    {.Command = GSM_AT_CNMI,     .CommandStrPtr = GSMSETCNMI,   .Timout = 4, .CallbackPostCondition = NULL},
    {.Command = GSM_AT_CPMS,     .CommandStrPtr = GSMSETCPMS,   .Timout = 4, .CallbackPostCondition = NULL},
    {.Command = GSM_AT_IFC,      .CommandStrPtr = GSMSETIFC,    .Timout = 4, .CallbackPostCondition = NULL},
    {.Command = GSM_ATE,         .CommandStrPtr = GSMSETATE,    .Timout = 4, .CallbackPostCondition = NULL},
    {.Command = GSM_QNSTATUS,    .CommandStrPtr = GSMQSTATUS,   .Timout = 4, .CallbackPostCondition = CheckIfNetLoginIsReady}
};
/* 
This is whee we pack the requiered information to execute a command, meaning the corresponding enum, the pointer to the commandstring, 
a timeout and maybe a pointer to a callback to check a certain condition 
*/

/* Also we have the responsestrings here */
const char GSM_RESPONSE_OK_WO_LIMITER[] PROGMEM = "OK";
const char GSM_RESPONSE_OK[] PROGMEM = "OK\r\n";
const char GSM_RESPONSE_CME_ERROR[] PROGMEM = "+CME ERROR:";
const char GSM_RESPONSE_CMS_ERROR[] PROGMEM = "+CMS ERROR:";
const char GSM_RESPONSE_CPMS[] PROGMEM = "+CPMS: ";
const char GSM_RESPONSE_ERROR[] PROGMEM = "ERROR";
const char GSM_RESPONSE_CMGR[] PROGMEM = "+CMGR: ";
const char GSM_RESPONSE_QINITSTAT[] PROGMEM = "+QINISTAT:";
const char GSM_RESPONSE_QNSTATUS[] PROGMEM = "+QNSTATUS:";
const char GSM_RESPONSE_CMGS[] PROGMEM = "+CMGS:";
const char GSM_RESPOANSE_CMGS_PRE[] PROGMEM = "> ";
const char GSM_RESPOANSE_CMGS_PRE_SHORT[] PROGMEM = ">";

PGM_P const pgmGSM_Response_Table[] PROGMEM =
{
    GSM_RESPONSE_OK_WO_LIMITER,
    GSM_RESPONSE_CME_ERROR,
    GSM_RESPONSE_CMS_ERROR,
    GSM_RESPONSE_CPMS,
    GSM_RESPONSE_ERROR,
    GSM_RESPONSE_CMGR,
    GSM_RESPONSE_QINITSTAT,
    GSM_RESPONSE_QNSTATUS,
    GSM_RESPONSE_CMGS,
    GSM_RESPOANSE_CMGS_PRE_SHORT
};

/* Used to send a SMS as Command String */
const char GSMSMSCMDSTR0[] PROGMEM = "AT+CMGS=\"";
const char GSMSMSCMDSTR1[] PROGMEM = "\"\r" ;

/* GSM Modem init swquence */
const GSMATCOMMAND_t InitCommands[] PROGMEM= {
    GSM_QINITSTAT,
    GSM_ATE,
    GSM_AT_IFC,
    GSM_AT_CPMS,
    GSM_AT_CNMI,
    GSM_AT_CMGF,
    GSM_QNSTATUS
};

#endif /* GSM_COMMANDS_H_ */

