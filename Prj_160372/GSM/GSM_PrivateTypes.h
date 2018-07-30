/*
 * GSM_PrivateTypes.h
 *
 * Created: 17.04.2018 13:04:40
 *  Author: mathiasc
 */ 


#ifndef GSM_PRIVATETYPES_H_
#define GSM_PRIVATETYPES_H_

/*

    This will be used as private typedef and enum witch shall not be exported outside the GSM Module 

*/

/* This is to define witch type of errocode we got CME or CMS */ 
typedef enum{
    GSM_CME_ERROR_CODE=0,
    GSM_CMS_ERROR_CODE
}GSM_ERRORCODE_TYPE_t;


/* This are the state for our FSM and must be keept that way */
typedef enum {
    FSM_GSM_IDLE=0,
    FSM_GSM_STARTDETECT,
    FSM_GSM_HEAD_DECODE,
    /* This states are for the URC Codes */
    FSM_GSM_URC_CODE_00_S0,
    FSM_GSM_URC_CODE_01_S0,
    FSM_GSM_URC_CODE_02_S0,
    FSM_GSM_URC_CODE_03_S0,
    FSM_GSM_URC_CODE_04_S0,
    FSM_GSM_URC_CODE_05_S0,
    FSM_GSM_URC_CODE_06_S0,
    FSM_GSM_URC_CODE_07_S0,
    FSM_GSM_URC_CODE_08_S0,
    FSM_GSM_URC_CODE_09_S0,
    FSM_GSM_URC_CODE_10_S0,
    FSM_GSM_URC_CODE_11_S0,
    FSM_GSM_URC_CODE_12_S0,
    FSM_GSM_URC_CODE_13_S0,
    FSM_GSM_URC_CODE_14_S0,
    FSM_GSM_URC_CODE_15_S0,
    FSM_GSM_URC_CODE_16_S0,
    FSM_GSM_URC_CODE_17_S0,
    FSM_GSM_URC_CODE_18_S0,
    FSM_GSM_URC_CODE_19_S0,
    FSM_GSM_URC_CODE_20_S0,
    FSM_GSM_URC_CODE_21_S0,
    FSM_GSM_URC_CODE_22_S0,
    FSM_GSM_URC_CODE_23_S0,
    FSM_GSM_URC_CODE_24_S0,
    FSM_GSM_URC_CODE_25_S0,
    FSM_GSM_URC_CODE_26_S0,
    FSM_GSM_URC_CODE_27_S0,
    FSM_GSM_URC_CODE_28_S0,
    FSM_GSM_URC_CODE_29_S0,
    FSM_GSM_URC_CODE_30_S0,
    FSM_GSM_URC_CODE_31_S0,
    FSM_GSM_URC_CODE_32_S0,
    FSM_GSM_URC_CODE_33_S0,
    FSM_GSM_URC_CODE_34_S0,
    FSM_GSM_URC_CODE_35_S0,
    FSM_GSM_URC_CODE_36_S0,
    FSM_GSM_URC_CODE_37_S0,
    FSM_GSM_URC_CODE_38_S0,
    FSM_GSM_URC_CODE_39_S0,
    FSM_GSM_URC_CODE_41_S0,
    FSM_GSM_URC_CODE_42_S0,
    FSM_GSM_URC_CODE_43_S0,
    /*    Following is the next URC Processing step */
    FSM_GSM_URC_CODE_02_S1,
    FSM_GSM_URC_CODE_02_S2,
    FSM_GSM_URC_CODE_02_S3,
    FSM_GSM_URC_CODE_02_S4,
    FSM_GSM_URC_CODE_02_S5,
    
    FSM_GSM_URC_CODE_03_S1,
    /*                                            */
    FSM_GSM_DECODE_ATRESPONSE,
    FSM_GSM_PARSE_GMGR_READSTATE,
    FSM_GSM_PARSE_GMGR_LINEID,
    FSM_GSM_PARSE_GMGR_PHONEBOOKENTRY,
    FSM_GSM_PARSE_GMGR_TIMESTAMP,
    FSM_GSM_PARSE_GMGR_MESSAGE,
    FSM_GSM_PARSE_CMS_ERROR,
    FSM_GSM_FIND_OK,
    FSM_GSM_SKIPTO_END,
    FSM_GSM_ERROR
} FSM_State_t;

/* struct for the errorcodes used inside the gsm.c */
typedef struct {
    uint16_t CME_Code;
    uint16_t CMS_Code;
    
} GSM_ErrorCodes_t;

/* This are the flags for the stream decoding */
typedef struct {
    uint8_t XOFF_Send:1;
    uint8_t DecodeURC:1;
    uint8_t Reserved:6;    
} GSM_RX_Statusflags_t;
 

/* internal SMS Message structure */
typedef struct {
    char Subscriber[20];    /* Shall be 15 at max according to E.164  plus 00 or +, with 20 we shall be save for a while      */
    uint8_t SubscriberLen;
    char Timestamp[24];    /* Shall be 22 at max and we reserve some byte for the timestamp to may parse it leater      */
    uint8_t TimepstampLen;
    char Message[161];    /* 140 chars with 7 Bit encoding for 140 byte payload of the message, we use 2 bytes reserver */
    uint8_t Messagelen;
} GSM_SMS_MSG_t;

typedef struct {
    uint8_t LocalBuffer[ 32 ];
    uint8_t BufferIndex;
} StreamBuffer_t;

typedef struct {
    uint8_t InitStatusModem;
    uint8_t GSMNetStatus;
} ModemStatus_t;

#endif /* GSM_PRIVATETYPES_H_ */