/* 
    USART code written by Mathias Clauﬂen, B.Sc.
    This code is public domain and free as free beer
    This code comes without any warenty kill cute kittens 
    and may bring the end of days, use with care 
    
    Code created 2013
*/

/* Included for the AVR */
#include <stdlib.h>         
#include <inttypes.h>          
#include <avr/io.h>         
#include <avr/interrupt.h> 

/* Own include shall be the last one */
#include "./usart.h"

/* Defines for different AVR Systems */

#ifdef USART_UDRE_vect
#define UDRE_vect USART_UDRE_vect
#endif

#ifdef USART0_UDRE_vect
#define UDRE_vect USART0_UDRE_vect
#endif

#ifdef USART_RXC_vect
    #define RXC_vect USART_RXC_vect
#else
    #ifdef USART_RX_vect
        #define RXC_vect USART_RX_vect
    #else
        #define RXC_vect USART0_RX_vect
    #endif
#endif

/* local variabels */
static void(*fptr)(uint8_t ch);

/*
The following section is for Buffered Transmisst to offload the AVR
Only used if BUFFERED_TX > 0
*/

#if BUFFERED_TX > 0
#include "./fifo.h"
/* This is the local fifo buffer */
uint8_t buffer0[BUF_SIZE0];
/* This is our fifo struct */
fifo_t tx0_fifo;

/**************************************************************************************************
 *    Function      : usart_putc
 *    Description   : Sends one character with the usart ( here buffered ) 
 *    Input         : uint8_t 
 *    Output        : none
 *....Remarks       : Blocks till character has put to fifo
 **************************************************************************************************/
void usart_putc(unsigned char c) {

    #ifdef UCSRB //If we have an older peripheral in the AVR
        if((UCSRB&(1<<(UDRIE)))==0)
        {
            UCSRB |= (1<<(UDRIE));
        }
    
        while(fifo_put(&tx0_fifo,c)==0)
        {
            UCSRB |= (1<<(UDRIE));
        }
    #endif
    
    
    #ifdef UCSR0B //If we have an newer peripheral in the AVR
        if((UCSR0B&(1<<(UDRIE0)))==0)
        {
            UCSR0B |= (1<<(UDRIE0));
        }
    
        while(fifo_put(&tx0_fifo,c)==0)
        {
            UCSR0B |= (1<<(UDRIE0));
        }        
    #endif
}

/**************************************************************************************************
 *    Function      : ISR ( UDRE_vect )
 *    Description   : USART Dataregister Empty INTERRUPT
 *    Input         : none
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
ISR(UDRE_vect)
{
    /* As we are transmitting in a buffered mode, we now set the next byte to the TX register */
    int from_buffer=fifo_get_nowait(&tx0_fifo);
    
    if(from_buffer>-1)
    {
        #ifdef UDR
            UDR=(unsigned char)(from_buffer);
        #else
            UDR0=(unsigned char)(from_buffer);
        #endif
    }
    else
    {
        /* We have no new data in the fifo and disable the interrupt */
        #ifdef UCSRB
            UCSRB &= ~(1<<(UDRIE));
        #else
            UCSR0B &= ~(1<<(UDRIE0));
        #endif
    }
}
#else

/**************************************************************************************************
 *    Function      : usart_putc
 *    Description   : Sends one character with the usart
 *    Input         : uint8_t 
 *    Output        : none
 *....Remarks       : Blocks till character has send
 **************************************************************************************************/
void usart_putc(unsigned char c) {
   // wait until UDR ready
      #ifdef UCSRA
      while(!(UCSRA & (1 << UDRE)));
        UDR = c;    // send character
   #endif
   
   #ifdef UCSR0A
    while(!(UCSR0A & (1 << UDRE0)));
        UDR0 = c;    // send character
   #endif
}

#endif

