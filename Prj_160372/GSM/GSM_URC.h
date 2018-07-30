/*
 * GSM_URC.h
 *
 * Created: 05.04.2018 08:31:40
 *  Author: mathiasc
 */ 


#ifndef GSM_URC_H_
#define GSM_URC_H_

/* This are the URC Messages , things that can be tranmitted by the modem without issuing a AT Command */
const char GSM_URC_00[] PROGMEM = "+CMTI:";
const char GSM_URC_01[] PROGMEM = "+CMT:";
const char GSM_URC_02[] PROGMEM = "+CMT:";
const char GSM_URC_03[] PROGMEM = "+CBM:";
const char GSM_URC_04[] PROGMEM = "+CBM:";
const char GSM_URC_05[] PROGMEM = "+CDS:";
const char GSM_URC_06[] PROGMEM = "+CDS:";
const char GSM_URC_07[] PROGMEM = "+CGEV:NW DEACT";
const char GSM_URC_08[] PROGMEM = "+CGEV_ME DEACT";
const char GSM_URC_09[] PROGMEM = "+CGEV:NW DEACT";
const char GSM_URC_10[] PROGMEM = "+CGEV:ME DEACT";
const char GSM_URC_11[] PROGMEM = "+CGREG:1";
const char GSM_URC_12[] PROGMEM = "+CGREG:0";
const char GSM_URC_13[] PROGMEM = "+CREG:1,";
const char GSM_URC_14[] PROGMEM = "+CREG:0,";
const char GSM_URC_15[] PROGMEM = "+CSQN:";
const char GSM_URC_16[] PROGMEM = "";
const char GSM_URC_17[] PROGMEM = "+CMWT:";
const char GSM_URC_18[] PROGMEM = "+QGURC:";
const char GSM_URC_19[] PROGMEM = "+CBCN";
const char GSM_URC_20[] PROGMEM = "+QBAND";
const char GSM_URC_21[] PROGMEM = "+TSMSINFO:";
const char GSM_URC_22[] PROGMEM = "+CCINFO:";
const char GSM_URC_23[] PROGMEM = "RING";
const char GSM_URC_24[] PROGMEM = "Call Ready";
const char GSM_URC_25[] PROGMEM = "UNDER_VOLTAGE";
const char GSM_URC_26[] PROGMEM = "UNDER_VOLTAGE";
const char GSM_URC_27[] PROGMEM = "OVER_VOLTAGE";
const char GSM_URC_28[] PROGMEM = "OVER_VOLTAGE";
const char GSM_URC_29[] PROGMEM = "UNDER_VOLTAGE";
const char GSM_URC_30[] PROGMEM = "+COPL:";
const char GSM_URC_31[] PROGMEM = "+CLIP:";
const char GSM_URC_32[] PROGMEM = "+CRING:";
const char GSM_URC_33[] PROGMEM = "+CREG:";
const char GSM_URC_34[] PROGMEM = "+CREG:";
const char GSM_URC_35[] PROGMEM = "+CCWA:";
const char GSM_URC_36[] PROGMEM = "RDY";
const char GSM_URC_37[] PROGMEM = "+CFUN:1";
const char GSM_URC_38[] PROGMEM = "+CPIN:";
const char GSM_URC_39[] PROGMEM = "MO RING";
const char GSM_URC_40[] PROGMEM = "MO CONNECTED";
const char GSM_URC_41[] PROGMEM = "ALARM RING";
const char GSM_URC_42[] PROGMEM = "ALARM MODE";
/* Beyound this we add some other codes to make it possible to have URC in front of an AT Resonse */


/* This is the Table for the URC Strings and a way to handel them more or less programaticylly */
PGM_P const pgmGSM_URC_Table[] PROGMEM =
{
    GSM_URC_00,
    GSM_URC_01,
    GSM_URC_02,
    GSM_URC_03,
    GSM_URC_04,
    GSM_URC_05,
    GSM_URC_06,
    GSM_URC_07,
    GSM_URC_08,
    GSM_URC_09,
    GSM_URC_10,
    GSM_URC_11,
    GSM_URC_12,
    GSM_URC_13,
    GSM_URC_14,
    GSM_URC_15,
    GSM_URC_16,
    GSM_URC_17,
    GSM_URC_18,
    GSM_URC_19,
    GSM_URC_20,
    GSM_URC_21,
    GSM_URC_22,
    GSM_URC_23,
    GSM_URC_24,
    GSM_URC_25,
    GSM_URC_26,
    GSM_URC_27,
    GSM_URC_28,
    GSM_URC_29,
    GSM_URC_30,
    GSM_URC_31,
    GSM_URC_32,
    GSM_URC_33,
    GSM_URC_34,
    GSM_URC_35,
    GSM_URC_36,
    GSM_URC_37,
    GSM_URC_38,
    GSM_URC_39,
    GSM_URC_40,
    GSM_URC_41,
    GSM_URC_42
};



#endif /* GSM_URC_H_ */