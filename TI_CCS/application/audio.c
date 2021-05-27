/*
 * audio.c
 *
 *  Created on: May 4, 2019
 *      Author: Yubo-ASUS
 */

//******************************************************************************
//!  main.c
//!
//!  Description: Sound detection demo on MSP-EXP430FR5994
//!               using ADMP401
//!
//!               the MCU samples a 300ms audio signal, and then uses threshold method to detect if an event happens
//!               Red LED1 indicating the MCU is sampling
//!
//!               After the red LED is off, the MCU starts processing data (event detecting)
//!               Green LED2 indicates an event is captured, green LED lasts for 0.25s
//!
//!               NOTE: This demo requires the ADMP401
//!
//!                  MSP430FR5994               Audio ADMP401
//!               -----------------             -----------------
//!              |             P1.3|<--------- | MIC OUT         |
//!              |             P6.2|---------> | MIC PWR         |
//!              |              GND|---------- | MIC GND         |
//!              |-----------------|           |-----------------|
//!
//******************************************************************************

#include "global.h"

#pragma LOCATION(dataRecorded, 0x10000);
#pragma PERSISTENT(dataRecorded);
uint16_t dataRecorded[SAMPLES_LENGTH] = {0};



//uint16_t* DataProcess = &dataRecorded[3000];

#pragma LOCATION(results_VCC, 0x09800);
#pragma PERSISTENT(results_VCC)
uint16_t results_VCC[200] = {0};

#pragma PERSISTENT(results_VCC_num)
uint16_t results_VCC_num = 0;


#pragma LOCATION(results_Audio, 0x09600);
#pragma PERSISTENT(results_Audio)
uint16_t results_Audio[100] = {0};

#pragma PERSISTENT(results_Audio_num)
uint16_t results_Audio_num = 0;

uint8_t recording = 0;

// Globals
DMA_initParam dma0Config;
Audio_configParams gAudioConfig;

/* when sampling the voltage, it is unreliable if only sample once
 * we sample VCC_sample_num times, and only use the last average_num data points,
 * then average them and get the final voltage
 * VCC_sample_num is used in the Audio-sample initialization part when setting up DMA transferSize
*/
uint16_t VCC_sample_num = 15; //


/* this function is for SVS, add_SW()
 * only do one-time sampling
 * runSampling_VCC() must be called before calling this function
 * we use average here, but get_voltage_StableVersion() uses min value
*/
uint16_t get_voltage(void){

    int i;
    int process_num = 5;  // must smaller than VCC_sample_num
    uint16_t sum = 0;


    /* for each sampling, we sample 10 data points and only use the last process_num points */
    // use the average value as the result
    for( i = VCC_sample_num - process_num; i < VCC_sample_num; i++){
        sum += dataRecorded[i];
    }


    return sum/process_num;


}

/* to decide which energy level (EL) the system resides, we need a stable verison of get_voltage
 * we do 10 times of samplings, each samplings samples 10 data points and we only use the last 5 points
 * use the lowest value of the 5 points as the result of each sampling
 * use the lowest value of the 10 sampling results as our final result
*/
uint16_t get_voltage_StableVersion(void){

    int i,j;
    int process_num = 5;  // must smaller than VCC_sample_num
    uint16_t min,volt = 10000;


    /* we sample VCC 10 times and use the min of all samplings*/
    for(i = 0; i < 10; i++){
        runSampling_VCC();

        /* for each sampling, we sample 10 data points and only use the last process_num points */
        // the min of these process_num points is the result of one sampling
        min = 10000;
        for( j = VCC_sample_num - process_num; j < VCC_sample_num; j++){
            if(dataRecorded[j] < min)
                min = dataRecorded[j];
        }

        if( min < volt)
            volt = min;  // volt saves the min of all samplings
    }

    return volt;

}



void test_VCC(void){
//
    int i;
    uint16_t range;
    for( i = 0; i<10; i++){
        runSampling_VCC();
        range = get_voltage();
        results_VCC[results_VCC_num++] = range;


        // delay for one second before start the next sensing test
        __delay_cycles(4000000);

        continue;
    }

}



