#ifndef _FFT_H_
#define _FFT_H_

#include <stdint.h>
#include "arm_math.h"

#define Fs 1000000.0f
#define NPT 4096
#define PAI 3.1415926f

extern arm_rfft_fast_instance_f32 S;

extern float fft_input[NPT];
extern float fft_output[NPT];
extern float fft_SignalStrength[NPT/2];
extern float max_SignalStrength;
extern uint32_t max_index;


void FFT_Init(void);
void FFT_Calculation(volatile uint16_t *adc_value,float* V,float* F);
int Wave_Judging(volatile uint16_t *adc_value);

/*
AD1_3 >> A6 << AD2_3
AD1_4 >> C4 << AD2_4? re
AD1_5 >> B1 << AD2_5? re

AD3_1 >> C3_C(C3) 最好 ，St↓充满C，也就意味着Fs↑
AD3_2 >> F9
AD3_3 >> F7

123/213/033喵

*/

#endif /* _FFT_H_ */
