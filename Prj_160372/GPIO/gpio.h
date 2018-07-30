/*
 * gpio.h
 *
 * Created: 03.04.2018 13:57:53
 *  Author: mathiasc
 */ 


#ifndef GPIO_H_
#define GPIO_H_

#include <stdint.h>

typedef enum {
PWRKEY=0,
EMOFF,
STATUS,
RTS,
CTS,
CLEAR_ALL,
LED3,
LED4,
LED5,
SUPPLYSTAT,
}GPIO_t;

typedef enum {
    GPIOLOW,
    GPIOHIGH,
    GPIOTRISTATE
}GPIO_STATE_t;

/**************************************************************************************************
 *    Function      : GPIO_init
 *    Description   : GPIO initalisation
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void GPIO_init( void );

/**************************************************************************************************
 *    Function      : GPIO_set
 *    Description   : Sets a GPIO to the given state
 *    Input         : GPIO_t , GPIO_STATE_t 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void GPIO_set(GPIO_t gpio, GPIO_STATE_t state);

/**************************************************************************************************
 *    Function      : GPIO_get
 *    Description   : Gets a GPIO state
 *    Input         : GPIO_t
 *    Output        : GPIO_STATE_t
 *....Remarks       : none
 **************************************************************************************************/
GPIO_STATE_t GPIO_get(GPIO_t gpio);

/**************************************************************************************************
 *    Function      : GPIO_toggle
 *    Description   : Toggles an GPIO State
 *    Input         : GPIO_t
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void GPIO_toggle(GPIO_t gpio);

#endif /* GPIO_H_ */