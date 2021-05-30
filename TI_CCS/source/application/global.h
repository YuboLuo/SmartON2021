/*
 * global.h
 *
 *  Created on: May 3, 2019
 *      Author: Yubo-ASUS
 */




//******************************************************************************
//!
//!  Description: pin usage
//!
//!                  MSP430FR5994               SW: add capacitors on the fly, switches
//!               -----------------             -----------------
//!              |             P3.0|---------> | SW1             |
//!              |             P3.1|---------> | SW2             |
//!              |             P3.2|---------> | SW3             |
//!              |             P3.3|---------> | SW4             |
//!              |-----------------|           |-----------------|

//!                                             other pins
//!               -----------------             -----------------
//!              |             P5.7|---------> | PushButtonStart |
//!              |             P8.3|---------> | ModelTrainPin   |
//!              |             P3.7|---------> | MemoryReadPin   |
//!              |             P8.2|---------> | DebugOntheFlyPin|
//!              |             P8.1|---------> | REDLED          |
//!              |             P1.1|---------> | GREENLED        |
//!              |-----------------|           |-----------------|

//!                                             audio sensor
//!               -----------------             -----------------
//!              |            *P1.3|<--------- | MIC OUT         |
//!              |            *P6.2|---------> | MIC PWR         |
//!              |-----------------|           |-----------------|


//!                                             RTC DS3231
//!               -----------------             -----------------
//!              |            *P7.0|<--------> | SDA RTC         |
//!              |            *P7.1|<--------> | SCL RTC         |
//!              |             P8.0|---------> | SDA RTC ctr     |
//!              |             P4.7|---------> | SCL RTC ctr     |
//!              |-----------------|           |-----------------|

//!                                             camera sensor
//!               -----------------             -----------------
//!              |            *P5.0|<--------> | SDA camera      |
//!              |            *P5.1|<--------> | SCL camera      |
//!              |             P3.6|---------> | Camera PWR      |
//!              |             P5.3|<--------- | FVLD            |
//!              |            *P3.4|---------> | MCLK            |
//!              |            *P1.0|<--------- | PCLK/DMAE0      |
//!              |                 |           |                 |
//!              |             P4.1|<--------- | D0 data bus     |
//!              |             P4.2|<--------- | D1 data bus     |
//!              |             P4.3|<--------- | D2 data bus     |
//!              |             P4.4|<--------- | D3 data bus     |
//!              |                 |           |                 |
//!              |             P2.5|---------> | TRIG            |
//!              |             P2.6|<--------- | LVLD            |
//!              |-----------------|           |-----------------|


//!                                             Pins for future use
//!               -----------------             -----------------
//!              |             P7.3|---------> |                 |
//!              |             P3.5|---------> |                 |
//!              |-----------------|           |-----------------|



//  our final SpotON PCB design follows this pin map

//   '*' means those pins are fixed special pins for special functions

//******************************************************************************




#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <msp430.h>
#include "driverlib.h"


#include "DS3231.h"
#include "audio.h"
#include "camera.h"
#include "algorithm.h"
#include "SVS.h"
#include "FFT.h"
#include "FFT_430.h"
#include "benchmark.h"

/* for debug, we need to turn on some LEDs; this if for conditional compilation */
#define DEBUG 0
#define TRAINING 1  // training = 1; testing = 0

#define SAMPLING_FREQUENCY              8000
#define __SYSTEM_FREQUENCY_MHZ__        8000000


// Define a fixed sample length
#define SAMPLES_LENGTH  256



/* SW module ports */
#define SW_CtrPins GPIO_PORT_P3, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3
#define SW1_CtrPin GPIO_PORT_P3, GPIO_PIN0   // high--turn on; low--trun off
#define SW2_CtrPin GPIO_PORT_P3, GPIO_PIN1
#define SW3_CtrPin GPIO_PORT_P3, GPIO_PIN2
#define SW4_CtrPin GPIO_PORT_P3, GPIO_PIN3
#define SW_add_test GPIO_PORT_P6, GPIO_PIN3


/* LEDs */
#define REDLED GPIO_PORT_P8, GPIO_PIN1
#define GREENLED GPIO_PORT_P1, GPIO_PIN1



/* special function pins for debug */
#define PushButtonStart GPIO_PORT_P5, GPIO_PIN7
#define ModelTrainPin GPIO_PORT_P4, GPIO_PIN3
#define MemoryReadPin GPIO_PORT_P3, GPIO_PIN7  // LOW: execute program, HIGH: read memory
#define DebugOntheFlyPin GPIO_PORT_P8, GPIO_PIN2  // sometimes, we need to debug without reprogramming the board, if this pin is High, the RED will blink while sensing


/* DS3231 I2C; use a MOSFET switch to control the RTC */
#define RTC_I2C GPIO_PORT_P7, GPIO_PIN0|GPIO_PIN1
#define RTC_I2C_CtrPins GPIO_PORT_P7, GPIO_PIN2|GPIO_PIN3
#define RTC_SDA GPIO_PORT_P7, GPIO_PIN0
#define RTC_SCL GPIO_PORT_P7, GPIO_PIN1
#define RTC_SDA_CtrPin GPIO_PORT_P7, GPIO_PIN2
#define RTC_SCL_CtrPin GPIO_PORT_P7, GPIO_PIN3


/* camera */
#define CAMERA_I2C GPIO_PORT_P5, GPIO_PIN0|GPIO_PIN1  // I2C pins, communicate with camera
#define CAMERA_CLK GPIO_PORT_P3, GPIO_PIN4   // output SMCLK as clock to camera
#define CAMERA_DATABUS  GPIO_PORT_P4, GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4 // camera data bus pins, transmit the image data
#define CMAERA_POWER GPIO_PORT_P3,GPIO_PIN6  // control a MOSFET switch to turn on/off the camera's power
#define CAMERA_FVLD GPIO_PORT_P5,GPIO_PIN3   // frame valid signal
#define CAMERA_DMA GPIO_PORT_P1,GPIO_PIN0    // DMAE0, camera outputs PCLK as DMA's trigger signal to read data bus


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
extern uint8_t restart;





#endif /* GLOBAL_H_ */