/**************************************************************************************************
 *    Function      : Set_RX_Hook
 *    Description   : Sets the Callback for a received character
 *    Input         : void* 
 *    Output        : none
 *....Remarks       : If a valid pointer is given the RX Interrupt is activated 
 **************************************************************************************************/
void Set_RX_Hook(void* funkptr)
{
fptr=funkptr;

#ifdef UBRRL
UCSRB|=(1<<RXCIE);
#elif defined UBRR0L
UCSR0B|=(1<<RXCIE0);
#endif

}


/**************************************************************************************************
 *    Function      : Delete_RX_Hook
 *    Description   : Deletes the RX Callback
 *    Input         : none
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void Delete_RX_Hook(void)
{
    #ifdef UBRRL
    UCSRB&=~(1<<RXCIE);
    #elif defined UBRR0L
    UCSR0B&=~(1<<RXCIE0);
    #endif
fptr=0;
}

/**************************************************************************************************
 *    Function      : ISR(RXC_vect)
 *    Description   : Receive INTERRUPT
 *    Input         : none
 *    Output        : none
 *....Remarks       : If set, it will execute the callback
 **************************************************************************************************/
ISR(RXC_vect)
{
    uint8_t temp __attribute__ ((unused));
    if(fptr!=0)
    {
            #ifdef UDR
                temp=UDR;
            #else
                temp=UDR0;
            #endif
            (*fptr)(temp);    
    }
    else
    {
        #ifdef UDR
         temp=UDR;
        #else
         temp=UDR0;
        #endif
    }
}


#ifdef UDR
/**************************************************************************************************
 *    Function      : usart_init
 *    Description   : usart initalisation
 *    Input         : usartparam_t* 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void usart_init(usartparam_t* Param) 
{
    
#ifdef REMAP
    /* If we have a Rempafunction this code will be incuded */
    if( false==Param->UseAlternatePins ){
        REMAP =REMAP & (~( 1 << U0MAP)); /* Clear the Remappin */
    } else {
        REMAP =REMAP | ( 1 << U0MAP); /* Set the Remappin */
    }
#endif

/* Only use the following code if we have a buffered tx */
#if BUFFERED_TX > 0
fifo_init (&tx0_fifo, buffer0, BUF_SIZE0);
#endif


unsigned int br=Param->BaudrateRegister;

UBRRL=(unsigned char)br;
UBRRH=(unsigned char)(br>>8);
UCSRB =(1<<RXEN)|(1<<TXEN);

/* Setup according to ATMEL Datasheet */
UCSRC=(1<<URSEL)|(0<<UMSEL)|(0<<UCPOL); /* This sets the correct register access */


uint8_t UCSRC_Temp=(1<<URSEL); /* We use a temporary variabel to get arround some ugly effects of two register at the same address */

switch(Param->Paritytype)
{
    case None:
    {
        UCSRC_Temp&=~( (1<<UPM0)||(1<<UPM1) );        
    }
    break;
    
    case Even:
    {
        UCSRC_Temp|=(1<<UPM1);        
    }
    break;
    
    case Odd:
    {
        UCSRC_Temp|=(1<<UPM0)||(1<<UPM1);
    }
    break;
    
    default:
    {
        UCSRC_Temp&=~( (1<<UPM0)||(1<<UPM1) );
    }
    break;
}

switch(Param->NoOfBits)
{
    case Nine:
    {
        UCSRC_Temp |=( (1<<UCSZ2) | (1<<UCSZ1) | (1<<UCSZ0));
    }
    
    case Eight:
    {
        UCSRC_Temp |=( (0<<UCSZ2) | (1<<UCSZ1) | (1<<UCSZ0));
    }
    break;
    
    case Seven:
    {
        UCSRC_Temp |=( (0<<UCSZ2) | (1<<UCSZ1) | (0<<UCSZ0));
    }
    break;
    
    case Six:
    {
        UCSRC_Temp |=( 0<<UCSZ2) | (0<<UCSZ1) | (1<<UCSZ0);    
    }
    break;
    
    case Five:
    {
        UCSRC_Temp |=( (0<<UCSZ2) | (0<<UCSZ1) | (0<<UCSZ0));    
    }
    break;
    
    default:
    {
        UCSRC_Temp |=( (0<<UCSZ2) | (1<<UCSZ1) | (1<<UCSZ0));
    }
    break;
}

