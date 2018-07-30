/*
 * gsm.c
 *
 * Created: 03.04.2018 13:58:17
 *  Author: mathiasc
 */ 

#include <string.h>
#include <stdbool.h>
#include <avr/cpufunc.h>
#include <avr/wdt.h>
#include "..\TIMER\timer.h"
#include "..\USART\fifo.h"
#include "..\USART\usart_hlf.h"
#include "..\EEPROM_FS\EEPFS.h"
#include "..\GPIO\gpio.h"
#include "..\BATTERY_MON\battery_mon.h"
#include "..\MAINS_MON\mains_mon.h"
#include "..\LED_LIGHT\led_light.h"
#include "GSM_PrivateTypes.h"
#include "GSM_URC.h"
#include "GSM_SMS_MSG.h"
#include "GSM_Commands.h"
#include "GSM_SMS_Parser.h"
#include "gsm.h"

#define BUFFER_SIZE ( 128 )

#define BUFFER_HIGH_TH ( BUFFER_SIZE - 4)
#define BUFFER_LOW_TH ( BUFFER_SIZE - 6 )

#define URC_TIMEOUT ( 10 )
#define CMDTIMEOUTCNTLIMIT ( 10 )

#define DBG_FSM
/* For Debug Only */
#ifdef DBG_FSM
    #define DGB_BUFFER_SIZE ( 8 )
    volatile FSM_State_t DBG_StateBuffer[DGB_BUFFER_SIZE] = { FSM_GSM_IDLE, };
    void DBG_StateBufferUpdate( FSM_State_t State ){    
        if(DBG_StateBuffer[DGB_BUFFER_SIZE-1] != State ){
            for(uint8_t i =0 ; i< DGB_BUFFER_SIZE-1 ; i++){
                DBG_StateBuffer[i]=DBG_StateBuffer[i+1];
            }
            DBG_StateBuffer[DGB_BUFFER_SIZE-1]=State;
        }
    }
#endif

/* this structures can be optimized as we use most of the time pointer */
static fifo_t RxFifo;
static uint8_t RxFifoBuffer[ BUFFER_SIZE ];


/*  #########################################################################
    # This is the variable part witch is modem / gsm related                #
    ######################################################################### */ 
/*  
    We define these ones volatile to prevend compiler optimizations and keep it
    fresh in SRAM, also we declare this static as we don't want someone to mess
    with an extern in our memory 
*/
static volatile ModemStatus_t ModemStatus = { .GSMNetStatus =0 , .InitStatusModem=255};
static volatile GSM_ErrorCodes_t LastErrorCode = {.CME_Code=UINT16_MAX,.CMS_Code=UINT16_MAX};
static volatile GSM_SMS_MSG_t tpSmsMessage;



static GSM_RX_Statusflags_t GSM_StreamFlags = {.XOFF_Send=0, .DecodeURC=0, .Reserved=0};
static volatile StreamBuffer_t ParserBuffer;
static volatile  uint8_t CommandTimeout=0; /* Command timout in Seconds, witch will be enough for our commands */
static volatile FSM_State_t FSM_State=FSM_GSM_IDLE;
static volatile GSMATCOMMAND_t tpProcessingAtCommand=GSM_NONE;
static volatile GSMCMDLastResult_t LastCmdResult ={.GSMCMD=GSM_NONE, .GSMCmdResult=CMD_FAIL};
                                                     
static volatile uint8_t CMDTimeoutCounter=0;

/* 
    Function prototypes 
*/

void GSM_RxTask( uint8_t element );
void GSM_FSM_RX_Timout( void );
void GSM_CMDProcessingFinsihed ( GSMCMDResult_t Result, FSM_State_t NextState, bool StopTimer );
void GSM_Process_OK_Response(  uint8_t element );
GSMCMDResult_t GSM_CMDSend( GSMATCOMMAND_t Command );

uint8_t FindNextIdx(uint8_t* Buffer, uint8_t BufferLen, uint8_t idx_start, uint8_t element);
uint16_t ParseInt(uint8_t* Buffer, uint8_t idx_start, uint8_t idx_end );
void GSM_Parse_Errorcode(GSM_ERRORCODE_TYPE_t Type,uint8_t* Buffer, uint8_t idx_start, uint8_t idx_end );


bool GSM_LocBufferAdd( uint8_t element);
void GSM_LocBufferClear( void );
bool GSM_LocBufferInserAtPosition(uint8_t element, uint8_t idx);
void GSM_SetLastCommandResult(GSMCMDResult_t enGSMCmdResult, GSMATCOMMAND_t enGSMCMD);
bool GSM_LocBufferFindDelimiter( void );
bool GSM_LocBufferFindDelimiterAfterIndex( uint8_t start_idx);

void GSM_Delay ( uint8_t delay_s );




/**************************************************************************************************
 *    Function      : voGSM_Init
 *    Description   : This will init the GSM Module, and return if intit was successful
 *    Input         : none 
 *    Output        : bool
 *....Remarks       : This will take up to 120 seconds 
 **************************************************************************************************/
bool GSM_Init(){
    bool InitStatus=true;
    bool ConditionFail=false;
    GSMATCOMMAND_t GSMCommand=GSM_NONE;
    GSMCMDResult_t Result = CMD_BUSY;
    CMDTimeoutCounter=0;
    LED_LIGHT_SetLed(LED_GREEN,LED_BLINK_10Hz);
    TIMER_StopwatchStart( GSMINITTIMER );
    fifo_init( &RxFifo, &RxFifoBuffer[0], sizeof(RxFifoBuffer) );
    
    Set_RX_Hook(GSM_RxData);
    /* First we issue a shutdown if the modem is active */
    if( GPIOHIGH == GPIO_get(STATUS) ){
        GSM_Shutdown();
    }    
    
    /* After this we are ready to proceed with the normal opperation */
    GPIO_set(PWRKEY,GPIOLOW);
    /* We need to do this for at least 2 seconds as we have no STATUS / PCM_SYNC Pin */
    GSM_Delay(2);
    GPIO_set(PWRKEY,GPIOHIGH);
    /* The module is now booting and we wait additional 2 seconds till we send the first commands*/
    TIMER_StopwatchStart( DELAYSTOPWATCH );
    while( (TIMER_StopwatchGetTime( DELAYSTOPWATCH ) < 5 ) && ( GPIOLOW == GPIO_get(STATUS) ) ){
        _NOP();   
    }
    if(GPIOLOW == GPIO_get(STATUS)){
        return false;
    }
    
    /* We now set the FSM to process incomming data and register the RX Hook */    
    tpProcessingAtCommand=GSM_NONE;
    

    for(uint8_t i=0;i<( sizeof(InitCommands)/sizeof(GSMATCOMMAND_t) ); ){
        Result = CMD_BUSY;
        while(Result == CMD_BUSY){
            memcpy_P(&GSMCommand, &InitCommands[i], sizeof(GSMATCOMMAND_t));
            Result = GSM_CMDSend(GSMCommand);
            GSM_Task();
        }
        /* We now wait for the FSM to process the response */
        TIMER_StopwatchStart( DELAYSTOPWATCH );
        while(  ( ( CMD_BUSY==GSM_GetLastCommandResult().GSMCmdResult) || (  CMD_PENDING==GSM_GetLastCommandResult().GSMCmdResult) ) && ( TIMER_StopwatchGetTime( DELAYSTOPWATCH ) < 8 ) ) {
            GSM_Task();
        }
        TIMER_StopwatchStop( DELAYSTOPWATCH );
    
        if(CMD_OK != GSM_GetLastCommandResult().GSMCmdResult){
             if(CMD_TIMEOUT==GSM_GetLastCommandResult().GSMCmdResult){
                 /* We send the command after 2 seconds again */
                 GSM_Delay(2);
             } else {
                 /* We need to decide if we try to reset the whole system */
                 GSM_Delay(2);     
             }    
        } else {
            ConditionFail=false;
            for(uint8_t i=0;i<( sizeof(GSM_COMMAND_TAB) / sizeof( PGM_GSM_Command_Tab_t ) ); i++){
                /* Grab the element form flash */
                        
                if( (GSMCommand == GSM_COMMAND_TAB[i].Command ) && ( GSM_COMMAND_TAB[i].CallbackPostCondition!=NULL) ){
                   if( 0x00 ==  GSM_COMMAND_TAB[i].CallbackPostCondition() ){
                        ConditionFail=false;     
                    } else {
                        ConditionFail=true;
                    }
                    break;
                }
              
            }
                    
            if(true == ConditionFail){
                 GSM_Delay(2);    
            } else {
                i++;
            }
            
        }
        
        /* If we hit a timout we return */
        if( TIMER_StopwatchGetTime( GSMINITTIMER) > 120 ){
            /* We used more than 2 Minutes and will now abort */
            InitStatus = false;
            break;
        }
    }                   
    
    TIMER_StopwatchStop( GSMINITTIMER );
    return InitStatus;
    LED_LIGHT_SetLed(LED_GREEN,LED_STATIC_OFF);
};

