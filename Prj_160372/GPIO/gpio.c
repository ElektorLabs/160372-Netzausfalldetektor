/*
 * gpio.c
 *
 * Created: 03.04.2018 13:58:06
 *  Author: mathiasc
 */ 
#include <avr/io.h>
#include "gpio.h"

/* Internal PIN definitions */
#define PWR_KEY_PORT PORTD
#define PWR_KEY_PIN PIND6
#define PWR_KEY_PREG PIND
#define PWR_KEY_DDR DDRD

#define EMOFF_KEY_PORT PORTD
#define EMOFF_KEY_PIN PIND7
#define EMOFF_KEY_PREG PIND
#define EMOFF_KEY_DDR DDRD

#define CLEARALL_PORT PORTB
#define CLEEARALL_PIN PINB0
#define CLEARALL_PREG PINB
#define CLEARALL_DDR DDRD

#define RTS_PORT PORTD
#define RTS_PIN PIND6
#define RTS_PREG PIND
#define RTS_DDR DDRD

#define CTS_PORT PORTC
#define CTS_PIN PINC3
#define CTS_PREG PINC
#define CTS_DDR DDRC

#define STATUS_PORT PORTD
#define STATUS_PIN PIND2
#define STATUS_PREG PIND
#define STATUS_DDR DDRD

#define LED3_PORT PORTB
#define LED3_PIN PINB2
#define LED3_PREG PINB
#define LED3_DDR DDRB

#define LED4_PORT PORTD
#define LED4_PIN PIND5
#define LED4_PREG PIND
#define LED4_DDR DDRD

#define LED5_PORT PORTB
#define LED5_PIN PINB1
#define LED5_PREG PINB
#define LED5_DDR DDRB

#define SUPPLYSTAT_PORT PORTC
#define SUPPLYSTAT_PIN PINC1
#define SUPPLYSTAT_PREG PINC
#define SUPPLYSTAT_DDR DDRC


/**************************************************************************************************
 *    Function      : GPIO_init
 *    Description   : GPIO initalisation
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void GPIO_init( void ){
    /* This will setup the GPIOs for the Plattform */
    /* At the start we have everything as input and need to set some pins as output */
    /* We need PB1, PB2, PD5 PD6, PD7 and PC2 as output                             */
    DDRB |= ( (1 << PB1) | ( 1 << PB2) );
    DDRC |= ( ( 1 << PC2 ) | ( 1 << PC3) );
    DDRD |= ( (1<<PD5) | ( 1 <<PD6) | ( 1 << PD7 ) );
    
    
    
    /* We need PC3, PD2 PC1 and PB0 as digital input                                */
    /* We need ADC7 and ADC0 as Analog input                                        */
    
    /* For PB0 we use the internal Pullup */
    PORTB |= ( 1<<PB0);
    PORTD |= ( 1 << PD3);
    /* Set the pins in a default state */
    
    PORTD |= ( ( 1 << PD6 ) | ( 1 << PD7 ) );
    LED3_PORT |= (1<<LED3_PIN);
    LED4_PORT |= (1<<LED4_PIN);
    LED5_PORT |= (1<<LED5_PIN);
    
    PORTC |= ( 1 << PC1);
    
}

