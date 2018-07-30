/*
 * GSM_PublicTypes.h
 *
 * Created: 17.04.2018 13:01:23
 *  Author: mathiasc
 */ 


#ifndef GSM_PUBLICTYPES_H_
#define GSM_PUBLICTYPES_H_

/* This are the supported SMS Messages we have in FLASH */
typedef enum {
    SMS_POWERLOSS=0,
    SMS_POWERRESTORED,
    SMS_BAT_LOW,
    SMS_BAT_EMPTY,
    SMS_NEW_CONFIG,
    SMS_REQ_STATUS,
} GSMSMSMESSAGE_t;

/* This are the results a Command can have */
typedef enum{
    CMD_OK=0,
    CMD_FAIL,
    CMD_BUSY,
    CMD_PENDING,
    CMD_TIMEOUT,
    CMD_ERROR,
    CMD_CME_ERROR,
    CMD_CMS_ERROR,
    CMD_FAIL_NO_ID,
} GSMCMDResult_t;


/* This are the supported commands we may can issue */
typedef enum {
    GSM_ATE=0,
    GSM_AT_IFC,
    GSM_AT_CPMS,
    GSM_AT_CMGD,
    GSM_AT_CMGS_SMS_START,
    GSM_AT_CMGS,
    GSM_AT_QPOWD,
    GSM_AT_CMGF,
    GSM_AT_CNMI,
    GSM_QINITSTAT,
    GSM_QNSTATUS,
    GSM_NONE /* If we are not expecting an answer */
} GSMATCOMMAND_t;

/* This is the result struct for an command */
typedef struct {
    GSMCMDResult_t GSMCmdResult;
    GSMATCOMMAND_t GSMCMD;
} GSMCMDLastResult_t;



#endif /* GSM_PUBLICTYPES_H_ */