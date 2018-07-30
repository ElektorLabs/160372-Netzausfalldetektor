/*
 * timer.c
 *
 * Created: 04.04.2018 09:14:25
 *  Author: mathiasc
 */ 

#include <stddef.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "../LED_LIGHT/led_light.h"
#include "../ADC/adc.h"
#include "timer.h"



/*
  We use volatile declaratins here as 
  this shall prevent that the compiler optimizes values
  to the register witch is bad if update the during interrupts 
*/
volatile uint16_t TimerTicks=0;

/* This will be out Stopwatches */
volatile struct {
    uint8_t StopwatchSecActive[TIMERCNT];
    uint16_t StopwatchSecPrescaler[TIMERCNT];
    uint8_t StopwatchSecCnt[TIMERCNT];
} TIMER_Flags;

/* This is for the deadtime detection */
volatile struct {
    int16_t DeatTimeLocal;
    int16_t DeadtimeCount;
    void(*fptrdeadtimecb)(void);
} WatchDogTimer;   



/**************************************************************************************************
 *    Function      : voTimer_init
 *    Description   : Init for the Timer
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void Timer_init(){
    /* For the ims interrupt we use the 8Bit timer with the F_CPU prescaled by 64, resulting in 
     * 125.000Hz Clockrate. with desired 1000Hz Rate we will have to count till 124 from 0 onwards 
     */
    TCCR0B = ( 1 << CS01) | ( 1 << CS00); /*Prescale 1024 */
    OCR0A = 124; /* 125 steps , zero to 124 */
    TCCR0A = ( 1 << WGM01); /* We set the CTC mode of the timer  */
    TIMSK0 = ( 1 << OCIE0A) | ( 1 << OCIE0B ) ; /* Compate Match interrupt A active */
    memset((void*)&TIMER_Flags , 0 , sizeof(TIMER_Flags));
    memset((void*)&WatchDogTimer , 0 , sizeof(WatchDogTimer));
}

/**************************************************************************************************
 *    Function      : ISR(TIMER0_COMPA_vect)
 *    Description   : Interrupt for out 8Bit TIMER0
 *    Input         : none 
 *    Output        : none
 *....Remarks       : Used for Systemtiming
 **************************************************************************************************/
ISR(TIMER0_COMPA_vect){
    static uint16_t SecondPrescaler=0;
    TimerTicks++;
    SecondPrescaler++;
    /* We do a conversation every second */
    if(SecondPrescaler>=1000){
            ADC_StartConversation();
            SecondPrescaler=0;
    }        
    
    /* This handles the internal used timer */
    for(uint8_t x=0; x < TIMERCNT ; x++){
        /* If the timer is active we do the housekeeping */
        if(1==TIMER_Flags.StopwatchSecActive[x]){
            TIMER_Flags.StopwatchSecPrescaler[x]++;
            /* As we use one second resolution we increment every 1000 calls */
            if(TIMER_Flags.StopwatchSecPrescaler[x]>=1000){
                /* reset of the counter */
                TIMER_Flags.StopwatchSecPrescaler[x]=0;
                /* We add a second to the counter if we are below 255, else we would get an overflow */
                if(TIMER_Flags.StopwatchSecCnt[x]<UINT8_MAX){
                    TIMER_Flags.StopwatchSecCnt[x]++;
                }
            }
        }
    }
    
    /* This is to determine if the mains has no longer a valid frequency */
    if( 0 ==  WatchDogTimer.DeadtimeCount ){
     /* If the mains are gone, we use a callback to execute a defined function */
     if(WatchDogTimer.fptrdeadtimecb != NULL){
        /* This will execute what ever is registered to the functionpointer */
        WatchDogTimer.fptrdeadtimecb();  
     }   
    }        
    
    /* Else if the deadtimecounter is not zero we will decrement it by one */
    if( WatchDogTimer.DeadtimeCount>=0 ){
        WatchDogTimer.DeadtimeCount--;
        
    } 
    
    
    
}

/**************************************************************************************************
 *    Function      : TIMER_GetTicks
 *    Description   : returns the ticks in ms till start
 *    Input         : void 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
uint16_t TIMER_GetTicks(){
    uint16_t Result = 0;
    /* This needs to be atomic */
    cli();
    Result = TimerTicks; /* To prevent corruption of the variable */
    sei();
    return Result;
}


