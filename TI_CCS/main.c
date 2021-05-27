/*
 * main.c
 *
 *  Created on: May 3, 2019
 *      Author: Yubo-ASUS
 */



#include "application\global.h"
#include "string.h"

// record the current connected capacitor number
#pragma PERSISTENT(SW_Mode)
SW_Mode_t SW_Mode = DEFAULT;

//uint16_t SW_Mode = 0;

uint8_t ModelTrainPinValue;
uint8_t DebugOntheFly = 0;


void initClock(void);
void initGPIO(void);


void main(){

    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    initClock();
    initGPIO();

    int i;


    /* Turn on LED showing the board is rebooting*/
    P1OUT |= BIT0;  // turn on Green LED
    __delay_cycles(8000000);
    P1OUT &= ~BIT0; // turn off Green LED


    if((SavedQtableCnt >= 10 && TRAINING == 1) || ( (SavedQtableCnt >= 11 || NumOfSense >= 1200) && TRAINING == 0) ){ // for training
        while(1){
            for(i=0; i<3; i++){  // blink
                P1OUT |= BIT0;
                __delay_cycles(500000);
                P1OUT &= ~BIT0;
                __delay_cycles(500000);
            }
            __delay_cycles(1500000);
            __delay_cycles(1500000);

        }
    }





    if(PowerOutageFlag == 0){ // only check RTC after a new erase

        /* for check if RTC is ready */
        GPIO_setAsInputPin(RTC_SDA);
        GPIO_setAsInputPin(RTC_SCL);
        uint8_t ResultSDA,ResultSCL;
        while(1){
            GPIO_setOutputHighOnPin(RTC_SDA_CtrPin);
            GPIO_setOutputHighOnPin(RTC_SCL_CtrPin);
            P1OUT |= BIT0;
            __delay_cycles(2000000);
            P1OUT &= ~BIT0;
            ResultSDA = GPIO_getInputPinValue(RTC_SDA); // read the two signal pins, one RTC is turned on, the pins should be HIGH
            ResultSCL = GPIO_getInputPinValue(RTC_SCL);
            GPIO_setOutputLowOnPin(RTC_SDA_CtrPin);
            GPIO_setOutputLowOnPin(RTC_SCL_CtrPin);
            if( ResultSDA == GPIO_INPUT_PIN_HIGH && ResultSCL == GPIO_INPUT_PIN_HIGH )
                break;
            __delay_cycles(2000000);
        }
        // check if GetTime works well
//        GetTime();
        for(i=0; i<3; i++){  // blink three times
            P1OUT |= BIT0;
            __delay_cycles(500000);
            P1OUT &= ~BIT0;
            __delay_cycles(500000);
        }
    }


//    else{ // if a power outage happens
//        for(i=0; i<3; i++){  // blink three times
//            P1OUT |= BIT0;
//            __delay_cycles(500000);
//            P1OUT &= ~BIT0;
//            __delay_cycles(500000);
//        }
//    }





    /* for the experiments related to different energy entry levels, we have to modify here */
    /* modify here accordingly */
    SW_Mode = SW2;
    GPIO_setOutputHighOnPin(SW1_CtrPin);
    GPIO_setOutputHighOnPin(SW2_CtrPin);
//    GPIO_setOutputHighOnPin(SW3_CtrPin);
//    GPIO_setOutputHighOnPin(SW4_CtrPin);

//    SW_Mode = DEFAULT;


    /* start SVS monitoring */
//    initTimerB_SupplyVoltage(8);



    uint8_t MemoryReadPinValue = GPIO_getInputPinValue(MemoryReadPin);   // get input pin value
    if(MemoryReadPinValue == GPIO_INPUT_PIN_LOW ){                        // if LOW, execute program, otherwise, do not execute any code



        NumOfRestart ++;  // increase NumOfRestart by one before we start runQlearning()

        if (TRAINING == 1)
            runQLearning();  // for training, RLEnergy
        else
            testQLearning();   // for testing, RLEnergy


//        RLEventConvergeRate();  // for RLEvent



    }


    __bis_SR_register(LPM3_bits + GIE);




}






