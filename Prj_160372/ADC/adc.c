#include <avr/io.h>
#include <avr/cpufunc.h> 
#include <util/delay.h>
#include <avr/interrupt.h>
/* own include as last one */
#include "../BATTERY_MON/battery_mon.h"
#include "adc.h"

typedef enum {
    ADCCH0 = 0,
    ADCCH1,
    ADCCH2,
    ADCCH3,
    ADCCH4,
    ADCCH5,
    ADCCH6,
    ADCCH7,
    ADCTEMPSENSOR,
    RESERVED0,
    RESERVED1,
    RESERVED2,
    RESERVED3,
    RESERVED4,
    ADCBANDGAP,
    ADCGND
} ADCChannel_t;

static volatile uint32_t VBatAVG=0;
/**************************************************************************************************
 *    Function      : ADC_init
 *    Description   : ADC initalisation
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void ADC_init(){
    /* This will basically set up the ADC for max accuracy not speed */
    /* We need to archive something between 50kHz and 200kHz with 8MHz clock*/
    ADCSRA = ( (1<<ADEN) | ( 1 << ADPS2) | ( 1<<ADPS1) | ( 1<< ADPS0) | ( 1 << ADIE) ); 
    ADCSRB = (1<<ADLAR);
    /* This will enable the adc and set the prescaler to 128 */ 
    ADMUX = ( 0 << REFS0 ) | ( 0 << REFS1) | ( 1 << ADLAR ) | ( 0 << MUX2 ) | ( 0 << MUX1 ) | ( 0 << MUX0 );
    
    
}

/**************************************************************************************************
 *    Function      : ADC_StartConversation
 *    Description   : starts a new ADC conversation
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void ADC_StartConversation(){
    ADCSRA = ( ADCSRA & (~  (1 << ADSC) ) );
    ADCSRA |= (1<<ADSC);
}

/**************************************************************************************************
 *    Function      : ISR(ADC_vect)
 *    Description   : ADC interrupt
 *    Input         : none 
 *    Output        : none
 *....Remarks       : Will call BATTERY_MON_VBatUpdate( )
 **************************************************************************************************/
ISR(ADC_vect) {

/* VBat has an 10k / 100k voltage divider and we will see max 0,409V at 4,5Volt with this  
   this means 0xFFFF is 1,1V and we will have at the end 16,7849uV pro Bit                   
   We try to avoid float math as long as possible, cause it's slow and takes a lot of flash */
    
    uint16_t Result = ADC;
    /* We now replicate the Bit with index 6. */
    if( ( Result&( 1<<6 ) ) != 0){
        Result |= 0x3F;
    }
    /* With this we get the full range */
    ADCSRA = ( ADCSRA & (~ ( (1 << ADSC) | ( 1 << ADIF) ) ) );
    uint32_t Vbat_mV = Result;
    /* 
        Result is due to the schematic VBat / 2  with 2.8V Vref 
        This means one bit is  2,8V / 65535 Bit = 42,725261uV / Bit 
        to get the VBat in mV we can do [Result] *2,8 / 65535 and end up with
        Result * 20 / 234 to avpid flotingpoint math as good as possible 
    */    
    Vbat_mV=Vbat_mV*20;
    Vbat_mV=Vbat_mV/234;
    /* The Value schall be less tha UINT16_MAX */
    if(Vbat_mV <= UINT16_MAX){
        Result = (uint16_t)(Vbat_mV);
    } else {
        Result = UINT16_MAX;
    }
    
    VBatAVG = VBatAVG *( 16 -1);
    VBatAVG = VBatAVG + Result;
    VBatAVG = VBatAVG / 16;
  
    Result = (uint16_t)(VBatAVG);
    BATTERY_MON_VBatUpdate( Result);
    
}