/**************************************************************************************************
 *    Function      : GPIO_set
 *    Description   : Sets a GPIO to the given state
 *    Input         : GPIO_t , GPIO_STATE_t 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void GPIO_set(GPIO_t gpio, GPIO_STATE_t state){
  
        switch ( state ){
            
            case GPIOLOW:{
               
                switch( gpio ){
                  
                    case CLEAR_ALL:{
                        CLEARALL_PORT &= ~(1<<CLEEARALL_PIN);
                    } break;
                    
                    case EMOFF:{
                          EMOFF_KEY_PORT &= ~(1<<EMOFF_KEY_PIN);
                          EMOFF_KEY_DDR |= ( 1 << EMOFF_KEY_PIN);
                    } break;
                    
                    case PWRKEY:{
                         PWR_KEY_PORT &= ~(1<<PWR_KEY_PIN);
                    } break;
                    
                    case RTS:{
                        RTS_PORT &=~(1<<RTS_PIN);
                    } break;
                    
                    case LED3:{
                        LED3_PORT &=~(1<<LED3_PIN);
                    } break;
                    
                    case LED4:{
                        LED4_PORT &=~(1<<LED4_PIN);
                    } break;
                    
                    case LED5:{
                        LED5_PORT &=~(1<<LED5_PIN);
                    } break;
                    
                    case CTS:{
                        /* Not supported by SW */ 
                    } break;
                    
                    case STATUS:{
                        /* Not supported by SW */
                    } break;
                    
                    case SUPPLYSTAT:{
                        /* Not supported by SW */
                    } break;
                }
             } break;
             
            case GPIOHIGH:{
              switch( gpio ){
                   case CLEAR_ALL:{
                       CLEARALL_PORT |= ( 1<< CLEEARALL_PIN);
                   } break;
                   
                    case EMOFF:{
                            /* Not supported by SW */
                    } break;
                
                    case PWRKEY:{
                        PWR_KEY_PORT |= ( 1<< PWR_KEY_PIN);
                    } break;
                    
                     case RTS:{
                         RTS_PORT |= (1<<RTS_PIN);
                     } break;
                     
                     case LED3:{
                         LED3_PORT |= (1<<LED3_PIN);
                     } break;
                     
                     case LED4:{
                         LED4_PORT |= (1<<LED4_PIN);
                     } break;
                     
                     case LED5:{
                         LED5_PORT |= (1<<LED5_PIN);
                     } break;
                     
                    case CTS:{
                        /* Not supported by SW */
                    } break;
                        
                    case STATUS:{
                        /* Not supported by SW */
                    } break;
                    
                    case SUPPLYSTAT:{
                        /* Not supported by SW */
                    } break;
              }                
            } break;
            
            case GPIOTRISTATE:{
                 switch( gpio ){
                    case CLEAR_ALL:{
                       /* Not support by SW */
                    } break;
                
                    case EMOFF:{
                        EMOFF_KEY_DDR &= ~(1<< PWR_KEY_PIN);
                    } break;
                
                    case PWRKEY:{
                         /* Not support by SW */
                    } break;
                    
                    case RTS:{
                         /* Not supported by SW */
                    } break;
                    
                    case LED3:{
                         /* Not supported by SW */
                    } break;
                    
                    case LED4:{
                          /* Not supported by SW */
                    } break;
                    
                    case LED5:{
                          /* Not supported by SW */
                    } break;
                    
                    case CTS:{
                        /* Not supported by SW */
                    } break;
                    
                    case STATUS:{
                        /* Not supported by SW */
                    } break;
                    
                    case SUPPLYSTAT:{
                        /* Not supported by SW */
                    } break;
                 }                
            } break;
            
        default:{
                
        } break;
     }        
}



/**************************************************************************************************
 *    Function      : GPIO_get
 *    Description   : Gets a GPIO state
 *    Input         : GPIO_t
 *    Output        : GPIO_STATE_t
 *....Remarks       : none
 **************************************************************************************************/
GPIO_STATE_t GPIO_get(GPIO_t gpio){
    GPIO_STATE_t State=GPIOLOW;
    switch(gpio){
        case PWRKEY:{
            if( 0 != (PWR_KEY_PREG & ( 1 << PWR_KEY_PIN) ) ){
                State=GPIOHIGH;
            } else {
                State=GPIOLOW;
            }
        }break;
        
        case EMOFF:{
            if( 0 != (EMOFF_KEY_PREG & ( 1 << EMOFF_KEY_PIN) ) ){
                State=GPIOHIGH;
            } else {
                State=GPIOLOW;
            }
        }break;
        
        case CLEAR_ALL:{
            if( 0 != (CLEARALL_PREG & ( 1 << CLEEARALL_PIN) ) ){
                State=GPIOHIGH;
            } else {
                State=GPIOLOW;
            }
        }break;
        
        case RTS:{
            if( 0 != (RTS_PREG & ( 1 << RTS_PIN) ) ){
                State=GPIOHIGH;
                } else {
                State=GPIOLOW;
            }
        } break;
          
        case LED3:{
            if( 0 != (LED3_PREG & ( 1 << LED3_PIN) ) ){
                State=GPIOHIGH;
                } else {
                State=GPIOLOW;
            }
        } break;
          
        case LED4:{
           if( 0 != (LED4_PREG & ( 1 << LED4_PIN) ) ){
               State=GPIOHIGH;
               } else {
               State=GPIOLOW;
           }
        } break;
          
        case LED5:{
             if( 0 != (LED5_PREG & ( 1 << LED5_PIN) ) ){
                 State=GPIOHIGH;
                 } else {
                 State=GPIOLOW;
             }
        } break;
          
        case CTS:{
             if( 0 != (CTS_PREG & ( 1 << CTS_PIN) ) ){
                 State=GPIOHIGH;
                 } else {
                 State=GPIOLOW;
             }
        } break;
          
        case STATUS:{
             if( 0 != (STATUS_PREG & ( 1 << STATUS_PIN ) ) ){
                 State=GPIOHIGH;
                 } else {
                 State=GPIOLOW;
             }
        } break;
        
        case SUPPLYSTAT:{
            if( 0 != (SUPPLYSTAT_PREG & ( 1 <<SUPPLYSTAT_PIN ) ) ){
                State=GPIOHIGH;
                } else {
                State=GPIOLOW;
            }
        } break;
        
        default:{
            /* We shall never end here */
        }break;
    }
    
    return State;
}

/**************************************************************************************************
 *    Function      : GPIO_toggle
 *    Description   : Toggles an GPIO State
 *    Input         : GPIO_t
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void GPIO_toggle(GPIO_t gpio){

    GPIO_STATE_t state = GPIO_get(gpio);
    if(GPIOLOW == state){
        GPIO_set(gpio,GPIOHIGH);
    } else {
        GPIO_set(gpio,GPIOLOW);
    }

}