/**************************************************************************************************
 *    Function      : voGSM_Shutdown
 *    Description   : This will issue a shutdown sequence
 *    Input         : none 
 *    Output        : none
 *....Remarks       : This will take up to 18 seconds 
 **************************************************************************************************/
void GSM_Shutdown( ) {
    GPIO_set(PWRKEY,GPIOHIGH);
    /* First we issue a power off */
    GSMCMDResult_t Result = CMD_BUSY;
    while(Result == CMD_BUSY){
        Result = GSM_CMDSend(GSM_AT_QPOWD);
        GSM_Task();
    }
    /* We now wait for the FSM to process the response */
    while( CMD_BUSY==GSM_GetLastCommandResult().GSMCmdResult){
        GSM_Task();
    }
    
    if(CMD_OK != GSM_GetLastCommandResult().GSMCmdResult){
        /* We may have an timeout if the system is not responding */
    } else {
    
        /* We should have an OK in response if the Modem is active */

        /* If we need a Net Logout we can have up to 12 seconds delay here */
        TIMER_StopwatchStart( DELAYSTOPWATCH);
        while(TIMER_StopwatchGetTime( DELAYSTOPWATCH )<15){
            _NOP();
            GSM_Task();
        }
        TIMER_StopwatchStop( DELAYSTOPWATCH );
        /* The module shall now be power off */
    }
   LED_LIGHT_SetLed(LED_GREEN,LED_STATIC_OFF);
}



/**************************************************************************************************
 *    Function      : boGetGSMPowerDown
 *    Description   : returns if we are in Powerdown or not
 *    Input         : none 
 *    Output        : bool
 *....Remarks       : none
 **************************************************************************************************/
bool GetGSMPowerDown ( ){
    /* This needs to be swapped with the gpio pin */
    bool GSMPowerDown = false;
    if(GPIOLOW == GPIO_get(STATUS)){
        GSMPowerDown = true;
    } else {
        GSMPowerDown = false;
    }
    
    return GSMPowerDown;
}

/**************************************************************************************************
 *    Function      : voGSM_Delay
 *    Description   : Delay function to sleep for some Seconds within the GSM Module 
 *    Input         : none 
 *    Output        : none
 *....Remarks       : Blocks untill time run out but will keep the GSM FSM running
 **************************************************************************************************/
void GSM_Delay ( uint8_t delay_s ){
    TIMER_StopwatchStart( DELAYSTOPWATCH);
    while(TIMER_StopwatchGetTime( DELAYSTOPWATCH )<delay_s){
        _NOP();
        GSM_Task();
    }
    TIMER_StopwatchStop( DELAYSTOPWATCH );
}



/**************************************************************************************************
 *    Function      : voGSM_RxData
 *    Description   : Function to add new character to the FiFo ( ISR safe )
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void GSM_RxData( uint8_t ch ){
    uint8_t FifoCnt = 0;
    FifoCnt= fifo_get_item_count(&RxFifo);
    if( (FifoCnt >= BUFFER_HIGH_TH) && (0 ==  GSM_StreamFlags.XOFF_Send ) ){
        /* immediate send an XOFF as we can't process any Data anymore! */
        GPIO_set(RTS,GPIOLOW);
        GSM_StreamFlags.XOFF_Send=1;
    }
    if( 0 == fifo_put(&RxFifo,ch) ){
        _NOP();
    }
}


/**************************************************************************************************
 *    Function      : voGSM_FSM_RX_Timout
 *    Description   : To be called if a Timout is detected within the FSM
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void GSM_FSM_RX_Timout( void ){
    FSM_State = FSM_GSM_IDLE;
    if(0 == GSM_StreamFlags.DecodeURC){
        GSM_SetLastCommandResult(CMD_TIMEOUT,tpProcessingAtCommand);
    }
    tpProcessingAtCommand=GSM_NONE;
    CommandTimeout = 0;
    /* Set Flags for CMD timout ... */
    TIMER_StopwatchStop( GSMTIMOUTSTOPWATCH );
    
}


/**************************************************************************************************
 *    Function      : voGSM_Task
 *    Description   : Cyclic to be called Task to keep the com with the modem alive
 *    Input         : none 
 *    Output        : none
 *....Remarks       : must be called at least within 1ms invervals @9600 baud
 **************************************************************************************************/
void GSM_Task(){
    int16_t s16fiforesponse=-1; /* Prepopulated with fifo empty */
    s16fiforesponse = fifo_get_nowait(&RxFifo);
     /* First we check if we have any timouts for processed commands */
    if( ( GSM_NONE != tpProcessingAtCommand ) && ( STOPWATCHSTART != TIMER_StopwatchStatus( GSMTIMOUTSTOPWATCH ) )) {
            GSM_FSM_RX_Timout();
    }  
    
    if( (1 == GSM_StreamFlags.DecodeURC ) && (  STOPWATCHSTART != TIMER_StopwatchStatus( GSMTIMOUTSTOPWATCH ) ) ){
                        /* This showed that something went horribly wrong */
                       GSM_StreamFlags.DecodeURC = 0;
    }
          
    if(  (CommandTimeout > 0 ) && ( STOPWATCHSTART == TIMER_StopwatchStatus( GSMTIMOUTSTOPWATCH ) ) ){
        if( ( TIMER_StopwatchGetTime( GSMTIMOUTSTOPWATCH ) > ( CommandTimeout ) )  ){
            GSM_FSM_RX_Timout();
        }
    }
    /* We need to determine if we had something in the fifo */
    if( (-1) < s16fiforesponse  ){
        GSM_RxTask( (uint8_t)s16fiforesponse ); /* This process all incomming data */
        if( ( 1 == GSM_StreamFlags.XOFF_Send ) && (fifo_get_item_count(&RxFifo) < BUFFER_LOW_TH ) ){
                
                GPIO_set(RTS,GPIOHIGH);
                GSM_StreamFlags.XOFF_Send=0;
                
        } else {
            _NOP();
        }
    }    
    
    /* Last part is to decide if we have to decare the com with the modem as dead and need to do a reinit */
    if(CMDTIMEOUTCNTLIMIT <= CMDTimeoutCounter ){
         wdt_enable(WDTO_2S);
            while ( 1 == 1 ){
                _NOP();
            }         
    }
    /* Last but not least we may have to care about the LED Task here */
    /* as the GSM Task is called besides the main loop in a periodic manner */
    LED_LIGHT_Task(); /* This can set the led to blink at the modem init */
}


