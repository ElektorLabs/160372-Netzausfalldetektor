/*
 * int0.c
 *
 * Created: 04.04.2018 09:49:53
 *  Author: mathiasc
 */ 

#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "..\TIMER\timer.h"
#include "mains_mon.h"

/* Define for the MAINS timout in MS */
#define MAINSTIMOUTMS ( 250 )



#define HZ2MS( x )       ( 1000 / x ) 
#define RECOVERYLIMIT ( 120 )




/* This are internal flags */
volatile MAINS_MON_Status_t MAINS_MON_Status;

/* Needs to be volatile ( in SRAM ) as we need to access it from different ISR and functions */
volatile uint8_t RecoveryCounter=0;
volatile uint16_t LastTimestamp=0;
volatile uint16_t CurrentTimestamp=0;



/**************************************************************************************************
 *    Function      : MAINS_MON_Timout
 *    Description   : To be called if a Timout is detected
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
static void  MAINS_MON_Timout( void );





/**************************************************************************************************
 *    Function      : MAINS_MON_init
 *    Description   : Init of the mains monitor
 *    Input         : bool 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void MAINS_MON_init(bool InitwithPowerloss){
    /* We setup INT0 to be responding to falling edges */
    EICRA = ( ( 1 << ISC11) | ( 0 << ISC10) );
    EIMSK = ( 1 << INT1);
    
    RecoveryCounter=0;
    
    if(true == InitwithPowerloss ){
        TIMER_InitWatchDogTimer(MAINS_MON_Timout, MAINSTIMOUTMS );
        MAINS_MON_Timout();
    } else {
        MAINS_MON_Status.Status=MAINS_OK;
        TIMER_StartWatchDogTimer(MAINS_MON_Timout, MAINSTIMOUTMS );
    }    
    CurrentTimestamp = TIMER_GetTicks();
    LastTimestamp = TIMER_GetTicks();
    
}

/**************************************************************************************************
 *    Function      : MAINS_MON_GetStatus
 *    Description   : Gets the current mains status
 *    Input         : none 
 *    Output        : struct 
 *....Remarks       : none
 **************************************************************************************************/
MAINS_MON_Status_t MAINS_MON_GetStatus( ){
    MAINS_MON_Status_t ResultStatus;
    /* We can delete the fault if we are in recovering mode */
    cli();
    ResultStatus=MAINS_MON_Status;
    sei();
    if( MAINS_RECOVERING == MAINS_MON_Status.Status )
    {
        ResultStatus.Status = MAINS_FAIL;
    }
    return ResultStatus;
}

/**************************************************************************************************
 *    Function      : voMAINS_MON_Timout
 *    Description   : To be called if a Timout is detected
 *    Input         : none 
 *    Output        : none
 *....Remarks       : Needs to be called from an ISR 
 **************************************************************************************************/
static void  MAINS_MON_Timout( void ){
    /* Timeout Interrupt for the mains    */
    /* Set the MAINS Fail Flag here        */
    MAINS_MON_Status.Status = MAINS_FAIL;
    /* Reset the Frequency counter */
    RecoveryCounter=0;
    MAINS_MON_Status.FreqDeziHz =0;
    /* Fast access to IO */
    PORTD |= (1<<PD5);
    
}


/**************************************************************************************************
 *    Function      : ISR(INT0_vect)
 *    Description   : INT0 Pin INTERRUPT, sensitive on falling EDGE
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
ISR(INT1_vect){
    /* Okay we reset the Timer1 here */
   uint16_t SignalDelta=0;
   /* This toggels the LED */
   PIND = ( 1<< PD5) ;
   TIMER_ResetWatchDogTimer();
    CurrentTimestamp = TIMER_GetTicks();
    if( CurrentTimestamp < LastTimestamp){
        SignalDelta = UINT16_MAX - LastTimestamp + CurrentTimestamp;
    } else {
        SignalDelta = CurrentTimestamp - LastTimestamp;
    }        
     MAINS_MON_Status.FreqDeziHz =  10000/SignalDelta;
    if( MAINS_FAIL == MAINS_MON_Status.Status  ){
        /* We need to set the recovered bit */
        MAINS_MON_Status.Status = MAINS_RECOVERING;    
    }
    
    if(  MAINS_RECOVERING == MAINS_MON_Status.Status ){
         if( RecoveryCounter < RECOVERYLIMIT ){
           
                if(RecoveryCounter<UINT8_MAX){
                    RecoveryCounter++;
                }
        } else {
                MAINS_MON_Status.Status = MAINS_OK;
        }
    } 
    
    LastTimestamp = CurrentTimestamp;
}