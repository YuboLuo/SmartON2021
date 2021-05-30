/*
 * audio.h
 *
 *  Created on: May 4, 2019
 *      Author: Yubo-ASUS
 */

#ifndef AUDIO_H_
#define AUDIO_H_



#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>


#define MIC_POWER_PORT_DIR  P6DIR
#define MIC_POWER_PORT_OUT  P6OUT
#define MIC_POWER_PIN       BIT2

#define AUDIO_PORT_SEL0     P1SEL0
#define AUDIO_PORT_SEL1     P1SEL1
#define MIC_INPUT_PIN       BIT3

#define MIC_INPUT_CHAN      ADC12INCH_3



/* The Audio object structure, containing the Audio instance information */
typedef struct Audio_configParams
{
    /* Size of both audio buffers */
    uint16_t bufferSize;

    /* audio sample rate in Hz */
    uint16_t sampleRate;

    /* sample type */
    uint8_t type;
    // 1: audio
    // 2: VCC

} Audio_configParams;


void test_Audio(void);
void test_VCC(void);
uint8_t runSampling_VCC(void);
uint8_t runSampling_Audio(void);

uint16_t get_voltage(void); // for SVS
uint16_t get_voltage_StableVersion(void);  // for EL
/*----------------------------------------------------------------------------
 * Functions
 * --------------------------------------------------------------------------*/
void Audio_setupCollect(Audio_configParams * audioConfig);

/* Start collecting audio samples in ping-pong buffers */
void Audio_startCollect(void);

/* Stop collecting audio samples in buffers */
void Audio_stopCollect(void);

/* Shut down the audio collection peripherals */
void Audio_shutdownCollect(void);

#endif /* AUDIO_H_ */