void initClock()
{
    // Configure one FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_1;  // uncomment this line if >8MHz

    // Clock System Setup
    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
    CSCTL1 = DCOFSEL_6;                     // Set DCO, DCOFSEL_0 = 1 MHz, DCOFSEL_6 = 8MHz
    CSCTL2 = SELM__DCOCLK | SELS__DCOCLK | SELA__VLOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers to 1
    CSCTL4 = LFXTOFF | HFXTOFF;             // save power
    CSCTL0_H = 0;                           // Lock CS registers

//    // the DriverLib version of clock systems setup for clock
//    CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_0);   // set DCO to 1 MHz
//    CS_initClockSignal(CS_ACLK, CS_VLOCLK_SELECT, CS_CLOCK_DIVIDER_1);  // ACLK to VLO(9.4KHz for FR5994), divided by 1
//    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);  // MCLK to DCO, divided by 1
//    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1); // SMCLK to DCO, divided by 1
//    CS_turnOffLFXT();  // Stops the LFXT oscillator, save power
//    CS_turnOffHFXT();  // Stops the HFXT oscillator, save power
}



void initGPIO()
{

    // Configure GPIO
    // save energy
    PADIR=0xffff; PAOUT=0x0000; // Ports 1 and 2
    PBDIR=0xffff; PBOUT=0x0000; // Ports 3 and 4
    PCDIR=0xffff; PCOUT=0x0000; // Ports 5 and 6
    PDDIR=0xffff; PDOUT=0x0000; // Ports 7 and 8
    PJDIR=0xff; PJOUT=0x00;     // Port J

    // for DS3132
    // I2C pins
//    P7SEL0 |= BIT0 | BIT1;
//    P7SEL1 &= ~(BIT0 | BIT1);


    // For DS3231, Connect SDA and SCL with MOSFET in case of the voltage leak of RTC's battery to the main board
    GPIO_setAsOutputPin(RTC_SDA_CtrPin);
    GPIO_setAsOutputPin(RTC_SCL_CtrPin);
    GPIO_setOutputLowOnPin(RTC_SDA_CtrPin);
    GPIO_setOutputLowOnPin(RTC_SCL_CtrPin);

    // for on board LEDs, green and red LEDs
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0|GPIO_PIN1);


    /* for reading memory with UniFlash without executing the code
     *  if this pin is LOW, then execute program
     *  if this pin is HIGH, do not execute, will use UniFlash to read memory values
     *  Why use P1.2? because in the Launchpad, P1.2 is beside to both GND and VCC
     *  we can easily use a jumper to connect P1.2 to VCC or GND without a wire
     */
    GPIO_setAsInputPin(MemoryReadPin);  // #define MemoryReadPin GPIO_PORT_P1, GPIO_PIN2
    GPIO_setAsInputPin(ModelTrainPin);  // train model or test model
    GPIO_setAsInputPin(SW_Add);  // if V_cap is charged to higher than 2.9V

//    GPIO_setAsInputPin(StartPin);  // if RTC is ON
    GPIO_setAsInputPin(DebugOntheFlyPin);  // if RTC is ON


    // *for SW control pin* //
    // set as output
    GPIO_setAsOutputPin(SW1_CtrPin);
    GPIO_setAsOutputPin(SW2_CtrPin);
    GPIO_setAsOutputPin(SW3_CtrPin);
    GPIO_setAsOutputPin(SW4_CtrPin);
    // all SWs are in OFF status from the beginning
    GPIO_setOutputLowOnPin(SW1_CtrPin);
    GPIO_setOutputLowOnPin(SW2_CtrPin);
    GPIO_setOutputLowOnPin(SW3_CtrPin);
    GPIO_setOutputLowOnPin(SW4_CtrPin);


    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;
}




