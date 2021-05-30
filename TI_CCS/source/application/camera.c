/*
 * camera.c
 *
 *  Created on: Sep 4, 2020
 *      Author: yubo
 */



//   Description: use I2C to communicate with the camera and setup its register
//   MSP430 outputs a clock signal to the camera as its clock source
//   DMA is triggered by the camera's PCLK to read image in the 4-bit mode
//
//   **NOTE** MSP430 DMAE0 seems to only support an external clock signal up to 1MHz
//   Higher than 1MHz, the DMA will not be able to read a full/complete frame
//   for 4bit mode, the camera's PCLK is 2xSensorCore frequency
//   Because we only use the camera to detect a LED blink, we do not need a full/complete frame
//   we use a threshold-based method to detect the event so we use DMAE0=2MHz
//   and we only sample 10000 data points from the data bus which are enough for LED blinking detection
//   one detection cycle takes 454ms
//
//
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
//!
//******************************************************************************

/*
 * we developed our camera module by referring both BackCam paper and TI official example code
 * BackCam: https://github.com/cjosephson/backcam/blob/master/camera-mcu/main.c
 * TI example code:
 *
 *
 *
 * */


#include "global.h"

int i,j;

#pragma PERSISTENT(img_buffer)
uint8_t img_buffer[framesize] = {0};

#pragma PERSISTENT(ResCameraDet)
uint16_t ResCameraDet[1000] = {0};

#pragma PERSISTENT(ResCameraDet_cnt)
uint16_t ResCameraDet_cnt = 0;


uint8_t image_ready = 0;
uint8_t camera_finish_flag = 0;  // image processing finished flag

void i2c_init_camera()
{


    /* configure eUSCI_B1 I2C */
    /* reset I2C interface */
    EUSCI_B_I2C_disable(EUSCI_B1_BASE);
    /* prepare configuration */
    EUSCI_B_I2C_initMasterParam master_config = {EUSCI_B_I2C_CLOCKSOURCE_SMCLK,       // source clock from SMCLK
                                                 CS_getSMCLK(),                       // get clock rate of SMCLK
                                                 EUSCI_B_I2C_SET_DATA_RATE_100KBPS,   // data rate 100kbps
                                                 0,                                   // tx byte counter, not used here
                                                 EUSCI_B_I2C_NO_AUTO_STOP};           // don't auto send stop
    /* write configuration */
    EUSCI_B_I2C_initMaster(EUSCI_B1_BASE, &master_config);
    /* camera address 0x24, read and write bit will be added by the hardware */
    EUSCI_B_I2C_setSlaveAddress(EUSCI_B1_BASE, HM01B0ADDR);
    return;
}


/* Read data from camera's register */
uint8_t read_camera(uint16_t regaddr)
{

    /* wait until bus is free */
    while (EUSCI_B_I2C_isBusBusy(EUSCI_B1_BASE)){;}
    /* write address to slave, switch to tx mode */
    EUSCI_B_I2C_disable(EUSCI_B1_BASE);
    EUSCI_B_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
    EUSCI_B_I2C_enable(EUSCI_B1_BASE);
    /* write address, higher byte first */
    EUSCI_B_I2C_masterSendMultiByteStart(EUSCI_B1_BASE, (uint8_t)((regaddr & 0xFF00) >> 8));
    EUSCI_B_I2C_masterSendMultiByteFinish(EUSCI_B1_BASE, (uint8_t)(regaddr & 0x00FF));
    /* wait until bus is free */
    while (EUSCI_B_I2C_isBusBusy(EUSCI_B1_BASE)){;}
    /* request data from slave, switch to rx mode */
    EUSCI_B_I2C_disable(EUSCI_B1_BASE);
    EUSCI_B_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_MODE);
    EUSCI_B_I2C_enable(EUSCI_B1_BASE);
    uint8_t result = EUSCI_B_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
    return result;

}