void test_Audio(void){


    int i;
    uint16_t diff;
    for( i = 0; i<10; i++){

        runSampling_Audio();
        diff = get_threshold();  // get the diff value which will be compared to the threshold


        if(diff > 200){  // if high, it means an event happened
            results_Audio[results_Audio_num++] = 0xaaaa;
            P1OUT |= BIT1;   // turn on the LED
            __delay_cycles(1600000);  // light up the green LED for 0.2s
            P1OUT &= ~BIT1;  // turn off the LED
        }
        else{
            results_Audio[results_Audio_num++] = 0x1111;  // no events happened
        }


    }
}




uint8_t runSampling_Audio(void){


#if DEBUG
    /* turn on RED LED when starting sampling, it should be turned off in DMA Interrupt */
    P1OUT |= BIT0;
#endif

    /* add an extra load for event-sensing */
    GPIO_setOutputHighOnPin(LEDLoad);


//    /* turn on RED LED when starting sampling, it should be turned off in DMA Interrupt */
//    if(DEBUG == 0 && DebugOntheFly == 1)
//        P1OUT |= BIT0;


    /* Initialize the microphone for recording */
    gAudioConfig.bufferSize = SAMPLES_LENGTH;
    gAudioConfig.sampleRate = SAMPLING_FREQUENCY;
    gAudioConfig.type = 1;

    /* setup audio-sampling configuration */
    Audio_setupCollect(&gAudioConfig);

    /* Start the recording by enabling the timer */
    Audio_startCollect();

//    /* StateSwitch may happen right before we enter LPM3, e.g. here */
//    if(StateSwitchFlag == 11){
//        Audio_shutdownCollect();
//        GPIO_setOutputLowOnPin(LEDLoad);  // turn off extra load, originally should by done by DMA ISR
//        return 0;
//    }


//    /* before we start sampling, we read the counter value of StateSwitch timer, if it is close to the next interrupt, we stop the timer during the sampling process */
//    uint16_t counter_value,flag;
//    flag = 0;
//    counter_value = Timer_A_getCounterValue(TIMER_BASE_StateSwitch);
//    if(counter_value > (StateDuration - 1 ) * NumOfOneSecond){
//        // if only less than one second left
//        Timer_A_stop(TIMER_BASE_StateSwitch);
//        flag = 1;  // set the flag
//    }



    __bis_SR_register(LPM3_bits + GIE);

//    if(flag == 1){ // if previously StateSwitch timer is halted, we restart it here
//        Timer_A_startCounter(TIMER_BASE_StateSwitch,TIMER_A_UP_MODE);
//    }

    /* if StateSwitch happens during audio sensing
     * audio sensing may be stopped before SAMPLING_FREQUENCY audio signal is collected
     * it happens during state switch and it is rare
     * so here we have to do Audio_shutdownCollect() which is supposed to be done originally by dmaIsrHandler
     */
    if(StateSwitchFlag == 11){
        Audio_shutdownCollect();
        GPIO_setOutputLowOnPin(LEDLoad);  // turn off extra load, originally should by done by DMA ISR
        return 0;
    }

    return 1;
}


/* start to sampling VCC */
uint8_t runSampling_VCC(void)
{
    /* use the same Audio sample example code, start the timer and start to sample the VCC */
    /* the data is saved in dataRecorded which will be further processed */

//    uint16_t recording = 0;

//    P1OUT |= BIT0; // turn on RED LED, it will be turned off in DMA Interrupt

    /* Initialize the timer/ADC/IMA for voltage logging */
    gAudioConfig.bufferSize = VCC_sample_num;
    gAudioConfig.sampleRate = SAMPLING_FREQUENCY;
    gAudioConfig.type = 2;        // type=2 voltage logging; type=1 audio sampling;
    Audio_setupCollect(&gAudioConfig);

    /* Start the recording by enabling the timer */
    Audio_startCollect();

    __no_operation();

//    /* StateSwitch may happen right before we enter LPM3, e.g. here */
//    if(StateSwitchFlag == 11){
//        Audio_shutdownCollect();
//        return 0;
//    }






    __bis_SR_register(LPM3_bits + GIE);

    /* if StateSwitch happens during VCC sampling
     * VCC sampling sensing may be stopped before enough data is collected
     * it happens during state switch and it is rare
     * so here we have to do Audio_shutdownCollect() which is supposed to be done originally by dmaIsrHandler
     */
    if(StateSwitchFlag == 11){
        Audio_shutdownCollect();
        return 0;
    }

    return 1;

}





