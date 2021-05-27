/* --COPYRIGHT--,BSD
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
//******************************************************************************
//!  TI-Design Signal Processing with LEA on MSP430FR5994
//!
//!  William Goh, L Reynoso
//!  Texas Instruments Inc.
//!  August 2019
//******************************************************************************

//#include <driverlib.h>
//#include <stdint.h>
////#include <grlib.h>
//#include <stdio.h>
//#include <math.h>
//#include "DSPLib.h"
//#include "FFT.h"
//#include "FFT_430.h"
//#include "benchmark.h"

#include "global.h"



#pragma DATA_SECTION(leaMemoryStartAdd, ".leaRAM")
LeaMemoryStartAdd leaMemoryStartAdd;

#pragma PERSISTENT(FFT_data)
int16_t FFT_data[VECTOR_SIZE] = {0};

#pragma PERSISTENT(FFT_data_copy)
int16_t FFT_data_copy[VECTOR_SIZE] = {0};

#pragma PERSISTENT(FFT_result);
uint16_t FFT_result[100] = {0};



uint16_t LargestFreq[8] = {0};
uint16_t NumLargestFreq = 8;



uint16_t i, index;
int16_t imag, real, real_abs, imag_abs, mag, max, min;
int16_t scale;

msp_cmplx_fft_q15_params complexFftParams;





uint16_t runFftWithLea(uint16_t* data)
{
    msp_status status;
    uint16_t i;



    resetBenchmark();
    complexFftParams.length = VECTOR_SIZE;
    complexFftParams.bitReverse = 1;
    complexFftParams.twiddleTable = msp_cmplx_twiddle_table_256_q15;//msp_cmplx_twiddle_table_256_q15;

    __no_operation();
    // Copy recorded ADC data to LEA RAM buffer for FFT processing
    for(i = 0; i < (VECTOR_SIZE); i++)
    {
        ((int32_t *)leaMemoryStartAdd.fftDataParam.fftInputOutput)[i] =
                data[i] & 0x0000FFFF;
    }

    resetBenchmark();
    startBenchmark();

    // Call fixed scaling fft function.
    status = msp_cmplx_fft_fixed_q15(&complexFftParams,
                                     leaMemoryStartAdd.fftDataParam.fftInputOutput);

    // Check status flag.
    if(status != MSP_SUCCESS)
    {
        // ERROR!
        __no_operation();
    }

    stopBenchmark();

    // Calculate magnitude for the positive frequency domain
    for(i = 0; i < (VECTOR_SIZE / 2); i++)
    {
        real = leaMemoryStartAdd.fftDataParam.fftInputOutput[2 * i];
        imag = leaMemoryStartAdd.fftDataParam.fftInputOutput[2 * i + 1];
        if(real < 0)
        {
            real_abs = ~real + 1;
        }


        else
        {
            real_abs = real;
        }
        if(imag < 0)
        {
            imag_abs = ~imag + 1;
        }
        else
        {
            imag_abs = imag;
        }
        if(real_abs >= imag_abs)
        {
            max = real_abs;
            min = imag_abs;
        }
        else
        {
            max = imag_abs;
            min = real_abs;
        }
        mag = max + 3 * (min >> 3);
        FFT_data[i] = mag;
        FFT_data_copy[i] = mag;
    }
    // FFT result is saved in FFT_data




    uint16_t max,index,freq;
    __no_operation();

    /* find out the index of the largest Numlargestelem numbers */
    for( i = 0; i <= NumLargestFreq; i++){
         index = find_index_of_max();
         LargestFreq[i] = index*((SAMPLING_FREQUENCY/2)/(VECTOR_SIZE/2));
//         printf("%u - ",LargestFreq[i]);



         /* we delete high-frequency noise */
         if(LargestFreq[i] > 2800){
             LargestFreq[i] = 0;
         }
    }

    /* now the most largest NumLargestFreq frequencies are in LargestFreq
     * let's find out the highest frequency */
    max = 0;
    for( i = 0; i <= NumLargestFreq; i++){
        if( max < LargestFreq[i]){
            max = LargestFreq[i];
        }
    }
    freq = max;
//    printf("\n%u\n",freq);


    return  freq;


}




/* *** Pick out the frequency with the highest value *** */
/* after doing FFT, there are some noisy data in the lower frequency
 * which may have higher intensity than the right frequency of the sound source
 * By doing experiment, I found that the right frequency is there
 * the right frequency may not be always the highest one but it is always within the largest N elements*/
uint16_t find_index_of_max( void ){

    int i,index,max=0;

    for(i = 1; i < VECTOR_SIZE/2; i++){
        if(max < FFT_data[i]){
            max = FFT_data[i];
            index = i;
        }
    }

    FFT_data[index] = 0;
    return index;
}


