/*
 * timer.h
 *
 * Created: 04.04.2018 09:14:11
 *  Author: mathiasc
 */ 


#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>

typedef enum {
    GSMTIMOUTSTOPWATCH=0,
    DELAYSTOPWATCH,
    GSMINITTIMER,
    TIMERCNT
} StopwatchTimer_t;

typedef enum {
    STOPWATCHSTART=0,
    STOPWATCHBUSY,
    STOPWATCHSTOP,
    STOPWATCHIDXOUTOFRANGE,    
} STOPWATCHSTART_t;


/**************************************************************************************************
 *    Function      : voTimer_init
 *    Description   : Init for the Timer
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void Timer_init();


/**************************************************************************************************
 *    Function      : TIMER_GetTicks
 *    Description   : returns the ticks in ms till start
 *    Input         : void 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
uint16_t TIMER_GetTicks();


/**************************************************************************************************
 *    Function      : TIMER_StopwatchStart
 *    Description   : starts a Stopwatch with a given index
 *    Input         : StopwatchTimer_t 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
STOPWATCHSTART_t TIMER_StopwatchStart(StopwatchTimer_t idx);


/**************************************************************************************************
 *    Function      : TIMER_StopwatchReset
 *    Description   : Restarte / Resets a Stopwatch
 *    Input         : StopwatchTimer_t 
 *    Output        : none
 *....Remarks       : Blocks till character has send
 **************************************************************************************************/
STOPWATCHSTART_t TIMER_StopwatchReset(StopwatchTimer_t idx);


/**************************************************************************************************
 *    Function      : TIMER_StopwatchGetTime
 *    Description   : Gets the current runtime of a Stopwatch in seconds
 *    Input         : uint8_t 
 *    Output        : uint8_t
 *....Remarks       : Blocks till character has send
 **************************************************************************************************/
uint8_t TIMER_StopwatchGetTime(StopwatchTimer_t idx);


/**************************************************************************************************
 *    Function      : TIMER_StopwatchStop
 *    Description   : Stops a Stopwatchtimer and returns the timespan it has been active
 *    Input         : uint8_t 
 *    Output        : unit8_t
 *....Remarks       : If timespan if more than 255 seconds the counter won't count any further seconds
 **************************************************************************************************/
uint8_t TIMER_StopwatchStop(StopwatchTimer_t idx);


/**************************************************************************************************
 *    Function      : TIMER_StopwatchStatus
 *    Description   : Returns the Stopwatch state
 *    Input         : uint8_t 
 *    Output        : enum
 *....Remarks       : none
 **************************************************************************************************/
STOPWATCHSTART_t TIMER_StopwatchStatus(StopwatchTimer_t idx);


/**************************************************************************************************
 *    Function      : TIMER_InitDeadTimeTimer
 *    Description   : Inits a deadtime counter witch will at its end call the given callback
 *    Input         : void*, uint16_t 
 *    Output        : none
 *....Remarks       : Kind of Softwarewatchdog
 **************************************************************************************************/
void TIMER_InitWatchDogTimer( void* cb, int16_t DeadTime);

/**************************************************************************************************
 *    Function      : TIMER_StartDeatTimeTimer
 *    Description   : Starts a deadtime counter witch will at its end call the given callback
 *    Input         : void*, uint16_t 
 *    Output        : none
 *....Remarks       : Kind of Softwarewatchdog
 **************************************************************************************************/
void TIMER_StartWatchDogTimer( void* cb, int16_t DeadTime);


/**************************************************************************************************
 *    Function      : TIMER_ResetDeatTimeCounter
 *    Description   : Triggers the Deadtimeconter 
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void TIMER_ResetWatchDogTimer();


#endif /* TIMER_H_ */