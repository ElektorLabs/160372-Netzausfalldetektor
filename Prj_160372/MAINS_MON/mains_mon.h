/*
 * int0.h
 *
 * Created: 04.04.2018 09:50:04
 *  Author: mathiasc
 */ 


#ifndef MAINS_MON_H_
#define MAINS_MON_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    MAINS_OK,
    MAINS_RECOVERING,
    MAINS_FAIL
} MAINS_STATUS_t;

typedef struct {
    MAINS_STATUS_t Status; 
    uint16_t FreqDeziHz;
}MAINS_MON_Status_t;

/**************************************************************************************************
 *    Function      : MAINS_MON_init
 *    Description   : Init of the mains monitor
 *    Input         : bool 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void MAINS_MON_init(bool InitwithPowerloss);

/**************************************************************************************************
 *    Function      : MAINS_MON_GetStatus
 *    Description   : Gets the current mains status
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
MAINS_MON_Status_t MAINS_MON_GetStatus();

#endif /* MAINS_MON_H_ */