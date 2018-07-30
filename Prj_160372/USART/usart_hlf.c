/* 
    USART code written by Mathias Claußen, B.Sc.
    This code is public domain and free as free beer
    This code comes without any warenty kill cute kittens 
    and may bring the end of days, use with care 
    
    Code created 2013
*/

#include "./usart_hlf.h"


/**************************************************************************************************
 *    Function      : usart_puts
 *    Description   : Sends a NULL_terminated String with the usart
 *    Input         : int8_t* 
 *    Output        : none
 *....Remarks       : String needs to be \0 Terminated
 **************************************************************************************************/
void usart_puts (char *s) {
    //  loop while *s != NULL
    while (*s) {
        usart_putc(*s);
        s++;
    }
}

/**************************************************************************************************
 *    Function      : usart_pute
 *    Description   : Sends a NULL_terminated String from EEPROM with the usart
 *    Input         : const int8_t* 
 *    Output        : none
 *....Remarks       : String needs to be \0 Terminated in EEPROM
 **************************************************************************************************/
void usart_pute (const char *s)
{
    char zeichen= (char) eeprom_read_byte((const uint8_t*)s);
    s++;
    while(zeichen!='\0')
    {
        usart_putc(zeichen);
        zeichen=(char) eeprom_read_byte((const uint8_t*)s);
        s++;
    }
}

/**************************************************************************************************
 *    Function      : usart_putp
 *    Description   : Sends a NULL_terminated String from FLASH with the usart
 *    Input         : const int8_t* 
 *    Output        : none
 *....Remarks       : String needs to be \0 Terminated in FLASH
 **************************************************************************************************/
void usart_putp ( const char *s)
{
    while (1)
    {
        unsigned char c = pgm_read_byte (s);
        s++;
        if ('\0' == c)
        break;
        usart_putc(c);
    }
}

/**************************************************************************************************
 *    Function      : usart_puti_help
 *    Description   : Helperfunction to generate the integer Elements
 *    Input         : unsigned char*, unsigned char*, unsigned char*, unsigned char*, unsigned char*
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void usart_puti_help(unsigned char* zehntausend,unsigned char* tausend,unsigned char* hundert,unsigned char* zehn,unsigned int* c)
{

    while(*c>9999) //max 6 runs
    {
        *zehntausend=*zehntausend+1;
        *c-=10000L;
    }

    while(*c>999) //max 9 runs
    {
        *tausend=*tausend+1;
        *c-=1000L;
    }

    while(*c>99) //max 9 runs
    {
        *hundert=*hundert+1;
        *c-=100;
    }

    while(*c>9) //max 9 runs
    {
        *zehn=*zehn+1;
        *c-=10;
    }

}

/**************************************************************************************************
 *    Function      : usart_puti
 *    Description   : Sends an unsigned integer as ASCII-String 
 *    Input         : uint16_t
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void usart_puti(unsigned int c)
{
    unsigned char zehntausend=0;
    unsigned char tausend=0;
    unsigned char hundert=0;
    unsigned char zehn=0;
    usart_puti_help(&zehntausend,&tausend,&hundert,&zehn,&c);

    if(zehntausend!=0)
    usart_putc(zehntausend+48);


    if((tausend!=0)||(zehntausend!=0))
    usart_putc(tausend+48);


    if((hundert!=0)||(tausend!=0)||(zehntausend!=0))
    usart_putc(hundert+48);

    if((zehn!=0)||(hundert!=0)||(tausend!=0)||(zehntausend!=0))
    usart_putc(zehn+48);

    usart_putc(c+48);

}

/**************************************************************************************************
 *    Function      : usart_puth_help
 *    Description   : Helperfunction to generate the HEX Elements
 *    Input         : unsigned char*, unsigned char*, unsigned char*
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void usart_puth_help(unsigned char* low_nibble,unsigned char* high_nibble, unsigned char* c)
{
    *low_nibble=(*c & 0x0F); //mask the upper 4 bit
    *high_nibble=(*c & 0xF0);
    *high_nibble=(*high_nibble>>4);

    if(*low_nibble>9)
    {
        *low_nibble=*low_nibble+55;
    }
    else
    {
        *low_nibble=*low_nibble+48;
    }


    if(*high_nibble>9)
    {
        *high_nibble=*high_nibble+55;
    }
    else
    {
        *high_nibble=*high_nibble+48;
    }


}

/**************************************************************************************************
 *    Function      : usart_puth
 *    Description   : Sends an unsigned integer as HEX-String 
 *    Input         : uint16_t
 *    Output        : none
 *....Remarks       : Sets 0x in front of the String
 **************************************************************************************************/
void usart_puth(unsigned char c)
{
    unsigned char low_nibble=0;
    unsigned char high_nibble=0;
    usart_puth_help(&low_nibble,&high_nibble,&c);
    usart_putc('0');
    usart_putc('x');
    /* divide the byte in two nibbel */
    usart_putc(high_nibble);
    usart_putc(low_nibble);
}

/**************************************************************************************************
 *    Function      : usart_put_XON
 *    Description   : Sends XON Character
 *    Input         : none
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void usart_put_XON( void ){
    usart_putc(0x11);
}

/**************************************************************************************************
 *    Function      : usart_put_XOFF
 *    Description   : Sends XOFF Character
 *    Input         : none
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void usart_put_XOFF( void ){
    usart_putc(0x13);
}