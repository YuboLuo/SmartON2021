/*
 * DS3132.h
 *
 *  Created on: May 3, 2019
 *      Author: Yubo-ASUS
 */

#ifndef DS3132_H_
#define DS3132_H_

#include <stdio.h>


//******************************************************************************
// Pin Config ******************************************************************
//******************************************************************************

#define LED_OUT     P1OUT
#define LED_DIR     P1DIR
#define LED0_PIN    BIT0
#define LED1_PIN    BIT1

// addresses
#define DS3231_I2C_ADDRESS 0x68
#define DS3231_REG_SECONDS    0x00
#define DS3231_REG_MINUTES    0x01
#define DS3231_REG_HOURS      0x02
#define DS3231_REG_DAYOFWEEK  0x03
#define DS3231_REG_DAYOFMONTH 0x04
#define DS3231_REG_MONTH      0x05
#define DS3231_REG_YEAR       0x06

#define SLAVE_ADDR  0x68


// other parameters
#define MAX_BUFFER_SIZE     20







// General I2C State Machine ***************************************************
typedef enum I2C_ModeEnum{
    IDLE_MODE,
    TX_REG_ADDRESS_MODE,
    RX_DATA_MODE,
    SWITCH_TO_RX_MODE,
} I2C_Mode;




// functions
void initI2C(void);
uint8_t decode_SM(uint8_t);
uint8_t decode_H(uint8_t);
void I2C_Master_ReadReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t count);

void GetTime(void); // get time and save time in a global variable



// functions for testing
void initTimerA0(void);   // set a timer to query time from DS3231 periodically
void test_DS3231(void); // print the time in calendar format
void get_time(void);  // get the time



#endif /* DS3132_H_ */