/**************************************************************************************************
 *    Function      : voGSM_RxTask
 *    Description   : RX Task to process incomming chars one by one
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void GSM_RxTask( uint8_t element ){   
/* 
    This is the primary FSM for the communication with the M95 
    due to the size limit of the AVR used is dose only the bare
    necessity's to keep the modem alive, also this means it is 
    more ugly than it should be to sqeeze out a few byte of code
    usage to get it in.
*/
#ifdef DBG_FSM
    DBG_StateBufferUpdate(FSM_State);
#endif

            switch(FSM_State){
                case FSM_GSM_IDLE:{
                    /* We add some code for a bit more robustness */
                    /* If we are idle, processing a command and no timeout is active we will start one */
                    if('\r'==element){
                        FSM_State=FSM_GSM_STARTDETECT;
                    } else {
                        FSM_State=FSM_GSM_IDLE;
                    }
                } break;
                
                case FSM_GSM_STARTDETECT:{
                    /* We need to receive a '\n' here */
                    if('\n' == element){
                        GSM_LocBufferClear();
                        /* at this point we need to decide if we expect a AT response or if this is a URC                       */
                        /* With the modem there is the rare case that we get an URC even if it shall respond to an AT Command   */
                        /* This is not completly according to Datasheet                                                         */
                        CommandTimeout = URC_TIMEOUT;
                        TIMER_StopwatchStart( GSMTIMOUTSTOPWATCH );
                        FSM_State=FSM_GSM_HEAD_DECODE;
                   } else if ('\r'==element){
                    /* We stay here */
                     FSM_State = FSM_GSM_STARTDETECT;
                    } else {
                        /* We fall back to IDLE */
                        FSM_State=FSM_GSM_IDLE;
                    }                
                } break;
                
                case FSM_GSM_HEAD_DECODE:{
                        if(true==GSM_LocBufferAdd(element) ){
                            /* We walk through all possible responses we may get and see what it might be */
                            if( true == GSM_LocBufferFindDelimiter() ){
                                /* Okay we have reached the end of the String and not found a valid command in here */
                                FSM_State=FSM_GSM_IDLE;
                            } else {
                                for(uint8_t idx=0;idx<43;idx++){
                            
                                    int16_t s16cmpresult = strncmp_P( (const char*)(&ParserBuffer.LocalBuffer[0]) ,(PGM_P)pgm_read_word(&(pgmGSM_URC_Table[idx])),32);
                                    /* if the buffer an the specific content in flash are equal we will continue there */
                                    if(0==s16cmpresult){
                                        /* Mark the hit and then continue to parse the response accordingly            */
                                        FSM_State=FSM_GSM_URC_CODE_00_S0+idx;
                                        break;
                                    } 
                                }
                                /* We check now for OK, ERROR and some other codes .... */
                                for(uint8_t idx=0;idx<10;idx++){
                                     int16_t s16cmpresult = strncmp_P( (const char*)(&ParserBuffer.LocalBuffer[0]) ,(PGM_P)pgm_read_word(&(pgmGSM_Response_Table[idx])),32);
                                     if(0==s16cmpresult){
                                         /* Mark the hit and then continue to parse the response accordingly            */
                                         /* Here it gets tricky as we need to jump to the current state for the command */
                                             FSM_State=FSM_GSM_DECODE_ATRESPONSE;
                                             GSM_StreamFlags.DecodeURC=0;
                                         break;
                                     }
                                }
                                
                                
                            }
                        } else {
                            /* Something went wrong and we switch to a different state*/
                            GSM_LocBufferClear();
                            GSM_LocBufferInserAtPosition(element,0);
                            FSM_State=FSM_GSM_SKIPTO_END;
                        }
                    
                } break;
    /*###############################################################################################################*/
    /*# This is the Part for the URC States. We handel as less as possible to save flash                            #*/
    /*###############################################################################################################*/
                case    FSM_GSM_URC_CODE_00_S0:{
                    /* This will be a CMTI: "SM",xxx\n\r */
                    /* With the current confiuration we won't expect that */    
                    if(true==GSM_LocBufferAdd(element) ){
                        /* We walk through all possible responses we may get and see what it might be */
                        if( true == GSM_LocBufferFindDelimiter() ){
                            FSM_State = FSM_GSM_IDLE;
                           }
                    }
                }break;
                /* This is for SMS reception */
                case    FSM_GSM_URC_CODE_01_S0:
                case    FSM_GSM_URC_CODE_02_S0:{
                    /* Same states, this is done by intention and assumed AT+CNMI=2,2 and encoding is text*/
                    /* The Buffer is cleared and we are running almost the same as if we were reading an sms */
                    if('"' == element){
                        memset( (void*)(&tpSmsMessage) , 0 , sizeof(GSM_SMS_MSG_t) );
                        FSM_State = FSM_GSM_URC_CODE_02_S1;
                    } else if( ' ' == element) {
                        GSM_LocBufferClear();
                    } else {
                        FSM_State = FSM_GSM_IDLE;
                    }
                    
                }break;
                
                case FSM_GSM_URC_CODE_02_S1:{
                    /* We now read the Subscriber ID */
                    if(',' == element){
                        /* Jump to next state */
                        FSM_State = FSM_GSM_URC_CODE_02_S2;
                    } else {
                            /* We process + and 0 to 9 */
                            if( ('+' == element ) || ( ( element >= '0') && ( element <='9') ) ){        
                                if(tpSmsMessage.SubscriberLen<20){
                                    tpSmsMessage.Subscriber[tpSmsMessage.SubscriberLen]=element;
                                    tpSmsMessage.SubscriberLen++;
                                } 
                            } else {
                                /* we don't process the element */                    
                            }
                    }
                    
                } break;
                
                case FSM_GSM_URC_CODE_02_S2:{
                    /* We skip the phonebook entry if there is any */
                    if(',' == element){
                        /* Jump to next state */
                        FSM_State = FSM_GSM_URC_CODE_02_S3;
                        GSM_LocBufferClear();
                    } else { 
                        /* We keep the state */
                    }
                } break;
                
                case FSM_GSM_URC_CODE_02_S3:{
                    if( '\r' == element ){
                        FSM_State = FSM_GSM_URC_CODE_02_S4; 
                        GSM_LocBufferClear();    
                    } else {
                        if( ('\n' != element) && ( '"' != element )){
                            if(tpSmsMessage.TimepstampLen<24){
                                tpSmsMessage.Timestamp[tpSmsMessage.TimepstampLen]=element;
                                tpSmsMessage.TimepstampLen++;
                            }    
                        }
                    }
                    
                } break;
                
                case FSM_GSM_URC_CODE_02_S4:{
                    /* This will be the main message */
                    if( '\r' == element ){
                        FSM_State = FSM_GSM_URC_CODE_02_S5;
                        GSM_LocBufferClear();
                        voGSM_ParseSMS((GSM_SMS_MSG_t*)&tpSmsMessage);
                        FSM_State = FSM_GSM_IDLE;
                        
                    } else {
                        if('\n' != element){
                            if(tpSmsMessage.Messagelen< ( sizeof( (GSM_SMS_MSG_t*)0)->Message) ){
                                tpSmsMessage.Message[tpSmsMessage.Messagelen]=element;
                                tpSmsMessage.Messagelen++;
                            }
                        }
                    }
                    
                } break;
                /* States for SMS reception end here */

                case    FSM_GSM_URC_CODE_03_S0:
                    /* We ignore the message waiting for two \r\n to be found  in the stream */
                case    FSM_GSM_URC_CODE_04_S0:
                case    FSM_GSM_URC_CODE_05_S0:{                
                    if('\n' == element ){
                        GSM_LocBufferClear();
                        FSM_State = FSM_GSM_SKIPTO_END;
                    }
                
                }break;
                
                case    FSM_GSM_URC_CODE_06_S0:
                case    FSM_GSM_URC_CODE_07_S0:
                case    FSM_GSM_URC_CODE_08_S0:
                case    FSM_GSM_URC_CODE_09_S0:
                case    FSM_GSM_URC_CODE_10_S0:
                case    FSM_GSM_URC_CODE_11_S0:
                case    FSM_GSM_URC_CODE_12_S0:
                case    FSM_GSM_URC_CODE_13_S0:
                case    FSM_GSM_URC_CODE_14_S0:
                case    FSM_GSM_URC_CODE_15_S0:
                case    FSM_GSM_URC_CODE_16_S0:
                case    FSM_GSM_URC_CODE_17_S0:
                case    FSM_GSM_URC_CODE_18_S0:
                case    FSM_GSM_URC_CODE_19_S0:
                case    FSM_GSM_URC_CODE_20_S0:
                case    FSM_GSM_URC_CODE_21_S0:
                case    FSM_GSM_URC_CODE_22_S0:
                case    FSM_GSM_URC_CODE_23_S0:
                    /* This is a RING and we need to hang up */
                case    FSM_GSM_URC_CODE_24_S0:
                case    FSM_GSM_URC_CODE_25_S0:
                    /* UNSERVOLATE POWER DOWN */
                case    FSM_GSM_URC_CODE_26_S0:
                    /* Unservoltage Warning */
                case    FSM_GSM_URC_CODE_27_S0:
                    /* Overvoltage Power down */
                case    FSM_GSM_URC_CODE_28_S0:
                    /* Overvoltage Warning */
                case    FSM_GSM_URC_CODE_29_S0:
                    /* Normal Powerdown */
                case    FSM_GSM_URC_CODE_30_S0:
                case    FSM_GSM_URC_CODE_31_S0:
                case    FSM_GSM_URC_CODE_32_S0:
                case    FSM_GSM_URC_CODE_33_S0:
                case    FSM_GSM_URC_CODE_34_S0:
                case    FSM_GSM_URC_CODE_35_S0:
                case    FSM_GSM_URC_CODE_36_S0:
                    /* RDY Information */
                case    FSM_GSM_URC_CODE_37_S0:
                    /*CFUN:1 */
                case    FSM_GSM_URC_CODE_38_S0:
                case    FSM_GSM_URC_CODE_39_S0:
                case    FSM_GSM_URC_CODE_41_S0:
                case    FSM_GSM_URC_CODE_42_S0:
                case    FSM_GSM_URC_CODE_43_S0:{
                    if('\n' ==element ){
                         GSM_StreamFlags.DecodeURC=0;
                        FSM_State = FSM_GSM_IDLE;
                    }else {
                        FSM_State = FSM_GSM_SKIPTO_END;
                    }                    
                }break;
                
                /* This part of the FSM is for response we expect for AT Commands */
    /*###############################################################################################################*/
    /*# This is the Part for the URC States. We handel as less as possible to save flash                            #*/
    /*###############################################################################################################*/
                
                case FSM_GSM_DECODE_ATRESPONSE:{
                    /* We need to see if we have transmitted an at command and what to expect */
                    switch(tpProcessingAtCommand){
                        
                        case GSM_AT_CMGD:{
                            /* We expect OK / ERROR or +CMS ERROR: <idx>    */
                            /* We shall have a response within 300ms        */
                            if(true==GSM_LocBufferAdd(element)){
                                /* Element is in Buffer we will do a quick strncmp with the expected responses */
                                if(true==GSM_LocBufferFindDelimiter()){
                                    /* Okay we can now pare the elements in the buffer and compare it with the strings form Flash (sortof) */
                                    if( strncmp_P((const char*)&ParserBuffer.LocalBuffer[0],GSM_RESPONSE_OK,4) ){
                                        /* Okay we have an OK as priarily expected */
                                        GSM_CMDProcessingFinsihed(CMD_OK,FSM_GSM_IDLE,true);
                                    }else if ( 0 == strncmp_P((const char*)&ParserBuffer.LocalBuffer[0],GSM_RESPONSE_ERROR,5) ){
                                        /* Something went wrong with the command */
                                        GSM_CMDProcessingFinsihed(CMD_ERROR,FSM_GSM_IDLE,true);
                                        /* We need to report that                 */
                                    } else if ( 0 == strncmp_P((const char*)&ParserBuffer.LocalBuffer[0],GSM_RESPONSE_CME_ERROR,11)) {
                                    /* We need to pare the index given, if any is given .... */
                                        GSM_Parse_Errorcode(GSM_CME_ERROR_CODE,(uint8_t*)&ParserBuffer.LocalBuffer[0],12,4);
                                        GSM_CMDProcessingFinsihed(CMD_CMS_ERROR,FSM_GSM_IDLE,false);
                                    } else {
                                    /* We have something unexpected here and need to handle it */
                                        GSM_CMDProcessingFinsihed(CMD_ERROR,FSM_GSM_IDLE,true);
    
                                    }
                               }
                            }
                            
                            
                        } break;
                        
                        /* This sets the SMS Store, to be determined if this is requiered after all */
                        case GSM_AT_CPMS:{
                        /* We expect an     +CPMS: 0,50,0,50,0,50\r\nOK\r\n                                            */
                        /* or we have an +CMS ERROR: <idx>                                                            */
                        if(true==GSM_LocBufferAdd(element)){
                            /* Element is in Buffer we will do a quick strncmp with the expected responses */
                            if(true==GSM_LocBufferFindDelimiter()){
                                /* Okay we can now parse the elements in the buffer and compare it with the strings form Flash (sortof) */
                                if( 0 == strncmp_P((const char*)&ParserBuffer.LocalBuffer[0],GSM_RESPONSE_CPMS,7) ){
                                    /* the string will now be parsed for its limits */
                                    GSM_LocBufferClear();    
                                    /* At the end we also expect an OK in there                              */
                                    FSM_State = FSM_GSM_FIND_OK;
                                    
                                    } else if ( 0 == strncmp_P((const char*)&ParserBuffer.LocalBuffer[0],GSM_RESPONSE_CME_ERROR,11)) {
                                          /* There was en error regarding the command, we try to parse this.....*/
                                        GSM_Parse_Errorcode(GSM_CME_ERROR_CODE,(uint8_t*)&ParserBuffer.LocalBuffer[0],12,4);
                                        GSM_CMDProcessingFinsihed(CMD_CME_ERROR,FSM_GSM_IDLE,true);
                                    } else if ( 0 == strncmp_P((const char*)&ParserBuffer.LocalBuffer[0],GSM_RESPONSE_CMS_ERROR,11)){
                                        GSM_Parse_Errorcode(GSM_CMS_ERROR_CODE,(uint8_t*)&ParserBuffer.LocalBuffer[0],12,4);
                                        GSM_CMDProcessingFinsihed(CMD_CME_ERROR,FSM_GSM_IDLE,true);
                                    } else {
                                    /* We have something unexpected here and need to handle it */
                                        GSM_CMDProcessingFinsihed(CMD_ERROR,FSM_GSM_IDLE,true);
                                }
                            }
                        }    
                            
                        } break;

                        case GSM_AT_QPOWD:
                        case GSM_AT_IFC:
                        case GSM_ATE:
                        case GSM_AT_CMGF:
                        case GSM_AT_CNMI:{
                            GSM_Process_OK_Response( element);
                        }break;
                        
                        case GSM_AT_CMGS_SMS_START:{
                            if(true==GSM_LocBufferAdd(element)){
                                if( 0 == strncmp_P((const char*)&ParserBuffer.LocalBuffer[0],GSM_RESPOANSE_CMGS_PRE,2) ) {
                                    GSM_CMDProcessingFinsihed(CMD_OK,FSM_GSM_IDLE,true);
                                    GSM_LocBufferClear();         
                                }
                            }
                        } break;

                        /* Response if an SMS has been send */
                        case GSM_AT_CMGS:{
                            /* We accept +CMGS: <mr>\n\rOK or +CMS ERROR: <err> */
                            /* This can take up to 15 seconds to get a response  */
                            if(true==GSM_LocBufferAdd(element)){
                                /* Element is in Buffer we will do a quick strncmp with the expected responses */
                                if(true==GSM_LocBufferFindDelimiter()){
                                    /* Okay we can now pare the elements in the buffer and compare it with the strings form Flash (sortof) */
                                    if( 0 == strncmp_P((const char*)&ParserBuffer.LocalBuffer[0],GSM_RESPONSE_CMGS,6) ){
                                        /* Okay we have an CPMS as primarily expected */
                                        /* We may need to parse the store information but can also skip the rest */
                                        GSM_LocBufferClear();
                                        GSM_CMDProcessingFinsihed(CMD_PENDING,FSM_GSM_FIND_OK,false);
                                    } else if ( 0 == strncmp_P((const char*)&ParserBuffer.LocalBuffer[0],GSM_RESPONSE_CME_ERROR,11) ) {
                                        GSM_Parse_Errorcode(GSM_CMS_ERROR_CODE,(uint8_t*)&ParserBuffer.LocalBuffer[0],12,4);
                                        GSM_CMDProcessingFinsihed(CMD_CME_ERROR,FSM_GSM_IDLE,true);
                                    } else if ( 0 == strncmp_P((const char*)&ParserBuffer.LocalBuffer[0],GSM_RESPONSE_CMS_ERROR,11) ){
                                       GSM_Parse_Errorcode(GSM_CMS_ERROR_CODE,(uint8_t*)&ParserBuffer.LocalBuffer[0],12,4);
                                       GSM_CMDProcessingFinsihed(CMD_CMS_ERROR,FSM_GSM_IDLE,true);
                                    } else {
                                        GSM_CMDProcessingFinsihed(CMD_ERROR,FSM_GSM_IDLE,true);
                                    }
                                }
                            }
                            
                            
                        } break;
                    
                        case GSM_QINITSTAT:{
                            
                            if(true==GSM_LocBufferAdd(element)){
                                /* Element is in Buffer we will do a quick strncmp with the expected responses */
                                if(true==GSM_LocBufferFindDelimiterAfterIndex(10)){
                                    /* Okay we can now pare the elements in the buffer and compare it with the strings form Flash (sortof) */
                                    if( 0 == strncmp_P((const char*)&ParserBuffer.LocalBuffer[0],GSM_RESPONSE_QINITSTAT,10) ){
                                        /* Okay we have an OK as priarily expected */
                                        ModemStatus.InitStatusModem=ParseInt( (uint8_t*)&ParserBuffer.LocalBuffer[0], 11 , 12 );
                                        GSM_LocBufferClear();    
                                        GSM_CMDProcessingFinsihed(CMD_PENDING,FSM_GSM_FIND_OK,false);
                                        
                                        } else {
                                        /* We have something unexpected here and need to handle it */
                                        GSM_CMDProcessingFinsihed(CMD_ERROR,FSM_GSM_IDLE,true);
                                    }
                                }
                            }
                        
                        }break;
                        
                        case GSM_QNSTATUS:{
                        if(true==GSM_LocBufferAdd(element)){
                                /* Element is in Buffer we will do a quick strncmp with the expected responses */
                                if(true==GSM_LocBufferFindDelimiter()){
                                    /* Okay we can now pare the elements in the buffer and compare it with the strings form Flash (sortof) */
                                    if( 0 == strncmp_P((const char*)&ParserBuffer.LocalBuffer[0],GSM_RESPONSE_QNSTATUS,10) ){
                                        /* Okay we have an OK as priarily expected */
                                        ModemStatus.GSMNetStatus=ParseInt( (uint8_t*)&ParserBuffer.LocalBuffer[0], 11 , 12 );
                                        GSM_LocBufferClear();    
                                        GSM_CMDProcessingFinsihed(CMD_PENDING,FSM_GSM_FIND_OK,false);
                                        
                                        } else {
                                        /* We have something unexpected here and need to handle it */
                                        GSM_CMDProcessingFinsihed(CMD_ERROR,FSM_GSM_IDLE,true);
                                    }
                                }
                            }
                        
                        } break;
                         
                        case GSM_NONE:
                        default:{
                        
                            /* this indicates a fatal error */
                            GSM_CMDProcessingFinsihed(CMD_ERROR,FSM_GSM_IDLE,true);
                        }break;
                    }
                    
                    
                    
                    
                } break;
                
                case FSM_GSM_PARSE_CMS_ERROR:{
                    /* We expect " x..x\r\n" */
                    
                    if(true==GSM_LocBufferAdd(element)){
                        /* Element is in Buffer we will do a quick strncmp with the expected responses */
                        if(true==GSM_LocBufferFindDelimiter()){
                                GSM_CMDProcessingFinsihed(CMD_ERROR,FSM_GSM_IDLE,true);                        
                        }    
                    }
                    
                } break;
                
                case FSM_GSM_FIND_OK:{ 
                    /* If the response has multiple lines we will try to find the last OK in it */
                    if(true==GSM_LocBufferAdd(element)){
                        /* Element is in Buffer we will do a quick strncmp with the expected responses */
                        if(true==GSM_LocBufferFindDelimiterAfterIndex(2)){
                            /* Okay we can now pare the elements in the buffer and compare it with the strings form Flash (sortof) */
                            if( ( ParserBuffer.BufferIndex>4 ) && ( 0 == strncmp_P((const char*)&ParserBuffer.LocalBuffer[ParserBuffer.BufferIndex-4],GSM_RESPONSE_OK,4) ) ){
                                /* Okay we have an OK as primarily expected */
                                GSM_CMDProcessingFinsihed(CMD_OK,FSM_GSM_IDLE,true);
                            } else {
                                /* We have something unexpected here and need to handle it */
                                /* Mark the response as inclomplete                           */
                                GSM_CMDProcessingFinsihed(CMD_ERROR,FSM_GSM_IDLE,true);
                            }
                        }
                    }
                } break;
                
                case FSM_GSM_SKIPTO_END:{
                    /* If we are here we have the last char received in the buffer and can grep the    */
                    /* current one from the element var                                                */
                        if( '\n' == element ){
                            /* We found the end and can get back to IDEL */
                            TIMER_StopwatchStop( GSMTIMOUTSTOPWATCH );
                            GSM_LocBufferClear();
                            if(1 == GSM_StreamFlags.DecodeURC ) {
                                GSM_StreamFlags.DecodeURC=0;
                                if( GSM_NONE != tpProcessingAtCommand){
                                    /* Restart the Timer as we are still processing an GSM Command */
                                    TIMER_StopwatchStart( GSMTIMOUTSTOPWATCH);
                                }
                                
                            } else {
                                tpProcessingAtCommand = GSM_NONE;
                            }  
                            FSM_State = FSM_GSM_IDLE;                              
                            
                        } 
                }break;
                
                case FSM_GSM_ERROR:{
                    GSM_CMDProcessingFinsihed(CMD_FAIL,FSM_GSM_IDLE,true);
                } break;
                
                default:{
                    GSM_CMDProcessingFinsihed(CMD_FAIL,FSM_GSM_IDLE,true);
                }
            } /* End of Switch */
}


