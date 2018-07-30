/* 
    USART code written by Mathias Clauﬂen, B.Sc.
    This code is public domain and free as free beer
    This code comes without any warenty kill cute kittens 
    and may bring the end of days, use with care 
    
    Code created 2013
*/

#include <avr/pgmspace.h>
#include <stdint.h>
#include <avr/eeprom.h>
#include <stdbool.h>
//USART Daten Senden
#ifndef USART_H_
#define USART_H_
/* BUFFERED_TX sets the use of an 16 Byte Fifo for TX Buffering to offloead the MCU a bit */
#define BUFFERED_TX 0

typedef enum {Five=5,Six,Seven,Eight,Nine} DataBits_t;
typedef enum {None,Even,Odd} Parity_t;
typedef enum {One,Two} StopBits_t;

typedef struct
{        
    int16_t BaudrateRegister;
    uint8_t Doublespeed;
    DataBits_t NoOfBits;
    Parity_t Paritytype;
    StopBits_t NoOfStopbits;
    #ifdef REMAP
        bool UseAlternatePins;    /* Used on newer devices */
    #endif
} usartparam_t;


/**************************************************************************************************
 *    Function      : usart_init
 *    Description   : usart initalisation
 *    Input         : usartparam_t* 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void usart_init(usartparam_t* Param);

/**************************************************************************************************
 *    Function      : usart_putc
 *    Description   : Sends one character with the usart
 *    Input         : uint8_t 
 *    Output        : none
 *....Remarks       : Blocks till character has send
 **************************************************************************************************/
void usart_putc(unsigned char c);

/**************************************************************************************************
 *    Function      : Set_RX_Hook
 *    Description   : Sets the Callback for a received character
 *    Input         : void* 
 *    Output        : none
 *....Remarks       : If a valid pointer is given the RX Interrupt is activated 
 **************************************************************************************************/
void Set_RX_Hook(void*);

/**************************************************************************************************
 *    Function      : Delete_RX_Hook
 *    Description   : Deletes the RX Callback
 *    Input         : none
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void Delete_RX_Hook(void);

#endif