//******************************************************************************
// Functions
//******************************************************************************
/* Function that powers up the external microphone and starts sampling
 * the microphone output.
 * The ADC is triggered to sample using the Timer module
 * Then the data is moved via DMA. The device would only wake-up once
 * the DMA is done. */
void Audio_setupCollect(Audio_configParams * audioConfig)
{
    Timer_A_initUpModeParam upConfig = {0};
        upConfig.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
        upConfig.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
        upConfig.timerPeriod = (__SYSTEM_FREQUENCY_MHZ__ / audioConfig->sampleRate) - 1;
        upConfig.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
        upConfig.captureCompareInterruptEnable_CCR0_CCIE = TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE;
        upConfig.timerClear = TIMER_A_DO_CLEAR;
        upConfig.startTimer = false;


    Timer_A_initCompareModeParam compareConfig = {0};
        compareConfig.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
        compareConfig.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE;
        compareConfig.compareOutputMode = TIMER_A_OUTPUTMODE_TOGGLE_RESET;
        compareConfig.compareValue = ((__SYSTEM_FREQUENCY_MHZ__ / audioConfig->sampleRate) / 2) - 1;


    // Initialize Timer_A channel 1 to be used as ADC12 trigger
    // Initialize TACCR0 (period register) __SYSTEM_FREQUENCY_MHZ__/sampleRate = NUM
    // Simple counter with no interrupt. 0...NUM = NUM counts/sample
    Timer_A_initUpMode(TIMER_A0_BASE, &upConfig);

    // Initialize TA0CCR1 to generate trigger clock output, reset/set mode
    Timer_A_initCompareMode(TIMER_A0_BASE, &compareConfig);


    // Turn on mic power full drive strength and enable mic input pin to ADC
    MIC_POWER_PORT_OUT |= MIC_POWER_PIN;
    MIC_POWER_PORT_DIR |= MIC_POWER_PIN;


    AUDIO_PORT_SEL0 |= MIC_INPUT_PIN;   // Enable A/D channel inputs
    AUDIO_PORT_SEL1 |= MIC_INPUT_PIN;

    // For safety, protect RMW Cpu instructions
    DMA_disableTransferDuringReadModifyWrite();

    // Initialize the DMA. Using DMA channel 1.
    dma0Config.channelSelect = DMA_CHANNEL_1;
    dma0Config.transferModeSelect = DMA_TRANSFER_SINGLE;

    dma0Config.transferSize = audioConfig->bufferSize;   // how many times of sampling

    dma0Config.triggerSourceSelect = DMA_TRIGGERSOURCE_26;
    dma0Config.transferUnitSelect = DMA_SIZE_SRCWORD_DSTWORD;
    dma0Config.triggerTypeSelect = DMA_TRIGGER_RISINGEDGE;


    DMA_init(&dma0Config);

    DMA_setSrcAddress(DMA_CHANNEL_1,
                      (uint32_t) &ADC12MEM0,
                      DMA_DIRECTION_UNCHANGED);

    DMA_setDstAddress(DMA_CHANNEL_1,
                      (uint32_t) (&dataRecorded),
                      DMA_DIRECTION_INCREMENT);


    // Configure ADC
    ADC12CTL0 &= ~ADC12ENC;           // Disable conversions to configure ADC12
    // Turn on ADC, sample 32 clock cycles =~ 2us
    ADC12CTL0 = ADC12ON + ADC12SHT0_3;

    // Use sample timer, rpt single chan 0, use MODOSC, TA0 timer channel 1
    ADC12CTL1 = ADC12SHP + ADC12CONSEQ_2 + ADC12SHS_1;

    // set input to ADC, (AVCC/AVSS ref), sequence end bit set
    if(audioConfig->type == 1){
        // if type == 1, sample from audio
        ADC12MCTL0 = MIC_INPUT_CHAN | ADC12VRSEL_0 | ADC12EOS; // Channel 31 for Vcc, last conversion in sequence  | ADC12VRSEL_1
    }
    else if(audioConfig->type == 2){
        // if type == 2, sample from VCC
        ADC12MCTL0 = ADC12_B_INPUT_BATMAP | ADC12VRSEL_1 | ADC12EOS;

        // Configure internal reference
        while(Ref_A_isRefGenBusy(REF_A_BASE));              // If ref generator busy, WAIT
        Ref_A_setReferenceVoltage(REF_A_BASE, REF_A_VREF2_0V);
        Ref_A_enableReferenceVoltage(REF_A_BASE);
    }


    // Enable ADC to convert when a TA0 edge is generated
    ADC12CTL0 |= ADC12ENC;

    if(audioConfig->type == 1){
        // if type == 1, sample from audio
        // Delay for the microphone to settle
        // ASMP401: the small red audio IC needs 0.2s initialization time before its stable sensing
        // later we found out if powered up by capacitor, ASMP401 needs longer initialization period
        // threshold-based sound-event detection will fail if we start sampling before the audio is stable
        __delay_cycles(4000000); // 0.5s  // we tried 0.2s but it does not work when capacitor is the power supply
    }

}