UCSRC=UCSRC_Temp; /* Write the value to the correct register */

if(Param->Doublespeed>0)
{
    UCSRA |= (1<<U2X); 
}
else
{
    UCSRA &= ~(1<<U2X); /* Clear the doublespeed bit */
}

#if BUFFERED_TX > 0
UCSRB |= (1<<(UDRIE)); /* Enable the Dataregister Empty Interrupt */
#endif

}
#endif

#ifdef UDR0
/**************************************************************************************************
 *    Function      : usart_init
 *    Description   : usart initalisation
 *    Input         : usartparam_t* 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void usart_init(usartparam_t* Param)
{
    #ifdef REMAP
        if( false==Param->UseAlternatePins ){
            REMAP =REMAP & (~( 1 << U0MAP)); /* Clear the Remappin */
        } else {
            REMAP =REMAP | ( 1 << U0MAP); /* Set the Remappin */
        }
    #endif

    #if BUFFERED_TX > 0
    fifo_init (&tx0_fifo, buffer0, BUF_SIZE0);
    #endif


    unsigned int br=Param->BaudrateRegister;

    UBRR0L=(unsigned char)br;
    UBRR0H=(unsigned char)(br>>8);
    UCSR0B =(1<<RXEN0)|(1<<TXEN0);

    uint8_t UCSRC_Temp=0; /* We use a temporary variabel to get arround some ugly effects of two register at the same address */
    
    switch(Param->Paritytype)
    {
        case None:
        {
            UCSRC_Temp&=~( (1<<UPM00)||(1<<UPM01) );
        }
        break;
        
        case Even:
        {
            UCSRC_Temp|=(1<<UPM01);
        }
        break;
        
        case Odd:
        {
            UCSRC_Temp|=(1<<UPM00)||(1<<UPM01);
        }
        break;
        
        default:
        {
            UCSRC_Temp&=~( (1<<UPM00)||(1<<UPM01) );
        }
        break;
    }

    switch(Param->NoOfBits)
    {
        case Nine:
        {
            UCSRC_Temp |=( (1<<UCSZ02) | (1<<UCSZ01) | (1<<UCSZ00));
        }
        
        case Eight:
        {
            UCSRC_Temp |=( (0<<UCSZ02) | (1<<UCSZ01) | (1<<UCSZ00));
        }
        break;
        
        case Seven:
        {
            UCSRC_Temp |=( (0<<UCSZ02) | (1<<UCSZ01) | (0<<UCSZ00));
        }
        break;
        
        case Six:
        {
            UCSRC_Temp |=( 0<<UCSZ02) | (0<<UCSZ01) | (1<<UCSZ00);
        }
        break;
        
        case Five:
        {
            UCSRC_Temp |=( (0<<UCSZ02) | (0<<UCSZ01) | (0<<UCSZ00));
        }
        break;
        
        default:
        {
            UCSRC_Temp |=( (0<<UCSZ02) | (1<<UCSZ01) | (1<<UCSZ00));
        }
        break;
    }

    UCSR0C=UCSRC_Temp;

    if(Param->Doublespeed>0)
    {
        UCSR0A |= (1<<U2X0);
    }
    else
    {
        UCSR0A &= ~(1<<U2X0); /* Clear the doublespeed bit */
    }

    #if BUFFERED_TX > 0
    UCSR0B |= (1<<(UDRIE0)); /* If we use bufferd Transmitt we enable the Dataregister Empty Interrupt */
    #endif

}
#endif