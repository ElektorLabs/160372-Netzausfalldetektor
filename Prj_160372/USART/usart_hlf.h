/* 
    USART code written by Mathias Clauﬂen, B.Sc.
    This code is public domain and free as free beer
    This code comes without any warenty kill cute kittens 
    and may bring the end of days, use with care 
    
    Code created 2013
*/

// High Level Functions for µC's (this time AVRs)

#ifndef USART_HLF_H_
#define USART_HLF_H_

#include <avr/pgmspace.h>
#include <stdint.h>
#include <avr/eeprom.h>
#include "./usart.h"

/**************************************************************************************************
 *    Function      : usart_puts
 *    Description   : Sends a NULL_terminated String with the usart
 *    Input         : int8_t* 
 *    Output        : none
 *....Remarks       : String needs to be \0 Terminated
 **************************************************************************************************/
void usart_puts (char *s);

/**************************************************************************************************
 *    Function      : usart_putp
 *    Description   : Sends a NULL_terminated String from FLASH with the usart
 *    Input         : const int8_t* 
 *    Output        : none
 *....Remarks       : String needs to be \0 Terminated in FLASH
 **************************************************************************************************/
void usart_putp(const char *s);

/**************************************************************************************************
 *    Function      : usart_pute
 *    Description   : Sends a NULL_terminated String from EEPROM with the usart
 *    Input         : const int8_t* 
 *    Output        : none
 *....Remarks       : String needs to be \0 Terminated in EEPROM
 **************************************************************************************************/
void usart_pute (const char *s);

/**************************************************************************************************
 *    Function      : usart_puti
 *    Description   : Sends an unsigned integer as ASCII-String 
 *    Input         : uint16_t
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void usart_puti(unsigned int c);

/**************************************************************************************************
 *    Function      : usart_puth
 *    Description   : Sends an unsigned integer as HEX-String 
 *    Input         : uint16_t
 *    Output        : none
 *....Remarks       : Sets 0x in front of the String
 **************************************************************************************************/
void usart_puth(unsigned char c);

/**************************************************************************************************
 *    Function      : usart_put_XON
 *    Description   : Sends XON Character
 *    Input         : none
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void usart_put_XON( void );

/**************************************************************************************************
 *    Function      : usart_put_XOFF
 *    Description   : Sends XOFF Character
 *    Input         : none
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void usart_put_XOFF( void );


#endif /* USART_HLF_H_ */