/*
 * adc.h
 *
 * Created: 03.04.2018 14:16:50
 *  Author: mathiasc
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>




/**************************************************************************************************
 *    Function      : ADC_init
 *    Description   : ADC initialization 
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void ADC_init(void);

/**************************************************************************************************
 *    Function      : ADC_StartConversation
 *    Description   : starts a new ADC conversation
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void ADC_StartConversation();


#endif /* ADC_H_ */