/**************************************************************************************************
 *    Function      : voGSM_Process_OK_Response
 *    Description   : This is a common part of the FSM to process the "OK" response 
 *                    this will take the current char from the modem as argument
 *    Input         : uint8_t 
 *    Output        : none
 *....Remarks       : Shifted to a function to safe space and have only one place to maintain 
 **************************************************************************************************/
void GSM_Process_OK_Response( uint8_t element ){
    /* If a command expects only a "OK" at all we use this function */
    /* Done to save a few bytes code                                */
    /* The response shall be OK */
    /* an is expected within 300ms */
    if(true==GSM_LocBufferAdd(element)){
        /* Element is in Buffer we will do a quick strncmp with the expected responses */
        if(true==GSM_LocBufferFindDelimiter()){
            /* Okay we can now pare the elements in the buffer and compare it with the strings form Flash (sortof) */
            if( 0 == strncmp_P((const char*)&ParserBuffer.LocalBuffer[0],GSM_RESPONSE_OK,4) ){
                /* Okay we have an OK as priarily expected */
                GSM_CMDProcessingFinsihed(CMD_OK,FSM_GSM_IDLE,true);
                } else if( 0 == strncmp_P((const char*)&ParserBuffer.LocalBuffer[0],GSM_RESPONSE_CME_ERROR,11)){
                GSM_Parse_Errorcode(GSM_CME_ERROR_CODE,(uint8_t*)&ParserBuffer.LocalBuffer[0],12,4);
                GSM_CMDProcessingFinsihed(CMD_CME_ERROR,FSM_GSM_IDLE,true);
                } else if ( 0 == strncmp_P((const char*)&ParserBuffer.LocalBuffer[0],GSM_RESPONSE_CMS_ERROR,11) ){
                GSM_Parse_Errorcode(GSM_CMS_ERROR_CODE,(uint8_t*)&ParserBuffer.LocalBuffer[0],12,4);
                GSM_CMDProcessingFinsihed(CMD_CMS_ERROR,FSM_GSM_IDLE,true);
                } else {
                GSM_CMDProcessingFinsihed(CMD_ERROR,FSM_GSM_IDLE,true);
                /* We have something unexpected here and need to handle it */
            }
        }

    }
}