/*--------------------------------------------------------------------------*/
/* Start collecting audio samples in ping-pong buffers */
void Audio_startCollect(void)
{
    // Enable DMA channel 1 interrupt
    DMA_enableInterrupt(DMA_CHANNEL_1);

    // Enable the DMA0 to start receiving triggers when ADC sample available
    DMA_enableTransfers(DMA_CHANNEL_1);

    // Start TA0 timer to begin audio data collection
    Timer_A_clear(TIMER_A0_BASE);
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
}

/*--------------------------------------------------------------------------*/
/* Stop collecting audio samples in buffers */
void Audio_stopCollect(void)
{
    Timer_A_stop(TIMER_A0_BASE);

    ADC12_B_disableConversions(ADC12_B_BASE, ADC12_B_COMPLETECONVERSION);

    // Disable DMA channel and interrupt
    DMA_disableTransfers(DMA_CHANNEL_1);
    DMA_disableInterrupt(DMA_CHANNEL_1);

}

/*--------------------------------------------------------------------------*/
/* Shut down the audio collection peripherals*/
void Audio_shutdownCollect(void)
{
    /*
     * DMA finishes
     */
    // Disable the dma transfer
    DMA_disableTransfers(DMA_CHANNEL_1);
    // Disable DMA channel 1 interrupt
    DMA_disableInterrupt(DMA_CHANNEL_1);
//    P1OUT &= ~BIT0;  // turn off LED

    /*
     * audio stop collect
     */
    Timer_A_stop(TIMER_A0_BASE);
    ADC12_B_disableConversions(ADC12_B_BASE, ADC12_B_COMPLETECONVERSION);
    // Disable DMA channel and interrupt
    DMA_disableTransfers(DMA_CHANNEL_1);
    DMA_disableInterrupt(DMA_CHANNEL_1);


    /*
     * audio shut down
     */
    // Turn off preamp power
    MIC_POWER_PORT_OUT &= ~MIC_POWER_PIN;
    // Disable the ADC
    ADC12_B_disable(ADC12_B_BASE);
}


//******************************************************************************
// DMA interrupt service routine
// every time when the ADC is ready, ADC will trigger the DMA which starts
// transferring data from ADC to memory
//******************************************************************************
#pragma vector=DMA_VECTOR
__interrupt void dmaIsrHandler(void)
{
    switch (__even_in_range(DMAIV, DMAIV_DMA2IFG))
    {
        case DMAIV_DMA0IFG: break;
        case DMAIV_DMA1IFG:


#if DEBUG
            /* turn off RED LED which means that the audio sampling is finished now */
            P1OUT &= ~BIT0;  // turn off LED
#endif

//            if(DEBUG == 0 && DebugOntheFly == 1)
//                /* turn off RED LED which means that the audio sampling is finished now */
//                P1OUT &= ~BIT0;  // turn off LED


            /* we add an extra load for event-sensing, turn off here when sensing is finished*/
            GPIO_setOutputLowOnPin(LEDLoad);


            /* shut down audio collect module */
            Audio_shutdownCollect();

            // Start Cpu on exit
            __bic_SR_register_on_exit(LPM3_bits);
            break;
        default: break;
   }
}



