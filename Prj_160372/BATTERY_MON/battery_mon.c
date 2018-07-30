/*
 * supply_mon.c
 *
 * Created: 11.04.2018 08:23:39
 *  Author: mathiasc
 */ 

#include "..\ADC\adc.h"
#include "..\GPIO\gpio.h"
#include "battery_mon.h"


/* This are inner defines for VBAT */
#define VBAL_LOW_LEVEL_MV 3900
#define VBAT_EMPTY_LEVEL_MV 3700
#define VBAT_HYST_MV 100

/* as this is to be used from an ISR we want that to be in SRAM and not in any register */
volatile static BatteryStatus_t Battery;

/**************************************************************************************************
 *    Function      : voBATTERY_MON_init
 *    Description   : Init of the battery monitor
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
void BATTERY_MON_init(){
    
    /* We assume no battery connected */
    Battery.Batterystatus = VBAT_EMPTY;
    Battery.VBat = 0;
        
}

/**************************************************************************************************
 *    Function      : voBATTERY_MON_VBatUpdate
 *    Description   : Processes the ADC Value and updates the flags for the battery level
 *    Input         : uint16_t  
 *    Output        : none
 *....Remarks       : To be called from the ADC ISR with fresh value
 **************************************************************************************************/
void BATTERY_MON_VBatUpdate( uint16_t mVolt){
    
        Battery.VBat = mVolt;
        
        /* We now determine the first values for the VBat Level */
        
        switch ( Battery.Batterystatus){
            
            case VBAT_OK :{
                
                if( Battery.VBat < ( VBAT_EMPTY_LEVEL_MV - VBAT_HYST_MV ) ){
                    
                    Battery.Batterystatus = VBAT_LOW;
                    
                } else if( Battery.VBat < ( VBAL_LOW_LEVEL_MV - VBAT_HYST_MV ) ){
                   
                    Battery.Batterystatus = VBAT_EMPTY;
                }
                
                
                
            } break;
            
            case VBAT_LOW :{
                
                  if( Battery.VBat < ( VBAT_EMPTY_LEVEL_MV - VBAT_HYST_MV ) ){
                    
                    Battery.Batterystatus = VBAT_EMPTY;
                    
                  }                    
                    
                  if( Battery.VBat > ( VBAL_LOW_LEVEL_MV + VBAT_HYST_MV ) ){
                   
                    Battery.Batterystatus = VBAT_OK;
                  
                  }
                
                
            } break;
            
            case VBAT_EMPTY :{
                
                  if( Battery.VBat > ( VBAT_EMPTY_LEVEL_MV + VBAT_HYST_MV ) ){
                    
                    Battery.Batterystatus = VBAT_LOW;
                    
                  }                    
                    
                  if( Battery.VBat > ( VBAL_LOW_LEVEL_MV + VBAT_HYST_MV ) ){
                   
                    Battery.Batterystatus = VBAT_OK;
                  
                  }
                
            } break;
        
            
        }
}

/**************************************************************************************************
 *    Function      : strBATTERY_MON_GetInfo
 *    Description   : Returns the current battery information
 *    Input         : none 
 *    Output        : none
 *....Remarks       : none
 **************************************************************************************************/
BatteryStatus_t BATTERY_MON_GetInfo( ){
    
    /* We determine what soure is active, battery or power supply */
    if ( GPIOLOW == GPIO_get(SUPPLYSTAT) ) {
        Battery.Supplystatus = SUPPLY_FAIL;
        Battery.Powersource = BATTERYIN;
    } else {
        Battery.Supplystatus = SUPPLY_OK;
        Battery.Powersource =POWERSUPPLY;
    }
    return Battery;
}