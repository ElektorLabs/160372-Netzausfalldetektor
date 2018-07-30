/*
elektor-project 1603372 
Netzausfalldetektor 

MCU used: Attiny 841 
FUSE Settigns are:
EXTENDED    0xFF
HIGH        0xDD
LOW         0xC2

Remarks: 
Use Atmelstudio 7.0.1645  with ATtiny DFP 1.3.172 
All required libary shall be in the project and bundled with the avr tool chain

*/


/* Include files for the project */ 
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <string.h>

#include ".\ADC\adc.h"
#include ".\GPIO\gpio.h"
#include ".\GSM\gsm.h"
#include ".\TIMER\timer.h"
#include ".\MAINS_MON\mains_mon.h"
#include ".\USART\usart.h"
#include ".\USART\usart_hlf.h"
#include ".\EEPROM_FS\EEPFS.h"
#include ".\BATTERY_MON\battery_mon.h"
#include ".\LED_LIGHT\led_light.h"

typedef enum {
    SMS_NOT_SEND=0,
    SMS_WAIT_CMD_RESPONSE,
    SMS_TRANSMISSION_OK,
    SMS_TRANSMISSION_ERROR,
} SMS_SEND_STATUS_t;

typedef enum {
     EVT_NO_EVENT=0,
     EVT_POWER_LOSS,
     EVT_POWER_RECOVER,
     EVT_BAT_LOW,
     EVT_BAT_EMPTY,
     EVT_NEW_CFG,
     EVT_REQ_STATUS,
} EVENT_TYPE_t;


typedef struct {
        EVENT_TYPE_t PowerEvent;
        SMS_SEND_STATUS_t SMSTXStatus;
} StatusFlags_t;


void voWaitForPowerGood( void );

