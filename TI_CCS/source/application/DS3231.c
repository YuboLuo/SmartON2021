/*
 * DS3132.c
 *
 *  Created on: May 3, 2019
 *      Author: Yubo-ASUS
 */


//******************************************************************************
//   MSP430FR599x Demo - eUSCI_B2, I2C Master multiple byte TX
//
//   Description: I2C master communicates to I2C slave sending and receiving
//   3 different messages of different length. I2C master will enter LPM0 mode
//   while waiting for the messages to be sent/receiving using I2C interrupt.
//
//                                     /|\ /|\
//                   MSP430FR5994      4.7k |
//                 -----------------    |  4.7k
//            /|\ |             P7.1|---+---|-- I2C Clock (UCB2SCL)
//             |  |                 |       |
//             ---|RST          P7.0|-------+-- I2C Data (UCB2SDA)
//                |                 |
//
//   based on this demo example code, we use MSP430FR5994 to receive time info
//   from external real-time clock, DS3231.
//
//   DS3231 was not used in DCOSS2021 submission as we eliminated the use of RTC
//
//******************************************************************************



#include "global.h"

/* Used to track the state of the software state machine*/
I2C_Mode MasterMode = IDLE_MODE;

/* The Register Address/Command to use*/
uint8_t TransmitRegAddr = 0;


// define variables
uint8_t ReceiveBuffer[MAX_BUFFER_SIZE] = {0};
uint8_t RXByteCtr = 0;
uint8_t ReceiveIndex = 0;
uint8_t TransmitBuffer[MAX_BUFFER_SIZE] = {0};

// time
uint16_t Time;  // call GetTime() and time will be saved in Time variable

void GetTime(void){

    /* For DS3231, Connect SDA and SCL with MOSFET in case of the voltage leak of RTC's battery to the main board */
    /* we need to first turn On the MOSFET by making these two pins HIGH before reading the time from DS3231*/
    P7SEL0 |= BIT0 | BIT1;    // set the SDA and SCL pin for data transfer
    P7SEL1 &= ~(BIT0 | BIT1);

    GPIO_setOutputHighOnPin(RTC_SDA_CtrPin);
    GPIO_setOutputHighOnPin(RTC_SCL_CtrPin);

    /* delay 0.2 second for the HIGH pin getting stable
     * if there is no delay, RTC does not work well when powered by capacitor */
    __delay_cycles(2000000);

    initI2C();
    I2C_Master_ReadReg(SLAVE_ADDR, DS3231_REG_SECONDS, 3);

    Time = decode_SM(ReceiveBuffer[0]) + decode_SM(ReceiveBuffer[1]) * 60 + decode_H(ReceiveBuffer[2]) * 3600;
//    printf("%10u",time); printf("\n");

    /* we need to turn Off the MOSFET after reading the time form DS3231*/
    GPIO_setOutputLowOnPin(RTC_SDA_CtrPin);
    GPIO_setOutputLowOnPin(RTC_SCL_CtrPin);

}




void test_DS3231(void){
    initI2C();
    printf("System Starts...\n");
    printf("   SECONDS | MINUTES |   HOURS |  D_OF_W |  D_OF_M |   MONTH  |   YEAR \n");

    get_time();
}


// for testing
// read the Register(time) from the slave
void get_time(void){

    I2C_Master_ReadReg(SLAVE_ADDR, DS3231_REG_SECONDS, 3);
    printf("%10u",decode_SM(ReceiveBuffer[0]));
    printf("%10u",decode_SM(ReceiveBuffer[1]));
    printf("%10u",decode_H(ReceiveBuffer[2]));
    printf("\n");
}


void I2C_Master_ReadReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t count)
{
    /* Initialize state machine */
    MasterMode = TX_REG_ADDRESS_MODE;
    TransmitRegAddr = reg_addr;
    RXByteCtr = count;
    ReceiveIndex = 0;

    /* Initialize slave address and interrupts */
    UCB2I2CSA = dev_addr;
    UCB2IFG &= ~(UCTXIFG + UCRXIFG);       // Clear any pending interrupts
    UCB2IE &= ~UCRXIE;                       // Disable RX interrupt
    UCB2IE |= UCTXIE;                        // Enable TX interrupt

    UCB2CTLW0 |= UCTR + UCTXSTT;             // I2C TX, start condition
    __bis_SR_register(LPM3_bits + GIE);              // Enter LPM w/ interrupts

}




// init I2C for communication
void initI2C()
{
    UCB2CTLW0 = UCSWRST;                      // Enable SW reset
    UCB2CTLW0 |= UCMODE_3 | UCMST | UCSSEL__ACLK | UCSYNC; // ACLK = VLOCLK // original comment: I2C master mode, SMCLK
    UCB2BRW = 2;                            // fSCL = ACLK/2 = 4.7KHz // original code: fSCL = SMCLK/160 = ~100kHz
    UCB2I2CSA = SLAVE_ADDR;                   // Slave Address
    UCB2CTLW0 &= ~UCSWRST;                    // Clear SW reset, resume operation
    UCB2IE |= UCNACKIE;
}