/**************************************************************************************************
 *    Function      : voGSM_SetLastCommandResult
 *    Description   : This will set the result of the last executed command
 *    Input         : GSM_CMD_Result_t, GSM_AT_COMMAND_t
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void GSM_SetLastCommandResult(GSMCMDResult_t GSMCmdResult, GSMATCOMMAND_t GSMCMD){
    /* 
        We have here a mechanism to detect if communication to the modem is totally lost 
        If 10 Commands in a row time out we assume that we need to do a complete system restat
    */
    if(CMD_TIMEOUT == LastCmdResult.GSMCmdResult ){
        CMDTimeoutCounter++;
    } else {
       CMDTimeoutCounter=0;
    }
    LastCmdResult.GSMCMD = GSMCMD;
    LastCmdResult.GSMCmdResult = GSMCmdResult;
}

/**************************************************************************************************
 *    Function      : voGSM_CMDProcessingFinsihed
 *    Description   : Common function used by the FSM to set the result of a command after it has
 *                    been finished processing, also switches th FSM into the next state
 *    Input         : GSM_CMD_Result_t, FSM_State_t, bool
 *    Output        : none
 *....Remarks       : can stop GSM_TIMOUT_STOPWATCH it last argument is true 
 **************************************************************************************************/
void GSM_CMDProcessingFinsihed ( GSMCMDResult_t Result, FSM_State_t NextState, bool StopTimer ) {
    GSM_SetLastCommandResult(Result,tpProcessingAtCommand);
    if(StopTimer != false){
        tpProcessingAtCommand=GSM_NONE;
        TIMER_StopwatchStop( GSMTIMOUTSTOPWATCH );
    }
    FSM_State = NextState;
}

