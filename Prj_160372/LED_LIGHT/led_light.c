/*
 * led_light.c
 *
 * Created: 2016 - 2018
 *  Author: mathiasc / calm 
 */ 

#include "..\TIMER\timer.h"
#include "..\GPIO\gpio.h"
#include "led_light.h"
/* GPIO Mapping */
GPIO_t Mapping[]={LED3,LED5};
/* This mapps to RED, GREEN */

/* We got here currently 3LEDs */
typedef struct{
    
    LED_BLINK_Frq_t CurrenFreq;
    LED_BLINK_Frq_t PrevFreq;
    GPIO_STATE_t InitialState;  
    
} LED_STATUS_t;

volatile LED_STATUS_t Led_State[2];

volatile uint8_t _10Hz_Counter;
volatile uint8_t _5Hz_Counter;
volatile uint8_t _2Hz_Counter;
volatile uint8_t _1Hz_Counter;


void LED_LIGHT_Init( void ){
    
    uint8_t Elements = sizeof(Led_State)/sizeof(LED_STATUS_t);
    
    for(uint8_t i=0;i<Elements;i++){
        
        Led_State[i].CurrenFreq=LED_BLINK_OFF;
        Led_State[i].PrevFreq=LED_BLINK_OFF;
        Led_State[i].InitialState=GPIOHIGH;
    
    }
    
    _10Hz_Counter=0;
    _1Hz_Counter=0;
    _2Hz_Counter=0;
    _5Hz_Counter=0;
    
    /* Init done for now */
}

void LED_LIGHT_Task( void ){
    static uint16_t LastTimestamp=0;
    /* We need to Check what to do */
    uint8_t Elements = sizeof(Led_State)/sizeof(LED_STATUS_t);
    uint16_t CurrentTimestamp = TIMER_GetTicks();
    uint16_t TimeDelta=0;
    if( CurrentTimestamp < LastTimestamp){
        TimeDelta = UINT16_MAX - LastTimestamp + CurrentTimestamp;
    } else {
        TimeDelta = CurrentTimestamp - LastTimestamp;
    } 
    if(TimeDelta < 100 ){
        return;
    } else {
        LastTimestamp = CurrentTimestamp;
    }        
        
    /* Keep track of the Timing */
    _10Hz_Counter++;
    _5Hz_Counter++;
    _2Hz_Counter++;
    _1Hz_Counter++;
    
    if(_10Hz_Counter>0){
        _10Hz_Counter=0;
    }
    
    if(_5Hz_Counter>1){
        _5Hz_Counter=0;
    }
    
    if(_2Hz_Counter>4){
        _2Hz_Counter=0;
    }
    
    
    if(_1Hz_Counter>9){
        _1Hz_Counter=0;
    }
    
    for(uint8_t i=0;i<Elements;i++){
        
        if(Led_State[i].CurrenFreq != Led_State[i].PrevFreq){
            Led_State[i].PrevFreq = Led_State[i].CurrenFreq;
            if(Led_State[i].CurrenFreq!=LED_BLINK_OFF){
                Led_State[i].InitialState=GPIO_get(Mapping[i]);
            } else {
                GPIO_set(Mapping[i],Led_State[i].InitialState);
            }
        }
        
        switch(Led_State[i].CurrenFreq){
            
            case LED_BLINK_10Hz:{
                if(_10Hz_Counter==0){
                    GPIO_toggle(Mapping[i]);
                }
            
            }
                
            case LED_BLINK_5Hz:{
                if(_5Hz_Counter==0){
                    GPIO_toggle(Mapping[i]);
                }
            } break;

            case LED_BLINK_2Hz:{
                if(_2Hz_Counter==0){
                    GPIO_toggle(Mapping[i]);
                }
            } break;

            case LED_BLINK_1Hz:{
                if(_1Hz_Counter==0){
                    GPIO_toggle(Mapping[i]);
                }
            } break;                            
            
            case LED_STATIC_OFF:{
                GPIO_set(Mapping[i],GPIOHIGH);
            }break;
            
            case LED_STATIC_ON:{
                GPIO_set(Mapping[i],GPIOLOW);
            }break;
            
            default:{
                /* We do nothing here as feading is not implimentend */
                
            }

        }
        
    }
    
    
}

void LED_LIGHT_SetLed( LED_t Led , LED_BLINK_Frq_t Freq ){
    
    /* we need to MAP the enum to the LED Index here */
    
    switch(Led){
        
        case LED_RED:{
            /* This should be 0 */
            if(Led_State[0].CurrenFreq!=Freq){
                Led_State[0].CurrenFreq=Freq;
            }
            
        } break;
        
        
        case LED_GREEN:{
            /* And this should be 1 */
            if(Led_State[1].CurrenFreq!=Freq){
                Led_State[1].CurrenFreq=Freq;
            }
            
            
        }break;
        
        default:{
            
            /* Do nothing as we don't know the LED */
            
        }break;
        
    }
    
    
    
    
}
LED_BLINK_Frq_t LED_LIGHT_getLed( LED_t Led){
                
        /* we need to MAP the enum to the LED Index here */
        
        LED_BLINK_Frq_t Return=LED_BLINK_OFF;
        
        switch(Led){
            
            case LED_RED:{
                /* This should be 0 */
                Return = Led_State[0].CurrenFreq;
                
            } break;
            
            case LED_GREEN:{
                /* And this should be 1 */
                Return = Led_State[1].CurrenFreq;
                
                
            }break;
            
            default:{
                
                /* Do nothing as we don't know the LED */
                
            }break;
            
        }
    
    return Return;
}