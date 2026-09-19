#include "fft.h"
#include <math.h>
#include <stdint.h>
arm_rfft_fast_instance_f32 S;

//刚学到的一个调试技巧：max_SignalStrength遍历就可以直观地观察频谱泄露了

float fft_input[NPT];
float fft_output[NPT];//只需要存储前一半的频域的实虚信息，因此是NPT*2/2
float fft_SignalStrength[NPT/2];
static float Hanning_Window[NPT];//NPT点汉宁窗系数，无单位，数值范围约为0~1
//汉宁窗补偿系数
static float Hanning_Repair = 1.0f;
float max_SignalStrength;
uint32_t max_index;
//下面这些是用来求相位的
float real;
float imag;
float phase;
float deg;
//时域数组


static uint8_t fft_init_flag = 0;

//尝试加汉宁窗，FFT初始化时顺便初始化汉宁窗
static void Hanning_Window_Init(void)
{
//易知w[n]=0.5*(1-cos(2*PI*n/NPT))
    arm_hanning_f32(Hanning_Window, NPT);

    //codex:根据实际生成的窗系数计算均方增益，汉宁窗结果约为0.61237
    float window_energy = 0;
    for(int i = 0; i < NPT; i++){
        window_energy += Hanning_Window[i] * Hanning_Window[i];
    }
    Hanning_Repair = sqrtf(window_energy / NPT);
}

void FFT_Init(void)
{
    arm_rfft_fast_init_f32(&S, NPT);
    Hanning_Window_Init();
    fft_init_flag = 1;
}

//处理时域：去直流偏置，上汉宁窗
static void FFT_adcProcessing(volatile uint16_t *adc_value)
{
    float adc_mean = 0;//一帧ADC采样值的平均值，单位为ADC码值

    for(int i = 0; i < NPT; i++){
        adc_mean += (float)adc_value[i];
    }
    adc_mean /= NPT;

    for(int i = 0; i < NPT; i++){
        fft_input[i] = ((float)adc_value[i] - adc_mean) * Hanning_Window[i];
    }
}

//能量收集
static float FFT_Energy_Collection(uint32_t index)
{
    float signal_energy = 0;
    int start_index = (int)index - 1;
    int end_index = (int)index + 1;

    if(start_index < 1) start_index = 1;
    if(end_index >= NPT / 2) end_index = NPT / 2 - 1;

    for(int i = start_index; i <= end_index; i++){
        signal_energy += fft_SignalStrength[i] * fft_SignalStrength[i];
    }

    return sqrtf(signal_energy);
}

void FFT_Calculation(volatile uint16_t *adc_value,float* V,float* F){
		if (fft_init_flag == 0) {
        FFT_Init();
    }

//处理时域数据：去直流，上汉宁窗
    FFT_adcProcessing(adc_value);
//这个是实数DSPFFT，对应的有arm_cfft_radix4_f32复数DSPFFT，要进行实部虚部来回赋值		
    arm_rfft_fast_f32(&S, fft_input, fft_output,0);

    arm_cmplx_mag_f32(fft_output, fft_SignalStrength, NPT/2);
    fft_SignalStrength[0] = 0;

    arm_max_f32(fft_SignalStrength, NPT/2, &max_SignalStrength, &max_index);
		
	real=fft_output[2*max_index];
	imag=fft_output[2*max_index+1];//*2是因为一个数据占用了数组的两位max_index对应的数据就在2倍的位置
		
		
//能量收集
    max_SignalStrength = FFT_Energy_Collection(max_index);

//再用Hanning_Repair把加窗的影响补偿回来
		*V = 4 * max_SignalStrength / (NPT * Hanning_Repair) * (3.3f / 65536.0f);
        *F = max_index*Fs/NPT;
		phase=atan2f(imag,real);//最后算出来的这个角是以余弦为基准的？哪里会有坑吗，注意
		deg=phase*180/PAI;//单开一路测相位是没有意义的，需要对比
}




// void FFT_Calculation(volatile uint16_t *adc_value,float* V,float* F){
// 		if (fft_init_flag == 0) {
//         FFT_Init();
//     }
		