/*###########################################################################################################
  #  Here will be the functions used to modify the local string buffer and function for string parsing      #
  #                                                                                                         #
  ###########################################################################################################*/

/**************************************************************************************************
 *    Function      : enGSM_Send_SMS_P
 *    Description   : This will try to send an SMS from FLASH 
 *    Input         : GSM_SMS_MESSAGE_P
 *    Output        : GSM_CMD_Result_t
 *....Remarks       : none
 **************************************************************************************************/
GSMCMDResult_t GSM_Send_SMS_P( GSMSMSMESSAGE_t Msg ){    
    /* This shall send a SMS to a remote location with the correct command */
    /* We need to send some strings from Flash and also some from RAM       */
    /* This is used for complete user generated SMS                           */
    GSMCMDResult_t Result=CMD_FAIL;
    EEPFS_RemoteLineID_t RemoteID;
    
    if(GSM_NONE != tpProcessingAtCommand){
        /* We have a Command still processing and return an error */
        Result=CMD_FAIL;
        } else {
            
        if( true == GetGSMPowerDown() ){
           if( false == GSM_Init() ){
               return CMD_FAIL;
           } 
        }
            
        /* We can start an Transmission but need to check if we have a LineID to send to */
        if( true == EEPFS_ReadRemoteLineID(&RemoteID) ){
            if(strnlen(RemoteID.LineID,20) > 0 ){
                /* We now can start the TX Process and take as much form flash as possible, as ram is a rare resource */
                /* AT+CMGS="[LineID]"<cr>"Message"<STRG-Z>    */
                usart_putp(GSMSMSCMDSTR0);
                for(uint8_t i=0;i<20;i++){
                    if(RemoteID.LineID[i]!='\0'){
                        usart_putc(RemoteID.LineID[i]);
                    } else {
                        break;
                    }
                }
                usart_putp(GSMSMSCMDSTR1);
                CommandTimeout=4;
                tpProcessingAtCommand=GSM_AT_CMGS_SMS_START;
                GSM_SetLastCommandResult(CMD_BUSY,tpProcessingAtCommand);
                TIMER_StopwatchStart( GSMTIMOUTSTOPWATCH);
                    /* We now wait for the FSM to process the response */
                 TIMER_StopwatchStart( DELAYSTOPWATCH );
                 while(  ( ( CMD_BUSY==GSM_GetLastCommandResult().GSMCmdResult) || (  CMD_PENDING==GSM_GetLastCommandResult().GSMCmdResult) ) && ( TIMER_StopwatchGetTime( DELAYSTOPWATCH ) < 8 ) ) {
                     GSM_Task();
                 }
                 TIMER_StopwatchStop( DELAYSTOPWATCH );    
           
                if(CMD_OK != GSM_GetLastCommandResult().GSMCmdResult){
                    usart_putc(27); /* Send ESC to stop transmission */
                    return CMD_FAIL;
                }
        
                    switch(Msg){
                        
                        case SMS_BAT_EMPTY:{
                            usart_putp(GSM_SMS_BATTERY_EMPTY_MSG);
                        } break;
                        
                        case SMS_BAT_LOW:{
                            usart_putp(GSM_SMS_BATTERY_LOW_MSG);
                        } break;
                        
                        case SMS_POWERLOSS:{
                            usart_putp(GSM_SMS_POWERLOSS_MSG);
                        } break;
                        
                        case SMS_POWERRESTORED:{
                            usart_putp(GSM_SMS_POWERRESTORED_MSG);
                        } break;
                        
                        case SMS_NEW_CONFIG:{
                            usart_putp(GSM_SMS_NEW_CONFIG_MSG);
                        } break;
                        
                        case SMS_REQ_STATUS:{
                            BatteryStatus_t Bat = BATTERY_MON_GetInfo();
                            MAINS_MON_Status_t MSt = MAINS_MON_GetStatus();
                            usart_putp(GSM_SMS_STATUS_P0);
                            if( MAINS_OK == MSt.Status ){
                                usart_putp(GSM_SMS_STATUS_P_OKAY);
                            } else {
                                usart_putp(GSM_SMS_STATUS_P_FAULTY);
                            }
                            usart_putp(GSM_SMS_STATUS_P1);
                          
                            if( 55 >  MAINS_MON_GetStatus().FreqDeziHz ){
                                usart_puti( 50 );
                            } else {
                               usart_puti( 60 );
                            }                               
                            usart_putp(GSM_SMS_STATUS_P2);
                            
                            usart_puti( BATTERY_MON_GetInfo().VBat );
                            
                            usart_putp(GSM_SMS_STATUS_P3);
                            
                            switch( Bat.Batterystatus ){
                                case VBAT_EMPTY:{
                                      usart_putp(GSM_SMS_STATUS_P_LOW);
                                } break;
                                
                                case VBAT_LOW:{
                                    usart_putp(GSM_SMS_STATUS_P_LOW);
                                } break;
                                
                                case VBAT_OK:{
                                    usart_putp(GSM_SMS_STATUS_P_OKAY);
                                }break;
                            }
                            
                        }break;
                        
                        default:{
                            
                        }break;
                        
                    }
                /* We finish the message with STRG-Z ( meaning 0x1A )*/
                usart_putc(0x1A);
                /* Response will take up to 120 seconds        */
                tpProcessingAtCommand=GSM_AT_CMGS;
                CommandTimeout=120;
                TIMER_StopwatchStart( GSMTIMOUTSTOPWATCH) ;
                Result=CMD_OK;
                } else {
                    /* We have no config in here and need to give up */
                     Result=CMD_FAIL_NO_ID;
                }
            } else {
            /* We can't read the LineID and must give up here */
                Result=CMD_FAIL_NO_ID;
        }
    }
    return Result;    
}