/* Write data to camera's register */
void write_camera(uint16_t regaddr, uint8_t data)
{
    /* wait until bus is free */
    while (EUSCI_B_I2C_isBusBusy(EUSCI_B1_BASE)){;}
    /* write address to slave, switch to tx mode */
    EUSCI_B_I2C_disable(EUSCI_B1_BASE);
    EUSCI_B_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
    EUSCI_B_I2C_enable(EUSCI_B1_BASE);
    /* write address, higher byte first */
    EUSCI_B_I2C_masterSendMultiByteStart(EUSCI_B1_BASE, (uint8_t)((regaddr & 0xFF00) >> 8));
    EUSCI_B_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, (uint8_t)(regaddr & 0x00FF));
    EUSCI_B_I2C_masterSendMultiByteFinish(EUSCI_B1_BASE, data);
    return;
}


/* write data to the register and read immediately to make sure the data is correctly written */
void write_verify_camera(uint16_t regaddr, uint8_t data)
{

    do {
        write_camera(regaddr, data);
    } while (read_camera(regaddr) != data);

    return;
}



void config_camera()
{
    /* write camera i2c regs */
    write_verify_camera(0x0383, 0b00000011);   // horizontal binning timing
    write_verify_camera(0x0387, 0b00000011);   // vertical binning timing
    write_verify_camera(0x0390, 0b00000011);   // enable v and h binning

    __delay_cycles(40000);
    write_camera(0x0104, 0b00000001);   // update CMU regs
}


void config_dma()
{
    DMA_disableInterrupt(DMA_CHANNEL_0);
    /* init channel 0 */
    DMA_initParam param0 = {DMA_CHANNEL_0,              // use channel 0
                            DMA_TRANSFER_SINGLE,         // single trigger mode
                            framesize, // transfer size, equals image size
                            DMA_TRIGGERSOURCE_31,        // trigger src 31 (DMAE0, P1.0 as input to accept external trigger signal)
                            DMA_SIZE_SRCBYTE_DSTBYTE,    // byte to byte transfer
                            DMA_TRIGGER_RISINGEDGE       // trigger at rising edge
    };
    DMA_init(&param0);

}

void config_TimerA0(void){

    // re-initialize timer
    Timer_A_initUpModeParam param = {0};
        param.clockSource = TIMER_A_CLOCKSOURCE_ACLK;  // ACLK = VLOCLK = 9.4KHz, in LPM3 VLO_typ=8000
        param.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_40;  // Divided by 40
        param.timerPeriod = 3 * NumOfOneSecond;  // count up to this number (period), request time every second
        param.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
        param.captureCompareInterruptEnable_CCR0_CCIE =
                TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
        param.timerClear = TIMER_A_DO_CLEAR;
        param.startTimer = true;
    Timer_A_initUpMode(TIMER_A0_BASE, &param);

}

void reset_camera(void){


    /* use a timer to stop image processing */
    camera_finish_flag = 1;  //0;
//    config_TimerA0();  // start the timer


    i2c_init_camera();
    write_camera(0x0103, 0xff);         // reset camera
    __delay_cycles(800000);             // wait 0.1s for the camera to reset

    /* databus controls */
    write_camera(0x0100, 0b00000001);   // set it to standby mode;
    write_camera(0x3059, 0b01000010);   // set data bus width = 4bit, [6:5]=10 -> 4bit; [4:0] reserved, default 0x02

    /* resolution and timing */
    write_camera(0x3010, 0b00000001);   // enable qvga timing
    config_camera();              // initialize as 164x122

    write_camera(0x3060, 0b00111000);   // gated pclock, MSB first, main clock 8x div, reg clock 1x div;
    // gated pclk means the pclk will be automatically set to low when the LVLD is low.
    // Only when the valid data is being transmitted, the pclk outputs a normal clock signal
    // 200ms per frame

    __delay_cycles(400000);

}



