/*
 * main.c
 *
 *  created on: May 3, 2019
 *      Author: Yubo-ASUS
 */



#include "application\global.h"
#include "string.h"

// record the current connected capacitor number
#pragma PERSISTENT(SW_Mode)
SW_Mode_t SW_Mode = DEFAULT;



uint8_t ModelTrainPinValue;
uint8_t DebugOntheFly = 0;
uint8_t restart = 1;


/* which mode you are running: learning or testing
 * learning phase (relating to Phase-1 and Phase-2) : running RL until it converges
 * testing phase (relating to Phase-3): after RL is converged, running converged RL to test its performance in event detection
 * */
uint8_t learning = 1;

void initClock(void);
void initGPIO(void);


void main(){

    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    initClock();
    initGPIO();

    int i;


    /* start SVS monitoring */
    initTimerB_SupplyVoltage(15);   // the parameter means how frequently(in seconds) you want to check the voltage, do not set it to be a multiple of 10


    if(GPIO_getInputPinValue(MemoryReadPin) == GPIO_INPUT_PIN_LOW ){                        // if LOW, execute program, otherwise, do not execute any code

        NumOfRestart ++;  // increase NumOfRestart by one before we start runQlearning() or testQLearning;

        if(learning)
            runQLearning();  // Phase-2 learning phase, running RL until it converges
        else
            testQLearning();   // Phase-3 testing, after RL is converged, we test how well it does in event detection


        // if you want to do Phase-1, call Phase1ConvergeRate() and comment the above if-else code snippet
    }


    __bis_SR_register(LPM3_bits + GIE);




}






void initClock()
{
    // Configure one FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_1;  // uncomment this line if >8MHz

    // using register-level command to set up the clock
    // this one works the same as the DriverLib version below
//    // Clock System Setup
//    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
//    CSCTL1 = DCOFSEL_6;                     // Set DCO, DCOFSEL_6 = 8MHz
//    CSCTL2 = SELM__DCOCLK | SELS__DCOCLK | SELA__VLOCLK;
//    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers to 1
//    CSCTL4 = LFXTOFF | HFXTOFF;             // save power
//    CSCTL0_H = 0;                           // Lock CS registers

    // the DriverLib version of clock systems setup for clock
    CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_6);   // set DCO to 8 MHz
    CS_initClockSignal(CS_ACLK, CS_VLOCLK_SELECT, CS_CLOCK_DIVIDER_1);  // ACLK to VLO(9.4KHz for FR5994), divided by 1
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);  // MCLK to DCO, divided by 1
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1); // SMCLK to DCO, divided by 1
    CS_turnOffLFXT();  // Stops the LFXT oscillator, save power
    CS_turnOffHFXT();  // Stops the HFXT oscillator, save power
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


    // for on board LEDs, green and red LEDs
    GPIO_setAsOutputPin(REDLED);
    GPIO_setAsOutputPin(GREENLED);


    /* for reading memory with UniFlash without executing the code
     *  if this pin is LOW, then execute program
     *  if this pin is HIGH, do not execute, will use UniFlash to read memory values
     */
    GPIO_setAsInputPin(MemoryReadPin);  // #define MemoryReadPin
    GPIO_setAsInputPin(ModelTrainPin);  // train model or test model
    GPIO_setAsInputPin(DebugOntheFlyPin);




    /* set SW related pins */
    GPIO_setAsOutputPin(SW_CtrPins);    // we have four control pins
    GPIO_setOutputLowOnPin(SW_CtrPins); // all SWs are in OFF status from the beginning



    /* camera related pins */
    /* I2C pins */
    GPIO_setAsPeripheralModuleFunctionOutputPin(CAMERA_I2C,GPIO_PRIMARY_MODULE_FUNCTION);
    /* output SMCLK as clock to camera */
    GPIO_setAsPeripheralModuleFunctionOutputPin(CAMERA_CLK,GPIO_TERNARY_MODULE_FUNCTION);
    /* set camera data bus pin to input mode */
    GPIO_setAsInputPin(CAMERA_DATABUS);
    /* power up the camera */
    GPIO_setAsOutputPin(CMAERA_POWER);
    GPIO_setOutputLowOnPin(CMAERA_POWER);
    /* set into interrupt mode, this pin accepts FVLD to start reading data from the data bus  */
    /* set as input, trigger at rising edge, clear interrupt flag */
    GPIO_setAsInputPin(CAMERA_FVLD);
    GPIO_selectInterruptEdge(CAMERA_FVLD,GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_clearInterrupt(CAMERA_FVLD);
    /* set pin 1.0 function to DMAE0 input (DMA trigger) */
    GPIO_setAsPeripheralModuleFunctionInputPin(CAMERA_DMA,GPIO_SECONDARY_MODULE_FUNCTION);



    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;
}


