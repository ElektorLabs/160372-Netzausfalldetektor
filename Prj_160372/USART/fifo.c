/* 
    This code is borrowed form  http://rn-wissen.de/wiki/index.php?title=FIFO_mit_avr-gcc
*/

#include "fifo.h"

/**************************************************************************************************
 *    Function      : _inline_fifo_put
 *    Description   : inlinefunction to add a FiFo Element
 *    Input         : fifo_t*, 
 *    Output        : uint8_t
 *....Remarks       : none
 **************************************************************************************************/
static inline uint8_t
_inline_fifo_put (fifo_t *f, const uint8_t data)
{
    if (f->count >= f->size)
        return 0;
        
    uint8_t * pwrite = f->pwrite;
    
    *(pwrite++) = data;
    
    uint8_t write2end = f->write2end;
    
    if (--write2end == 0)
    {
        write2end = f->size;
        pwrite -= write2end;
    }
    
    f->write2end = write2end;
    f->pwrite = pwrite;

    uint8_t sreg = SREG;
    cli();
    f->count++;
    SREG = sreg;
    
    return 1;
}

/**************************************************************************************************
 *    Function      : _inline_fifo_get
 *    Description   : inlinefunction to get a FiFo Element
 *    Input         : fifo_t*, 
 *    Output        : uint8_t
 *....Remarks       : none
 **************************************************************************************************/
static inline uint8_t 
_inline_fifo_get (fifo_t *f)
{
    uint8_t *pread = f->pread;
    uint8_t data = *(pread++);
    uint8_t read2end = f->read2end;
    
    if (--read2end == 0)
    {
        read2end = f->size;
        pread -= read2end;
    }
    
    f->pread = pread;
    f->read2end = read2end;
    
    uint8_t sreg = SREG;
    cli();
    f->count--;
    SREG = sreg;
    
    return data;
}

/**************************************************************************************************
 *    Function      : fifo_init
 *    Description   : initializes a fifo element
 *    Input         : fifo_t*, uint8_t* , const uint8_t 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void fifo_init (fifo_t *f, uint8_t *buffer, const uint8_t size)
{
    f->count = 0;
    f->pread = f->pwrite = buffer;
    f->read2end = f->write2end = f->size = size;
}

/**************************************************************************************************
 *    Function      : fifo_put
 *    Description   : adds an element to the fifo
 *    Input         : fifo_t*, const uint8_t* 
 *    Output        : uint8_t 
 *....Remarks       : If return code equal 1 the operation was successfully 
 **************************************************************************************************/
uint8_t fifo_put (fifo_t *f, const uint8_t data)
{
    return _inline_fifo_put (f, data);
}

/**************************************************************************************************
 *    Function      : fifo_get_wait
 *    Description   : Gets an element form the FiFo, blocks till one is available 
 *    Input         : fifo_t*
 *    Output        : uint8_t 
 *....Remarks       : Blocks till data is in FiFo
 **************************************************************************************************/
uint8_t fifo_get_wait (fifo_t *f)
{
    while (!f->count);
    
    return _inline_fifo_get (f);    
}

/**************************************************************************************************
 *    Function      : fifo_get_nowait
 *    Description   : Gets an element form the FiFo
 *    Input         : fifo_t*
 *    Output        : int16_t 
 *....Remarks       : Returns values less than zero if ther is no Data 
 **************************************************************************************************/
int fifo_get_nowait (fifo_t *f)
{
    if (!f->count)        return -1;
        
    return (int) _inline_fifo_get (f);    
}

/**************************************************************************************************
 *    Function      : fifo_get_item_count
 *    Description   : Gets the amount of elements in FiFo
 *    Input         : fifo_t*
 *    Output        : int16_t 
 *....Remarks       : none
 **************************************************************************************************/
uint8_t fifo_get_item_count(fifo_t *f)
{
    return f->count; 
}