// 		for(int i=0;i<NPT;i++){
// 			fft_input[i] = (float)adc_value[i];
// 		}
// //这个是实数DSPFFT，对应的有arm_cfft_radix4_f32复数DSPFFT，要进行实部虚部来回赋值		
//     arm_rfft_fast_f32(&S, fft_input, fft_output,0);

//     arm_cmplx_mag_f32(fft_output, fft_SignalStrength, NPT/2);
//     fft_SignalStrength[0] = 0;

//     arm_max_f32(fft_SignalStrength, NPT/2, &max_SignalStrength, &max_index);
		
// 		real=fft_output[2*max_index];
// 		imag=fft_output[2*max_index+1];//*2是因为一个数据占用了数组的两位max_index对应的数据就在2倍的位置
		
		
//     max_SignalStrength = 0;
//     for(int i=max_index-1;i<=max_index+1;i++){
//         max_SignalStrength += fft_SignalStrength[i]*fft_SignalStrength[i];
//     }
//     max_SignalStrength = sqrtf(max_SignalStrength);

// 		*V = 4 * max_SignalStrength / NPT * (3.3f / 65536.0f);
//     *F = max_index*Fs/NPT;
// 		phase=atan2f(imag,real);//最后算出来的这个角是以余弦为基准的？哪里会有坑吗，注意
// 		deg=phase*180/PAI;//单开一路测相位是没有意义的，需要对比
// }








//波形判断函数：输入FFT幅值谱和基波下标，返回0为正弦波，返回1为三角波
static int Harmonic_Judging(float *SignalStrength, uint32_t index){
    uint32_t tri_index = 3 * index;
    uint32_t fif_index = 5 * index;
    float fundamental_signalstrength = 0;
    float tri_signalstrength = 0;
    float fif_signalstrength = 0;
    float tri = 0;
    float fif = 0;

    //基波附近能量，和谐波使用相同的频点范围，便于计算幅值比例
    if(index < 3 || index + 3 >= NPT / 2){
        return 0;
    }
    for(int i = (int)index - 3; i <= (int)index + 3; i++){
        fundamental_signalstrength += SignalStrength[i] * SignalStrength[i];
    }
    fundamental_signalstrength = sqrtf(fundamental_signalstrength);
    if(fundamental_signalstrength == 0){
        return 0;
    }

    //三角波的三次谐波满足A3≈A1/9，因此sqrt(A1/A3)应在3附近
    if(tri_index >= 3 && tri_index + 3 < NPT / 2){
        for(int i = (int)tri_index - 3; i <= (int)tri_index + 3; i++){
            tri_signalstrength += SignalStrength[i] * SignalStrength[i];
        }
        tri_signalstrength = sqrtf(tri_signalstrength);
        if(tri_signalstrength != 0){
            tri = sqrtf(fundamental_signalstrength / tri_signalstrength);
            if(tri >= 2.0f && tri <= 4.0f){
                return 1;
            }
        }
    }

    //三角波的五次谐波满足A5≈A1/25，因此sqrt(A1/A5)应在5附近
    if(fif_index >= 3 && fif_index + 3 < NPT / 2){
        for(int i = (int)fif_index - 3; i <= (int)fif_index + 3; i++){
            fif_signalstrength += SignalStrength[i] * SignalStrength[i];
        }
        fif_signalstrength = sqrtf(fif_signalstrength);
        if(fif_signalstrength != 0){
            fif = sqrtf(fundamental_signalstrength / fif_signalstrength);
            if(fif >= 4.0f && fif <= 6.0f){
                return 1;
            }
        }
    }

    return 0;
}

//波形判断函数：输入一组时域ADC采样值，返回0为正弦波，返回1为三角波
int Wave_Judging(volatile uint16_t *adc_value){
    float wave_v = 0;
    float wave_f = 0;

    //先由时域数据完成FFT，再利用基波及三、五次谐波的幅值关系判断波形
    FFT_Calculation(adc_value, &wave_v, &wave_f);
    return Harmonic_Judging(fft_SignalStrength, max_index);
}
