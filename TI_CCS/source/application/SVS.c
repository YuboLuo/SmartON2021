/*
 * SVS.c  -- Supply Voltage Supervisor
 *
 *  Created on: May 13, 2019
 *      Author: Yubo-ASUS
 */

/*
 * we need to monitor the voltage of the capacitor array.
 * LTC3105 has a PGOOD pin which outputs HIGH when the V_out reaches 90% of the regulated voltage
 * We can use one pin of MSP430 to connect to the PGOOD pin of LTC3105
 * Once read high, we turn on another SW and add another capacitor to the array
 */

#include "global.h"



uint8_t SupplyVoltageFlag = 0;
uint16_t SupplyVoltage;

uint16_t count = 0;

uint8_t mode = 1; // for testing the fixed system
uint8_t cnt = 0; // for testing the fixed system

/* Helper function, used individually
 * find the relation between VCC and sampled value
 */
void Find_RelationBetweenValueAndVCC(void){
    // *** Find the relation between the actual voltage and the detected value *** //
    // MSP430FR5994 board has an internal channel to monitor the board VCC voltage
    // but the value is not exactly the same as the voltage value. For example, when VCC is 3.3V, the value maybe 2000ish
    // We need to find out the relationship between them because we would need this information later when we want to
    // change the threshold voltage based on which we turn on a new SW.

    // let's save data in results_VCC[]

    // blink GREEN LED
    uint8_t i,j;
    for(i=0; i<5; i++){
        P1OUT |= BIT1; // turn on Green LED
        __delay_cycles(1600000); // 0.2s
        P1OUT &= ~BIT1;
        __delay_cycles(1600000); // 0.2s
    }
    __delay_cycles(8000000); // 1s
    __delay_cycles(8000000); // 1s


    uint16_t range;
    results_VCC_num = 0;

    /* save the first set of data */
    for(j=0; j<10; j++){
        P1OUT |= BIT1; // turn on Green LED
        __delay_cycles(1600000); // 1s
        P1OUT &= ~BIT1;
        __delay_cycles(1600000); // 1s

        runSampling_VCC();
        range = get_voltage();
        results_VCC[results_VCC_num++] = range;
    }

    /* save the rest sets of data */
    for(i=0; i<14; i++ ){
        __delay_cycles(8000000); // 1s
        __delay_cycles(8000000); // 1s
        __delay_cycles(8000000); // 1s
        __delay_cycles(8000000); // 1s
        __delay_cycles(8000000); // 1s
        __delay_cycles(8000000); // 1s

        for(j=0; j<10; j++){
            P1OUT |= BIT1; // turn on Green LED
            __delay_cycles(1600000); // 1s
            P1OUT &= ~BIT1;
            __delay_cycles(1600000); // 1s

            runSampling_VCC();
            range = get_voltage();
            results_VCC[results_VCC_num++] = range;
        }


    }

    __delay_cycles(8000000); // 1s
    __delay_cycles(8000000); // 1s
    P1OUT |= BIT1; // turn on Green LED

}


/* turn on a new SW
 * each time when the voltage of the capacitor array reaches the
 * pre-defined turn-on threshold, we add a new capacitor to the array
 * on-the-fly
 * */
void SW_AddNew(void){

    switch(SW_Mode){

        case DEFAULT: // currently no SW is on; let's turn on SW1
            GPIO_setOutputHighOnPin(SW1_CtrPin);
            SW_Mode = SW1;
            break;
        case SW1: // currently SW1 is on; let's turn on SW2
            GPIO_setOutputHighOnPin(SW2_CtrPin);
            SW_Mode = SW2;
            break;
        case SW2: // currently SW1/SW2 is on; let's turn on SW3
            GPIO_setOutputHighOnPin(SW3_CtrPin);
            SW_Mode = SW3;
            break;
        case SW3: // currently SW1/SW2/SW3 is on; let's turn on SW4
            GPIO_setOutputHighOnPin(SW4_CtrPin);
            SW_Mode = SW4;
            break;
        case SW4:
            // all SWs are on, we reach our MAX capacity
            // we do not monitor supply voltage anymore
            Timer_B_stop(TIMER_B0_BASE);
            break;

        default: break;

    }

}