/**************************************************************************************************
 *    Function      : enGSM_CMDSend
 *    Description   : This will issue a command to the modem if possible 
 *    Input         : GSM_AT_COMMAND_t
 *    Output        : GSM_CMD_Result_t
 *....Remarks       : none
 **************************************************************************************************/
GSMCMDResult_t GSM_CMDSend( GSMATCOMMAND_t Command ){
    GSMCMDResult_t Result=CMD_FAIL;
    if(fifo_get_item_count(&RxFifo) != 0){
        return CMD_BUSY;
    }
    if( (GSM_NONE==tpProcessingAtCommand) && ( ( FSM_GSM_IDLE == FSM_State ) || ( FSM_GSM_STARTDETECT == FSM_State ) ) ){
        for(uint8_t i=0;i<( sizeof(GSM_COMMAND_TAB) / sizeof( PGM_GSM_Command_Tab_t ) ); i++){
            /* Grab the element form flash */
            
            if(Command == GSM_COMMAND_TAB[i].Command ){
                usart_putp(GSM_COMMAND_TAB[i].CommandStrPtr);
                usart_putc('\r');
                tpProcessingAtCommand=Command;
                GSM_SetLastCommandResult(CMD_BUSY,tpProcessingAtCommand);
                TIMER_StopwatchStart( GSMTIMOUTSTOPWATCH );
                CommandTimeout=GSM_COMMAND_TAB[i].Timout;
                Result=CMD_OK;
                break;
            }
        }
    } else {
         Result=CMD_BUSY;
    }
    return Result;
    
}

/*###########################################################################################################
  #  Here will be the functions used to modify the local string buffer and function for string parsing      #
  #                                                                                                         #
  ###########################################################################################################*/

/**************************************************************************************************
 *    Function      : voGSM_Parse_Errorcode
 *    Description   : This will parse a buffer for a uint16 and put it to the CME or CSM Errorcode
 *    Input         : GSM_ERRORCODE_TYPE_t, uint8_t*, uint8_t, uint8_t
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void  GSM_Parse_Errorcode(GSM_ERRORCODE_TYPE_t Type,uint8_t* Buffer, uint8_t idx_start, uint8_t len ){
    switch(Type){
        case GSM_CME_ERROR_CODE:{
            LastErrorCode.CME_Code = ParseInt(Buffer,idx_start,idx_start+len);
        } break;
        
        case GSM_CMS_ERROR_CODE:{
            LastErrorCode.CME_Code = ParseInt(Buffer,idx_start,idx_start+len);
        }
        
        default:{
            
        } break;
    }
}

/**************************************************************************************************
 *    Function      : voGSM_LocBufferClear
 *    Description   : This will clear the local buffer 
 *    Input         : GSM_ERRORCODE_TYPE_t, uint8_t*, uint8_t, uint8_t
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void GSM_LocBufferClear( void ){
    memset( (void*)(&ParserBuffer.LocalBuffer[0]), 0 , sizeof(ParserBuffer.LocalBuffer) );
    ParserBuffer.BufferIndex=0;
}

/**************************************************************************************************
 *    Function      : boGSM_LocBufferAdd
 *    Description   : This will add an element to the buffer if there is space left
 *    Input         : uint8_t
 *    Output        : bool
 *....Remarks       : returns false if buffer is full
 **************************************************************************************************/
bool GSM_LocBufferAdd( uint8_t element){
    bool Inserted=false;
    if( ParserBuffer.BufferIndex < (sizeof(ParserBuffer.LocalBuffer) ) ){
        ParserBuffer.LocalBuffer[ParserBuffer.BufferIndex]=element;
        ParserBuffer.BufferIndex++;
        Inserted = true;
        } else {
        Inserted = false;
    }
    return Inserted;
}

