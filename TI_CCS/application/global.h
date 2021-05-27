/*
 * global.h
 *
 *  Created on: May 3, 2019
 *      Author: Yubo-ASUS
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <msp430.h>
#include "driverlib.h"


#include "DS3231.h"
#include "audio.h"
#include "algorithm.h"
#include "SVS.h"
#include "helper.h"
#include "FFT.h"
#include "FFT_430.h"
#include "benchmark.h"

/* for debug, we need to turn on some LEDs; this if for conditional compilation */
#define DEBUG 0
#define TRAINING 0  // training = 1; testing = 0

#define SAMPLING_FREQUENCY              8000
#define __SYSTEM_FREQUENCY_MHZ__        8000000


// Define a fixed sample length
#define SAMPLES_LENGTH  256*5
//#define DataProcessLength 2000


// SW module ports
//#define SW1_CtrPin GPIO_PORT_P8, GPIO_PIN0   // high--turn on; low--trun off
//#define SW2_CtrPin GPIO_PORT_P3, GPIO_PIN1
//#define SW3_CtrPin GPIO_PORT_P1, GPIO_PIN5
//#define SW4_CtrPin GPIO_PORT_P3, GPIO_PIN3
#define SW1_CtrPin GPIO_PORT_P3, GPIO_PIN0   // high--turn on; low--trun off
#define SW2_CtrPin GPIO_PORT_P3, GPIO_PIN1
#define SW3_CtrPin GPIO_PORT_P3, GPIO_PIN2
#define SW4_CtrPin GPIO_PORT_P3, GPIO_PIN3


#define SW_Add GPIO_PORT_P4, GPIO_PIN4

#define LEDLoad GPIO_PORT_P6, GPIO_PIN3     // blink an LED: add extra power load for event detection

// special function pins for debug
#define ModelTrainPin GPIO_PORT_P1, GPIO_PIN2
#define MemoryReadPin GPIO_PORT_P5, GPIO_PIN7  // LOW: execute program, HIGH: read memory
//#define StartPin GPIO_PORT_P6, GPIO_PIN3         // after reboot, we check this pin, if it is high
#define DebugOntheFlyPin GPIO_PORT_P8, GPIO_PIN1  // sometimes, we need to debug without reprogramming the board, if this pin is High, the RED will blink while sensing

// For DS3231, Connect SDA and SCL with MOSFET in case of the voltage leak of RTC's battery to the main board
//#define RTC_SDA_CtrPin GPIO_PORT_P5, GPIO_PIN2
//#define RTC_SCL_CtrPin GPIO_PORT_P1, GPIO_PIN4
#define RTC_SDA GPIO_PORT_P7, GPIO_PIN0
#define RTC_SCL GPIO_PORT_P7, GPIO_PIN1
#define RTC_SDA_CtrPin GPIO_PORT_P8, GPIO_PIN0
#define RTC_SCL_CtrPin GPIO_PORT_P4, GPIO_PIN7



/* record the current capacitor opening status
 * When restart from a power-off, it is DEFAULT
 * after turn on the first SW capacitor, it is SW0
 * turn on the second, it is SW1, and etc.
 */
typedef enum
{
  DEFAULT = 0,
  SW1,
  SW2,
  SW3,
  SW4
} SW_Mode_t;


/*
 * external variables from audio.c
 */
extern uint16_t results_VCC_num;
extern uint16_t results_VCC[];
extern uint16_t results_Audio_num;
extern uint16_t results_Audio[];
extern uint16_t dataRecorded[];
extern uint16_t* DataProcess;      // we only process the second half part of sampled audio data
extern uint8_t recording;
extern uint16_t VCC_sample_num;


/*
 * external variables from DS3231.c
 */
extern uint16_t Time;


/*
 * external variables from SVS.c
 */
extern uint8_t SupplyVoltageFlag;
//extern uint16_t Data[][];

/*
 * external variables from algorithm.c
 */
extern float qtable[][];
extern uint8_t StateSwitchFlag;
extern uint8_t PeriodicWakingupFlag;
extern uint16_t TestResults[1200][2];
extern uint16_t NumOfSense;
extern uint16_t NumOfRestart;
extern uint8_t PowerOutageFlag;
extern uint16_t ConvergedFlag;
extern uint16_t SavedQtableCnt;

/*
 * external variables from main.c
 */
extern uint8_t ModelTrainPinValue;  // 0=test; 1=train
extern SW_Mode_t SW_Mode;
extern uint8_t DebugOntheFly;






#endif /* GLOBAL_H_ */
