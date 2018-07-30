/* 
    This code is borrowed form  http://rn-wissen.de/wiki/index.php?title=FIFO_mit_avr-gcc
*/

#ifndef _FIFO_H_
#define _FIFO_H_

#include <avr/io.h>
#include <avr/interrupt.h>

typedef struct
{
    uint8_t volatile count;       // Elements in Buffer
    uint8_t size;                 // Buffersize
    uint8_t *pread;               // Readpointer
    uint8_t *pwrite;              // Writepointer
    uint8_t read2end, write2end;  
} fifo_t;


/**************************************************************************************************
 *    Function      : fifo_init
 *    Description   : initializes a fifo element
 *    Input         : fifo_t*, uint8_t* , const uint8_t 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void fifo_init (fifo_t*, uint8_t* buf, const uint8_t size);

/**************************************************************************************************
 *    Function      : fifo_put
 *    Description   : adds an element to the fifo
 *    Input         : fifo_t*, const uint8_t* 
 *    Output        : uint8_t 
 *....Remarks       : If return code equal 1 the operation was successfully 
 **************************************************************************************************/
uint8_t fifo_put (fifo_t*, const uint8_t data);

/**************************************************************************************************
 *    Function      : fifo_get_wait
 *    Description   : Gets an element form the FiFo, blocks till one is available 
 *    Input         : fifo_t*
 *    Output        : uint8_t 
 *....Remarks       : Blocks till data is in FiFo
 **************************************************************************************************/
uint8_t fifo_get_wait (fifo_t*);

/**************************************************************************************************
 *    Function      : fifo_get_nowait
 *    Description   : Gets an element form the FiFo
 *    Input         : fifo_t*
 *    Output        : int16_t 
 *....Remarks       : Returns values less than zero if ther is no Data 
 **************************************************************************************************/
int fifo_get_nowait (fifo_t*);

/**************************************************************************************************
 *    Function      : fifo_get_item_count
 *    Description   : Gets the amount of elements in FiFo
 *    Input         : fifo_t*
 *    Output        : int16_t 
 *....Remarks       : none
 **************************************************************************************************/
uint8_t fifo_get_item_count(fifo_t *f);



#endif /* _FIFO_H_ */