/**************************************************************************************************
 *    Function      : voGSM_LocBufferInserAtPosition
 *    Description   : This will add an element at a given index to the buffer and overwrites
 *    Input         : uint8_t, uint8_t
 *    Output        : bool
 *....Remarks       : returns false if the position is out of range
 **************************************************************************************************/
bool GSM_LocBufferInserAtPosition(uint8_t element, uint8_t idx){
    bool IsOutOfRange=true;
    if( idx >= sizeof(ParserBuffer.LocalBuffer) ){
        IsOutOfRange=true;
        } else {
        ParserBuffer.LocalBuffer[idx]=element;
    }
    return IsOutOfRange;
}


/**************************************************************************************************
 *    Function      : GSM_LocBufferReadAtPosition
 *    Description   : This will read the buffer at a given position
 *    Input         : uint8_t
 *    Output        : uint8_t
 *....Remarks       : returns 0 if the position is out of range
 **************************************************************************************************/
uint8_t GSM_LocBufferReadAtPosition( uint8_t idx){
    uint8_t ReturnVlaue=0;
    if( idx >= sizeof(ParserBuffer.LocalBuffer) ){
        ReturnVlaue=0;
        } else {
        ReturnVlaue = ParserBuffer.LocalBuffer[idx];
    }
    return ReturnVlaue;
}

/**************************************************************************************************
 *    Function      : FindNextIdx
 *    Description   : This will read the buffer and return the index of the given element if found
 *    Input         : uint8_t*, uint8_t, uint8_t , uint8_t
 *    Output        : uint8_t
 *....Remarks       : returns 0 if the element is not found
 **************************************************************************************************/
uint8_t FindNextIdx(uint8_t* Buffer, uint8_t BufferLen, uint8_t idx_start, uint8_t element){
    uint8_t NextIdx=0;
    
    for(uint8_t i=idx_start;i<BufferLen;i++){
        
        if( element == Buffer[i]  ){
            NextIdx = i;
            break;
        }
        
    }
    return NextIdx;
    
}

/**************************************************************************************************
 *    Function      : ParseInt
 *    Description   : This will read an uint16 from the buffer from start to end
 *    Input         : uint8_t*, uint8_t , uint8_t
 *    Output        : uint16_t
 *....Remarks       : returns 0 if we hint an error
 **************************************************************************************************/
uint16_t ParseInt(uint8_t* Buffer, uint8_t idx_start, uint8_t idx_end ){
    uint16_t ParsedElement=0;
    
    for(uint8_t i=idx_start;i<idx_end;i++){
        
        if( ( Buffer[i]>='0' ) && ( Buffer[i]<='9') ){
            ParsedElement=ParsedElement*10;
            ParsedElement=ParsedElement + Buffer[i]-'0';
            } else {
            /* Report error */
            break;
            
        }
    }
    
    return ParsedElement;

}


/**************************************************************************************************
 *    Function      : boGSM_LocBufferFindDelimiter
 *    Description   : this will try to find an \r\n in the buffer
 *    Input         : none
 *    Output        : bool
 *....Remarks       : returns true if a delimiter is found
 **************************************************************************************************/
bool GSM_LocBufferFindDelimiter( void ){
    return GSM_LocBufferFindDelimiterAfterIndex( 0 );
}

/**************************************************************************************************
 *    Function      : boGSM_LocBufferFindDelimiterAfterIndex
 *    Description   : this will try to find an \r\n in the buffer starting from a given index
 *    Input         : uint8_t
 *    Output        : bool
 *....Remarks       : returns true if a delimiter is found
 **************************************************************************************************/
bool GSM_LocBufferFindDelimiterAfterIndex( uint8_t start_idx)
{
    /* This will search for a Delimiter inside the buffer */
    bool Found=false;
    if(start_idx> (sizeof(ParserBuffer.LocalBuffer)-2) ){
        return false;
    }
    /* We do the following: we run along the buffer and keep an eye out for \r */
    for(uint8_t idx=start_idx;idx<sizeof(ParserBuffer.LocalBuffer)-2;idx++){
        if('\r' == ParserBuffer.LocalBuffer[idx] ){
            /* We check if the next element is \n */
            if('\n' == ParserBuffer.LocalBuffer[idx+1] ){
                Found=true;
                break;
            }
        }
    }
    return Found;
}

/*###########################################################################################################
  #  Here will be the functions used to check certain conditions afer a command is send                     #
  #                                                                                                         #
  ###########################################################################################################*/


/**************************************************************************************************
 *    Function      : CheckIfSIMisReady
 *    Description   : this will check if the modem has reported that the SIM is ready
 *    Input         : none
 *    Output        : uint8_t 
 *....Remarks       : returns 255 if the SIM is not ready, otherwise 0
 **************************************************************************************************/
uint8_t CheckIfSIMisReady( void ){
    if(ModemStatus.InitStatusModem != 3){
        return 255;
    } else {
        return 0;
    }
}

/**************************************************************************************************
 *    Function      : CheckIfNetLoginIsReady
 *    Description   : this will check if the modem has reported that the netlogin is okay
 *    Input         : none
 *    Output        : uint8_t 
 *....Remarks       : returns 255 if the SIM is not ready, otherwise 0
 **************************************************************************************************/
uint8_t CheckIfNetLoginIsReady(){
    if(ModemStatus.GSMNetStatus != 0){
        return 255;
    } else {
        return 0;
    }
}




/**************************************************************************************************
 *    Function      : enGSM_GetLastCommandResult
 *    Description   : This will get the result of the last executed command
 *    Input         : none
 *    Output        : GSM_CMD_LastResult_t
 *....Remarks       : none
 **************************************************************************************************/
GSMCMDLastResult_t GSM_GetLastCommandResult ( void ){
    return LastCmdResult;
}

/**************************************************************************************************
 *    Function      : s16GSM_GetLastCME_Error
 *    Description   : This will get the last code of a recognised CME error
 *    Input         : none
 *    Output        : int16_t
 *....Remarks       : if code is 65535 no error so far has been captured
 **************************************************************************************************/
uint16_t GSM_GetLastCME_Error( void ){
    return LastErrorCode.CME_Code;
}

/**************************************************************************************************
 *    Function      : GSM_GetLastCMS_Error
 *    Description   : This will get the last code of a recognised CMS error
 *    Input         : none
 *    Output        : int16_t
 *....Remarks       : if code is 65535 no error so far has been captured
 **************************************************************************************************/
uint16_t GSM_GetLastCMS_Error( void ) {
    return LastErrorCode.CMS_Code;
}

/**************************************************************************************************
 *    Function      : GSM_GetConfigChanged
 *    Description   : This will get if the Config has Changed
 *    Input         : none
 *    Output        : bool
 *....Remarks       : This will be one shot and reset after a read
 **************************************************************************************************/
bool GSM_GetConfigChanged( void ){
    return GSM_ConfigParsedOkay();
}

/**************************************************************************************************
 *    Function      : GSM_GetStatusRequested
 *    Description   : This will get if the Status is requested
 *    Input         : none
 *    Output        : bool
 *....Remarks       : This will be one shot and reset after a read
 **************************************************************************************************/
bool GSM_GetStatusRequested( void ){
    return GSM_RequestParsedOkay();
}

/**************************************************************************************************
 *    Function      : GSM_GetNetworkStatus
 *    Description   : This will get the Networkstatus
 *    Input         : none
 *    Output        : uint8
 *....Remarks       : none
 **************************************************************************************************/
uint8_t GSM_GetNetworkStatus( void ){
    return ModemStatus.GSMNetStatus;
}