uint8_t capture_image(void){

    uint16_t i = 0;
    uint32_t sum;
    uint8_t temp = 0;

    config_dma();

//    memset(img_buffer,0,framesize);

//    GPIO_setOutputHighOnPin(CMAERA_POWER);  // turn on camera's power
    __delay_cycles(8000);

    reset_camera(); // set up camera's registers


    GPIO_clearInterrupt(CAMERA_FVLD);    // clear irq flag
    GPIO_enableInterrupt(CAMERA_FVLD);   // enable interrupt
    image_ready = 0;


    while(1){
        if(image_ready){
            __no_operation();


            /* if StateSwitch happens during image taking, we will discard this image
             */
            if(StateSwitchFlag == 11){

                return 0;
            }

            /* event detection */
            sum = 0;
            for(i = 0; i<framesize/2; i++){
                temp = (img_buffer[i*2] & 0b00011110); // we only need the high-4-bit of each pixel
                sum += temp;
            }

            if (sum > 100000 ){  // our threshold is 50000, get the threshold empirically
                /* an event is detected */
                results_VCC[results_VCC_num*2+1] = 9;
                return 9;
            }
            else{
                /* no event detected */
                results_VCC[results_VCC_num*2+1] = 1;
                return 1;
            }


//            while(camera_finish_flag == 0); // using a Timer to make image-taking execute at a fixed amount of time

        }
    }



}




void image_process(void){

    /* event detection */
    uint16_t i = 0;
    uint32_t sum;
    uint8_t temp = 0;

    sum = 0;
    for(i = 0; i<framesize/2; i++){
        temp = (img_buffer[i*2] & 0b00011110); // we only need the high-4-bit of each pixel
        sum += temp;
    }

    if (sum > 100000 ){  // our threshold is 50000
        /* an event is detected */
        results_VCC[results_VCC_num*2+1] = 9;
        return 9;
    }
    else{
        /* no event detected */
        results_VCC[results_VCC_num*2+1] = 1;
        return 1;
    }
}


/* ********** ATTENTION **********
 * this function should be commented when doing audio-based experiments
 * because audio sampling implicitly uses Timer A0 and this ISR will cause issue
*/

#pragma vector=TIMER0_A0_VECTOR
__interrupt
void TIMER_ISR_A0(void){

    Timer_A_stop(TIMER_A0_BASE);
    camera_finish_flag = 1;

}



#pragma vector=PORT5_VECTOR
__interrupt
void port5IsrHandler(void)
{
    switch(__even_in_range(P5IV, P5IV_P5IFG7))
    {

        case P5IV_NONE: break;
        case P5IV_P5IFG0: break;
        case P5IV_P5IFG1: break;
        case P5IV_P5IFG2: break;


        case P5IV_P5IFG3:

            GPIO_disableInterrupt(CAMERA_FVLD);    // disable interrupt

            DMA_setSrcAddress(DMA_CHANNEL_0,
                              (uint32_t)(&P4IN),                // data source PORT3
                              DMA_DIRECTION_UNCHANGED);         // src addr doesn't change
            DMA_setDstAddress(DMA_CHANNEL_0,
                              (uint32_t)(img_buffer),           // data dest img_buffer
                              DMA_DIRECTION_INCREMENT);         // dest addr self increases
            DMA_enableTransfers(DMA_CHANNEL_0);

            DMA_enableInterrupt(DMA_CHANNEL_0);
            // enable the DMA, start transferring data,
            // once done, image_ready will be set to 1 in dmaIsrHandler

            // DMA dmaIsrHandler() is in audio.c, because audio/camera both need dma and there can only be one DMA dmaIsrHandler
            // so we put it in the audio.c file

        //    /* for debug, you can use the Discover logic to monitor when the DMA is enabled */
        //    read_camera(0x0100);

            break;

        case P5IV_P5IFG4: break;
        case P5IV_P5IFG5:
            break;
        case P5IV_P5IFG6:
            break;
        case P5IV_P5IFG7:


            break;
        default: break;


    }
}





