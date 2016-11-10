/*
 * Generic CL90135 leds interface.
 * 
 * Copyright (C) 2001 Cielle srl
 */
#ifndef _LINUX_CL90135_LDES_H_
#define _LINUX_CL90135_LDES_H_
//  
//  MICHELE: DEFINIZIONE GPIO 4_4..7 COME USCITE DI COMANDO LED
//
//      GPIO 4_4=0       --> LED ROSSO ACCESO
//      GPIO 4_4=1       --> LED ROSSO SPENTO
//      GPIO 4_{5,6,7}=0 --> LED VERDE 1,2,3 ACCESO
//      GPIO 4_{5,6,7}=1 --> LED VERDE 1,2,3 SPENTO
//
#define ON_BD_RED_LED	    GPIO(4*16+4)	// GPIO4[4]
#define ON_BD_GREEN_LED_1	GPIO(4*16+5)	// GPIO4[5]
#define ON_BD_GREEN_LED_2	GPIO(4*16+6)	// GPIO4[6]
#define ON_BD_GREEN_LED_3	GPIO(4*16+7)	// GPIO4[7]

// enum the leds on CL90135
typedef enum{
    enum_CL90135_led_red=0,
    enum_CL90135_led_green1,
    enum_CL90135_led_green2,
    enum_CL90135_led_green3,
    enum_CL90135_led_numOf
}enum_CL90135_led;

typedef struct _TipoStruct_Initialize_CL90135_leds{
    unsigned int uiGPIO;    // the gpio correct code
    unsigned int uiValue;   // the init value
    char        *name;      // the name
}TipoStruct_Initialize_CL90135_leds;
extern TipoStruct_Initialize_CL90135_leds init_CL90135_leds[enum_CL90135_led_numOf];

//
// ioctl calls that are permitted to the /dev/rtc interface, if
// any of the CL90135_LED drivers are enabled.
typedef enum{
    enumCL90135_Led_command_on=0,
    enumCL90135_Led_command_off,
    enumCL90135_Led_command_toggle,
    enumCL90135_Led_command_get_value,
    enumCL90135_Led_command_all_off,
    enumCL90135_Led_command_all_on,
    enumCL90135_Led_command_all_toggle,
    enumCL90135_Led_command_numof
}enumCL90135_Led_command;

typedef struct _TipoStruct_CL90135Led_Command{
    enumCL90135_Led_command theAction;
    enum_CL90135_led theLed;
    unsigned int uiTheValue;
}TipoStruct_CL90135Led_Command;

// with this interface U can set/reset/toggle/get led values
// if u sepcify enumCL90135_Led_command_get_value, the funztion returns 1 if led is lit, 0 else
#define CL90135_LED_COMMAND	_IOWR('p', 0x01, TipoStruct_CL90135Led_Command)	    



#endif // _LINUX_CL90135_LDES_H_