/* ADC12 Interrupt Service Routine*/
#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
  switch(__even_in_range(ADC12IV,76))
  {
    case ADC12IV_NONE: break;                // Vector  0:  No interrupt
    case ADC12IV_ADC12OVIFG: break;          // Vector  2:  ADC12MEMx Overflow
    case ADC12IV_ADC12TOVIFG: break;         // Vector  4:  Conversion time overflow
    case ADC12IV_ADC12HIIFG: break;          // Vector  6:  ADC12HI
    case ADC12IV_ADC12LOIFG: break;          // Vector  8:  ADC12LO
    case ADC12IV_ADC12INIFG: break;           // Vector 10:  ADC12IN
    case ADC12IV_ADC12IFG0:  break;
    case ADC12IV_ADC12IFG1:                   // Vector 14:  ADC12MEM1
    case ADC12IV_ADC12IFG2: break;            // Vector 16:  ADC12MEM2
    case ADC12IV_ADC12IFG3: break;            // Vector 18:  ADC12MEM3
    case ADC12IV_ADC12IFG4: break;            // Vector 20:  ADC12MEM4
    case ADC12IV_ADC12IFG5: break;            // Vector 22:  ADC12MEM5
    case ADC12IV_ADC12IFG6: break;            // Vector 24:  ADC12MEM6
    case ADC12IV_ADC12IFG7: break;            // Vector 26:  ADC12MEM7
    case ADC12IV_ADC12IFG8: break;            // Vector 28:  ADC12MEM8
    case ADC12IV_ADC12IFG9: break;            // Vector 30:  ADC12MEM9
    case ADC12IV_ADC12IFG10: break;           // Vector 32:  ADC12MEM10
    case ADC12IV_ADC12IFG11: break;           // Vector 34:  ADC12MEM11
    case ADC12IV_ADC12IFG12: break;           // Vector 36:  ADC12MEM12
    case ADC12IV_ADC12IFG13: break;           // Vector 38:  ADC12MEM13
    case ADC12IV_ADC12IFG14: break;           // Vector 40:  ADC12MEM14
    case ADC12IV_ADC12IFG15: break;           // Vector 42:  ADC12MEM15
    case ADC12IV_ADC12IFG16: break;           // Vector 44:  ADC12MEM16
    case ADC12IV_ADC12IFG17: break;           // Vector 46:  ADC12MEM17
    case ADC12IV_ADC12IFG18: break;           // Vector 48:  ADC12MEM18
    case ADC12IV_ADC12IFG19: break;           // Vector 50:  ADC12MEM19
    case ADC12IV_ADC12IFG20: break;           // Vector 52:  ADC12MEM20
    case ADC12IV_ADC12IFG21: break;           // Vector 54:  ADC12MEM21
    case ADC12IV_ADC12IFG22: break;           // Vector 56:  ADC12MEM22
    case ADC12IV_ADC12IFG23: break;           // Vector 58:  ADC12MEM23
    case ADC12IV_ADC12IFG24: break;           // Vector 60:  ADC12MEM24
    case ADC12IV_ADC12IFG25: break;           // Vector 62:  ADC12MEM25
    case ADC12IV_ADC12IFG26: break;           // Vector 64:  ADC12MEM26
    case ADC12IV_ADC12IFG27: break;           // Vector 66:  ADC12MEM27
    case ADC12IV_ADC12IFG28: break;           // Vector 68:  ADC12MEM28
    case ADC12IV_ADC12IFG29: break;           // Vector 70:  ADC12MEM29
    case ADC12IV_ADC12IFG30: break;           // Vector 72:  ADC12MEM30
    case ADC12IV_ADC12IFG31: break;           // Vector 74:  ADC12MEM31
    case ADC12IV_ADC12RDYIFG: break;          // Vector 76:  ADC12RDY
    default: break;
  }
}