/**************************************************************************************************
 *    Function      : enTIMER_StopwatchStart
 *    Description   : starts a Stopwatch with a given index
 *    Input         : StopwatchTimer_t 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
STOPWATCHSTART_t TIMER_StopwatchStart( StopwatchTimer_t idx){
   
    STOPWATCHSTART_t Result=STOPWATCHBUSY;
    if(idx >= TIMERCNT){
    return STOPWATCHIDXOUTOFRANGE;
    }
    
    
    if(0==TIMER_Flags.StopwatchSecActive[idx]){
       
        while(TIMER_Flags.StopwatchSecCnt[idx]!=0){
            TIMER_Flags.StopwatchSecCnt[idx]=0;
        }            
        TIMER_Flags.StopwatchSecActive[idx]=1;
        
        Result=STOPWATCHSTART;
        } else {
        Result=STOPWATCHBUSY;
    }
    return Result;
}

/**************************************************************************************************
 *    Function      : TIMER_StopwatchGetTime
 *    Description   : Gets the current runtime of a Stopwatch in seconds
 *    Input         : uint8_t 
 *    Output        : uint8_t
 *....Remarks       : Blocks till character has send
 **************************************************************************************************/
uint8_t TIMER_StopwatchGetTime(StopwatchTimer_t idx){
    if(idx >= TIMERCNT){
        return 0;
    }
    return TIMER_Flags.StopwatchSecCnt[idx];
}

/**************************************************************************************************
 *    Function      : TIMER_StopwatchStop
 *    Description   : Stops a Stopwatchtimer and returns the timespan it has been active
 *    Input         : uint8_t 
 *    Output        : unit8_t
 *....Remarks       : If timespan if more than 255 seconds the counter won't count any further seconds
 **************************************************************************************************/
uint8_t TIMER_StopwatchStop(StopwatchTimer_t idx){
    if(idx >= TIMERCNT){
        return 0;
    } 
    TIMER_Flags.StopwatchSecActive[idx]=0;
    return TIMER_Flags.StopwatchSecCnt[idx];
}

/**************************************************************************************************
 *    Function      : enTIMER_StopwatchStatus
 *    Description   : Returns the Stopwatch state
 *    Input         : uint8_t 
 *    Output        : enum
 *....Remarks       : none
 **************************************************************************************************/
STOPWATCHSTART_t TIMER_StopwatchStatus( StopwatchTimer_t idx ) {
    if(idx >= TIMERCNT){
        return STOPWATCHIDXOUTOFRANGE;
    }
    if(0==TIMER_Flags.StopwatchSecActive[idx]){
        return STOPWATCHSTOP;
    } else {
        return STOPWATCHSTART;
    }
}

/**************************************************************************************************
 *    Function      : enTIMER_StopwatchReset
 *    Description   : Restarte / Resets a Stopwatch
 *    Input         : StopwatchTimer_t 
 *    Output        : none
 *....Remarks       : Blocks till character has send
 **************************************************************************************************/
STOPWATCHSTART_t TIMER_StopwatchReset( StopwatchTimer_t idx ){
   
    if(idx >= TIMERCNT){
        return STOPWATCHIDXOUTOFRANGE;
    }
    if(0==TIMER_Flags.StopwatchSecActive[idx]){
        TIMER_Flags.StopwatchSecCnt[idx]=0;
        
    } 
    return STOPWATCHSTART;
}

/**************************************************************************************************
 *    Function      : voTIMER_InitDeadTimeTimer
 *    Description   : Inits a deadtime counter witch will at its end call the given callback
 *    Input         : void*, uint16_t 
 *    Output        : none
 *....Remarks       : Kind of Softwarewatchdog
 **************************************************************************************************/
void TIMER_InitWatchDogTimer( void* cb, int16_t DeadTime){
    WatchDogTimer.fptrdeadtimecb = cb;
    cli();
    WatchDogTimer.DeatTimeLocal=DeadTime;
    WatchDogTimer.DeadtimeCount=(-1);
    sei();
}

/**************************************************************************************************
 *    Function      : voTIMER_StartDeatTimeTimer
 *    Description   : Starts a deadtime counter witch will at its end call the given callback
 *    Input         : void*, uint16_t 
 *    Output        : none
 *....Remarks       : Kind of Softwarewatchdog
 **************************************************************************************************/
void TIMER_StartWatchDogTimer( void* cb, int16_t DeadTime){
    cli();
    WatchDogTimer.fptrdeadtimecb = cb;
    WatchDogTimer.DeatTimeLocal=DeadTime;
    WatchDogTimer.DeadtimeCount=WatchDogTimer.DeatTimeLocal;
    sei();
}


/**************************************************************************************************
 *    Function      : voTIMER_ResetDeatTimeCounter
 *    Description   : Triggers the Deadtimeconter 
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void TIMER_ResetWatchDogTimer(){
    cli();
    WatchDogTimer.DeadtimeCount=WatchDogTimer.DeatTimeLocal;
   sei();
}