/* TimerB initialization, monitor supply voltage */
void initTimerB_SupplyVoltage(uint8_t count){

    // re-initialize timer
    Timer_B_clearTimerInterrupt(TIMER_B0_BASE);
    Timer_B_initUpModeParam param = {0};
        param.clockSource = TIMER_B_CLOCKSOURCE_ACLK;  // ACLK = VLOCLK = 9.4KHz, in LPM3 VLO_typ=8000
        param.clockSourceDivider = TIMER_B_CLOCKSOURCE_DIVIDER_40;  // Divided by 40, 8000/40 = 200; or 9400/40=235
        // f=200Hz, it supports around 300s at max
        param.timerPeriod =  count *  NumOfOneSecond;  // how frequently you want to check the voltage, do not set it to be a multiple of 10
        param.timerInterruptEnable_TBIE = TIMER_B_TBIE_INTERRUPT_DISABLE;
        param.captureCompareInterruptEnable_CCR0_CCIE =
                TIMER_B_CAPTURECOMPARE_INTERRUPT_ENABLE;
        param.timerClear = TIMER_B_DO_CLEAR;
        param.startTimer = true;
    Timer_B_initUpMode(TIMER_B0_BASE, &param);

}







/* TimerB0 interrupt service, monitor supply voltage */
#pragma vector=TIMER0_B0_VECTOR
__interrupt void TIMERB_ISR_SupplyVoltage(void){



    /* race condition happens w.r.t TIMER_ISR_PeriodicWakingup, directly exit this current ISR function */
    if(SupplyVoltageFlag || PeriodicWakingupFlag){
        return;
    }


    /* when sampling VCC, we stop the audio sensing timer */
    Timer_A_stop(TIMER_BASE_PeriodicWakingup);


    /* to avoid race condition with StateSwitch, see the explanation above when defining StateSwitchFlag
     * we need to immediately clear the StateSwitchFlag
     */
    StateSwitchFlag = 0;

    /* to avoid race condition with audio_sampling, set the SupplyVoltageFlag */
    /* hold the lock */
    SupplyVoltageFlag = 1;


#if DEBUG
    P1OUT |= BIT1;  // turn on Green LED
    __delay_cycles(800000); // 0.2s
    P1OUT &= ~BIT1; // turn off Green LED
#endif

    /*
     * periodically check the VCC voltage level
     * if the VCC is higher than the threshold, turn on a new SW
     */
    /* sample VCC */
    uint16_t result;
    result = runSampling_VCC();  // if result = 0, it means StateSwitch happens during sampling process
    SupplyVoltage = get_voltage();


    if(!StateSwitchFlag && SupplyVoltage > 3166 && result){
        /* the 850 is the turn-on threshold for adding a new capacitor,
         * you have to find the relationship between the real voltage and
         * the sampled voltage value empirically, using Find_RelationBetweenValueAndVCC() to
         * gather data (using a fixed voltage to power up the board and then log the sampled
         * voltage value, tried 1.8V - 3.0V with an increment of 0.05V)
         */


        /* turn on a new capacitor */
        SW_AddNew();  // add one more capacitor
    }



    /* race condition w.r.t StateSwitch happens
     * we need to do what is supposed to be done originally by TIMERA_ISR_StateSwitch
     * exit the LPM entered in void step()
     */
    if(StateSwitchFlag){
        __bic_SR_register_on_exit(LPM3_bits); // do the job which should be done by TIMERA_ISR_StateSwitch
    }


    /* release the lock */
    SupplyVoltageFlag = 0;

    /* restart the audio sensing timer */
    Timer_A_startCounter(TIMER_BASE_PeriodicWakingup,TIMER_A_UP_MODE);
}






