// decode SECOND, MINUTE format
// copied from DS3132 library of C++ version
// http://www.rinkydinkelectronics.com/library.php?id=73
uint8_t decode_SM(uint8_t value)
{
    uint8_t decoded = value & 127;
    decoded = (decoded & 15) + 10 * ((decoded & (15 << 4)) >> 4);
    return decoded;
}


// decode HOUR format
// copied from DS3132 library of C++ version
// http://www.rinkydinkelectronics.com/library.php?id=73
uint8_t decode_H(uint8_t value)
{

  if (value & 128)
    value = (value & 15) + (12 * ((value & 32) >> 5));
  else
    value = (value & 15) + (10 * ((value & 48) >> 4));
  return value;
}


//// for testing
//// query the time from DS3231 periodically, every 1 second
//void initTimerA0(void){
//
//    // Start timer
//    // use TIMER_A1_BASE
//
//
//    Timer_A_clearTimerInterrupt(TIMER_A1_BASE);
//    Timer_A_initUpModeParam param = {0};
//        param.clockSource = TIMER_A_CLOCKSOURCE_ACLK;  // ACLK = VLOCLK = 9.4KHz
//        param.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;  // Divided by 1
//        param.timerPeriod = 9400;  // count up to this number (period), request time every second
//        param.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
//        param.captureCompareInterruptEnable_CCR0_CCIE =
//                TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
//        param.timerClear = TIMER_A_DO_CLEAR;
//        param.startTimer = true;
//    Timer_A_initUpMode(TIMER_A1_BASE, &param);
//
//}







//// for testing
//// timer interrupt service for DS3231
//#pragma vector=TIMER1_A0_VECTOR
//__interrupt void TIMERA0_ISR(void){
//
//    // stop the timer, clear MC to 00
//    Timer_A_stop(TIMER_A1_BASE);  // stop the counting of the timer
//    // clear the timer counter, so that when it starts next time, the counter starts from 0
//    // instead of starting from where it left
//    Timer_A_clear(TIMER_A1_BASE);
//
//    // get the current time
//    GetTime();
//
//    // start Timer
//    Timer_A_startCounter(TIMER_A1_BASE,TIMER_A_UP_MODE);
//}



//******************************************************************************
// I2C Interrupt ***************************************************************
//******************************************************************************

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B2_VECTOR
__interrupt void USCI_B2_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B2_VECTOR))) USCI_B2_ISR (void)
#else
#error Compiler not supported!
#endif
{
  //Must read from UCB2RXBUF
  uint8_t rx_val = 0;
  switch(__even_in_range(UCB2IV, USCI_I2C_UCBIT9IFG))
  {
    case USCI_NONE:          break;         // Vector 0: No interrupts
    case USCI_I2C_UCALIFG:   break;         // Vector 2: ALIFG
    case USCI_I2C_UCNACKIFG:                // Vector 4: NACKIFG
      break;
    case USCI_I2C_UCSTTIFG:  break;         // Vector 6: STTIFG
    case USCI_I2C_UCSTPIFG:  break;         // Vector 8: STPIFG
    case USCI_I2C_UCRXIFG3:  break;         // Vector 10: RXIFG3
    case USCI_I2C_UCTXIFG3:  break;         // Vector 12: TXIFG3
    case USCI_I2C_UCRXIFG2:  break;         // Vector 14: RXIFG2
    case USCI_I2C_UCTXIFG2:  break;         // Vector 16: TXIFG2
    case USCI_I2C_UCRXIFG1:  break;         // Vector 18: RXIFG1
    case USCI_I2C_UCTXIFG1:  break;         // Vector 20: TXIFG1
    case USCI_I2C_UCRXIFG0:                 // Vector 22: RXIFG0
        rx_val = UCB2RXBUF;
        if (RXByteCtr)
        {
          ReceiveBuffer[ReceiveIndex++] = rx_val;
          RXByteCtr--;
        }

        if (RXByteCtr == 1)
        {
          UCB2CTLW0 |= UCTXSTP;
        }
        else if (RXByteCtr == 0)
        {
          UCB2IE &= ~UCRXIE;
          MasterMode = IDLE_MODE;
          __bic_SR_register_on_exit(LPM3_bits);      // Exit LPM
        }
        break;
    case USCI_I2C_UCTXIFG0:                 // Vector 24: TXIFG0
        switch (MasterMode)
        {
          case TX_REG_ADDRESS_MODE:        // no matter in Rx or Tx mode, the first step is to transmit the
              UCB2TXBUF = TransmitRegAddr;
              MasterMode = SWITCH_TO_RX_MODE;   // Need to start receiving now
              break;

          case SWITCH_TO_RX_MODE:
              UCB2IE |= UCRXIE;              // Enable RX interrupt
              UCB2IE &= ~UCTXIE;             // Disable TX interrupt
              UCB2CTLW0 &= ~UCTR;            // Switch to receiver
              MasterMode = RX_DATA_MODE;    // State state is to receive data
              UCB2CTLW0 |= UCTXSTT;          // Send the TransmitRegAddr
              break;


          default:
              __no_operation();
              break;
        }
        break;
    default: break;
  }
}



