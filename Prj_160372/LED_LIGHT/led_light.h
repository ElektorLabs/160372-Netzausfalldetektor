/*
 * led_light.h
 *
 * Created: 29.05.2018 11:37:09
 *  Author: mathiasc
 */ 


#ifndef LED_LIGHT_H_
#define LED_LIGHT_H_

typedef enum {
    LED_BLINK_10Hz,
    LED_BLINK_5Hz,
    LED_BLINK_2Hz,
    LED_BLINK_1Hz,
    LED_FADE_IN,  /* IF LED IS OFF THIS WILL DO A FADEOUT */
    LED_FASE_OUT, /* IF LED IS ON THIS WILL DO A FADEOUT */
    LED_FADE_IN_OUT, /* FADEIN / FADEOUT Effect */
    LED_BLINK_OFF,
    LED_STATIC_OFF,
    LED_STATIC_ON,
} LED_BLINK_Frq_t;

typedef enum {
    LED_RED,
    LED_GREEN,
} LED_t;



void LED_LIGHT_Init( void );
void LED_LIGHT_Task( void );

void LED_LIGHT_SetLed( LED_t Led , LED_BLINK_Frq_t Freq );
LED_BLINK_Frq_t LED_LIGHT_getLed( LED_t Led);


#endif /* LED_LIGHT_H_ */