int main(void)
 {
    /* We disable the watcHdog */      
    MCUSR = 0;
    wdt_disable();
    /* We switch off everything we don't need */
    PRR = ( ( 1 << PRTWI ) | ( 0 << PRUSART0) | ( 1 << PRSPI ) | ( 0 << PRTIM0) | ( 0 << PRTIM1) | ( 0 << PRTIM2) | ( 0 << PRADC) );
       
    /* local variabels for the statusmonitor */
    uint16_t NetStatusRequestLastTimeStamp=0;
    uint16_t NetStatusRequestTimeStamp=0;
    uint16_t TimeDelta=0;
    MAINS_MON_Status_t MainStatusCurrent;
    MAINS_MON_Status_t MainStatusLast;
    BatteryStatus_t Battery;
    StatusFlags_t StatusFlags;
    EEPFS_RemoteLineID_t LineID;
    EEPFS_Systemstatus_t SystemStatus;
    bool RemoteID_Fault=true;
    usartparam_t usart_parameter = {.NoOfBits=Eight,.Paritytype=None,.NoOfStopbits=One,.Doublespeed=0,.BaudrateRegister=51};
    memset((void*)&StatusFlags,0,sizeof(StatusFlags_t));
    /* Setup the GPIO */
    GPIO_init();
    /* Next is to setup the ADC */
    ADC_init();
    Timer_init(); /* We need a timebase */
    LED_LIGHT_Init();
   /* At this point we need interrupts */
    sei();
    /* We start the mains monitor and wait what will happen                                                      */
    /* If the Resetjumper is set we erase the eeprom an fall into deepsleep */
    if(GPIOLOW == GPIO_get(CLEAR_ALL) ){
        EEPFS_EraseAll();
    }     
    
    if( false == EEPFS_ReadSystemstatus(&SystemStatus) ){
        /* We write defautl Vlaues */
        SystemStatus.BatteryStatus=VBAT_OK;
        SystemStatus.PowerLoss = 0;
        SystemStatus.LastStatusReported=1;
        EEPFS_WriteSystemstatus( &SystemStatus);
    }
    
    if( false == EEPFS_ReadRemoteLineID(&LineID)){
        /* We set the LED to Blink */
       RemoteID_Fault=true;
    } else {
       RemoteID_Fault=false;
    }
    /* The Timer must be initialized at thi spoint */
    MAINS_MON_init(1 == SystemStatus.PowerLoss);
    MainStatusLast=MAINS_MON_GetStatus();
    BATTERY_MON_init();

  /* Setup the USART for 9600 Baud 8n1 */
    usart_init(&usart_parameter);
    Set_RX_Hook(GSM_RxData);
    
   do{
       
       voWaitForPowerGood(); /* This will issue a power down and a sleep command */
       
   } while( false ==  GSM_Init() );

    while ( 1 == 1 ) 
    {
        /* If we have nothing to do we went to IDLE */
        /*
        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_mode();
        */
        /*                                          */
        GSM_Task( );
        /* The GSM Task also calls the LED_LIGHT_Task() witch gives fast blinking led at init */
        /* We need to be able to call the service periodically */
        MainStatusCurrent=MAINS_MON_GetStatus();
        if( MAINS_FAIL == MainStatusCurrent.Status){
            /* We check if the Bat is low */
            LED_LIGHT_SetLed(LED_RED,LED_STATIC_OFF);
           // LED_LIGHT_SetLed(LED_ORANGE,LED_STATIC_OFF);
            LED_LIGHT_SetLed(LED_GREEN,LED_STATIC_OFF);
        } else {
            Battery=BATTERY_MON_GetInfo(); 
            switch(Battery.Batterystatus){
                case VBAT_EMPTY:{
                    LED_LIGHT_SetLed(LED_RED,LED_STATIC_ON);
                }break;
                
                case VBAT_LOW:{
                    LED_LIGHT_SetLed(LED_RED,LED_BLINK_5Hz);
                } break;
                
                case VBAT_OK:{
                    LED_LIGHT_SetLed(LED_RED,LED_STATIC_OFF);
                }
            
            } 
            
             if(false == GetGSMPowerDown() ){
            if(RemoteID_Fault==true){
                LED_LIGHT_SetLed(LED_GREEN,LED_BLINK_1Hz);
            } else {
                LED_LIGHT_SetLed(LED_GREEN,LED_STATIC_ON);
            }                
        }      
        }  
        
        NetStatusRequestTimeStamp = TIMER_GetTicks();
        if( NetStatusRequestTimeStamp < NetStatusRequestLastTimeStamp){
            TimeDelta = UINT16_MAX - NetStatusRequestLastTimeStamp + NetStatusRequestTimeStamp;
        } else {
            TimeDelta = NetStatusRequestTimeStamp - NetStatusRequestLastTimeStamp;
        } 
        
        if(60000<TimeDelta){
        /* We check every 10 Seconds if we are still loged in to the network */
             GSM_CMDSend(GSM_QNSTATUS);
            /* Set the intervall for the next request */
             NetStatusRequestLastTimeStamp = NetStatusRequestTimeStamp;
        }        
        
        /*    Insert Netstatus request here    */
             
                
        /* We need to see if something has changed here */
            if( ( StatusFlags.PowerEvent == EVT_NO_EVENT) && ( SMS_NOT_SEND ==  StatusFlags.SMSTXStatus ) && (0 == SystemStatus.LastStatusReported) ){
              /* What ever happend we need to report it */
               if( 1==SystemStatus.PowerLoss){
                StatusFlags.PowerEvent=EVT_POWER_LOSS;
               } else {
                StatusFlags.PowerEvent=EVT_POWER_RECOVER;   
               }               
               StatusFlags.SMSTXStatus = SMS_NOT_SEND;
            }
            
            if( ( MAINS_FAIL == MainStatusCurrent.Status ) && ( MainStatusCurrent.Status != MainStatusLast.Status) )  {
                /* We need to send an SMS */
                StatusFlags.PowerEvent=EVT_POWER_LOSS;
                StatusFlags.SMSTXStatus = SMS_NOT_SEND;
                SystemStatus.PowerLoss=1; 
                SystemStatus.LastStatusReported=0;
                EEPFS_WriteSystemstatus( &SystemStatus);
            }
            
            if( ( MAINS_OK == MainStatusCurrent.Status ) && ( MainStatusCurrent.Status != MainStatusLast.Status ) ){
                /* We need to send an SMS that the System has recovered */
                /* We need to send an SMS */
                StatusFlags.PowerEvent=EVT_POWER_RECOVER;
                StatusFlags.SMSTXStatus = SMS_NOT_SEND;
                SystemStatus.PowerLoss=0;
                SystemStatus.LastStatusReported=0;
                EEPFS_WriteSystemstatus( &SystemStatus);
            }
            
            if(  StatusFlags.PowerEvent == EVT_NO_EVENT){
                if(true ==  GSM_GetConfigChanged() ){
                  StatusFlags.PowerEvent=EVT_NEW_CFG;
                  StatusFlags.SMSTXStatus = SMS_NOT_SEND;
                  LED_LIGHT_SetLed(LED_GREEN, LED_STATIC_ON);
                } else if ( true == GSM_GetStatusRequested() ){
                  StatusFlags.PowerEvent=EVT_REQ_STATUS;
                  StatusFlags.SMSTXStatus = SMS_NOT_SEND;
                 
                }
            }
        
        
            switch ( StatusFlags.SMSTXStatus){
                case SMS_NOT_SEND:{
                    /* We need to start an SMS Transmission if possible */
                    switch(StatusFlags.PowerEvent){
                        case EVT_POWER_LOSS:{
                            if(CMD_OK == GSM_Send_SMS_P(SMS_POWERLOSS) ){
                                StatusFlags.SMSTXStatus = SMS_WAIT_CMD_RESPONSE; 
                            }
                        } break;
                            
                        case EVT_POWER_RECOVER:{
                            if(CMD_OK == GSM_Send_SMS_P(SMS_POWERRESTORED) ){
                                StatusFlags.SMSTXStatus = SMS_WAIT_CMD_RESPONSE;
                            }
                        } break;
                        
                        case EVT_BAT_LOW:{
                             if(CMD_OK == GSM_Send_SMS_P(SMS_BAT_LOW) ){
                                StatusFlags.SMSTXStatus = SMS_WAIT_CMD_RESPONSE;
                            }
                        } break;                             
                            
                        case EVT_BAT_EMPTY:{
                                 if(CMD_OK == GSM_Send_SMS_P(SMS_BAT_EMPTY) ){
                                StatusFlags.SMSTXStatus = SMS_WAIT_CMD_RESPONSE;
                            }
                        }break;
                        
                        case EVT_NEW_CFG:{
                             if(CMD_OK == GSM_Send_SMS_P(SMS_NEW_CONFIG) ){
                             StatusFlags.SMSTXStatus = SMS_WAIT_CMD_RESPONSE;
                             }                             
                        } break;  
                        
                        case EVT_REQ_STATUS:{
                            if(CMD_OK == GSM_Send_SMS_P(SMS_REQ_STATUS) ){
                                StatusFlags.SMSTXStatus = SMS_WAIT_CMD_RESPONSE;
                            }
                        } 
                        
                       case EVT_NO_EVENT:{
                           _NOP();
                       } break;
               
                    } break;
                        
                    case SMS_WAIT_CMD_RESPONSE:{
                        if( GSM_AT_CMGS == GSM_GetLastCommandResult().GSMCMD  ){
                            switch(GSM_GetLastCommandResult().GSMCmdResult){
                                case CMD_OK:{
                                    StatusFlags.SMSTXStatus = SMS_TRANSMISSION_OK;
                                } break;
                                    
                                case CMD_BUSY:
                                case CMD_PENDING:{
                                    StatusFlags.SMSTXStatus = SMS_WAIT_CMD_RESPONSE;
                                } break;
                                    
                                default:{
                                    StatusFlags.SMSTXStatus = SMS_TRANSMISSION_ERROR; 
                                } break;
                            }
                        }
                    }break;
                        
                    case SMS_TRANSMISSION_OK:{
                        /* We can clear the event */
                        /* This is misused for some post processing */
                        switch ( StatusFlags.PowerEvent){
                            case EVT_BAT_LOW:{
                                SystemStatus.BatteryStatus=VBAT_LOW;
                                EEPFS_WriteSystemstatus( &SystemStatus);
                            } break;
                            
                            case EVT_BAT_EMPTY:{
                                SystemStatus.BatteryStatus=VBAT_EMPTY;
                                EEPFS_WriteSystemstatus( &SystemStatus);
                            } break;
                            
                            case EVT_POWER_RECOVER:
                            case EVT_POWER_LOSS:{
                                SystemStatus.LastStatusReported=0;
                                EEPFS_WriteSystemstatus( &SystemStatus);
                            } break;
                            
                            default:{
                                
                            } break;
                        } /* Endo of Switch */
                        StatusFlags.PowerEvent = EVT_NO_EVENT;
                    } break;
                        
                    case SMS_TRANSMISSION_ERROR:{
                        /* We may need to add a touniut here if space if left */
                        StatusFlags.SMSTXStatus = SMS_NOT_SEND;
                    } break;
                }
            }
           
                   
        /* We save the MainStatus for the next RUN */
        MainStatusLast=MainStatusCurrent;

        /* Last part is to take care of the VBat from the Chemical Cells */
          if( VBAT_OK ==Battery.Batterystatus ){
              SystemStatus.BatteryStatus=Battery.Batterystatus;
              EEPFS_WriteSystemstatus( &SystemStatus);
          }
        
        /* If we run on Mains we will try to send all BatLevel if possible */
        
        if( ( VBAT_LOW == Battery.Batterystatus) && ( VBAT_LOW != SystemStatus.BatteryStatus) ){
            if(EVT_NO_EVENT == StatusFlags.PowerEvent ){
                StatusFlags.PowerEvent = EVT_BAT_LOW;
                StatusFlags.SMSTXStatus = SMS_NOT_SEND;
            }           
        } 
        
        if( ( VBAT_EMPTY== Battery.Batterystatus ) && (MAINS_OK == MainStatusCurrent.Status)  && ( VBAT_EMPTY !=  SystemStatus.BatteryStatus ) ){
            if(EVT_NO_EVENT == StatusFlags.PowerEvent ){
                StatusFlags.PowerEvent = EVT_BAT_EMPTY;
                StatusFlags.SMSTXStatus = SMS_NOT_SEND;
            }           
        } 
        
      /* At this point we don't have power from the mains and the chemical cells are empty, therefore we need to do a shutdown */
      if ( ( VBAT_EMPTY == Battery.Batterystatus ) && ( MAINS_FAIL == MainStatusCurrent.Status ) ){
        /* We try a propper shudown here */
            while( ( GSM_GetLastCommandResult().GSMCmdResult == CMD_BUSY ) || ( GSM_GetLastCommandResult().GSMCmdResult == CMD_PENDING) ){
                GSM_Task(); 
            }        
            GSM_Shutdown();
            wdt_enable(WDTO_2S);
            while ( 1 == 1 ){
                _NOP();
            }            
      }
      
       /* Last step is to check if we run on chemical cells and may disable the modem if nothing is to do */
    if( ( StatusFlags.PowerEvent == EVT_NO_EVENT) && ( SMS_NOT_SEND ==  StatusFlags.SMSTXStatus ) && (1 == SystemStatus.LastStatusReported) && ( MAINS_FAIL == MAINS_MON_GetStatus().Status ) ){
       if( false == GetGSMPowerDown()){
        GSM_Shutdown();
        voWaitForPowerGood();
        
       }        
    } 
      
                
    } /* End wile loop */
    
          
             
}


void voWaitForPowerGood(){
      /* We send a Shutdown the the Modem just to be sure */
      GSM_Shutdown();
      while( ( VBAT_EMPTY == BATTERY_MON_GetInfo().Batterystatus) && ( MAINS_FAIL == MAINS_MON_GetStatus().Status  ) ){
          /* Fore Modem Power off */
          /* Go to sleep and wake on interrupts */
          /* We need to enable the level interrupt for INT0 */
          EICRA = ( ( 0 << ISC11) | ( 0 << ISC10) ); /* Set to low level interrupt */
          set_sleep_mode(SLEEP_MODE_IDLE);
          sleep_mode();
          /* We wake only on ext INT0 */
      }
}


ISR(BADISR_vect){
    /* If we end here something in the system went wrong */
    
}