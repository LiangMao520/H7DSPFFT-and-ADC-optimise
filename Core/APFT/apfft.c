#include "main.h"
#include "usart.h"
#include "apfft.h"

#include <math.h>
#include "arm_math.h"
#include "arm_const_structs.h"
//全相位fft
arm_cfft_radix4_instance_f32 scfft;

float Hanning_Input[APFT_NPT]={0};
float Hanning_Output[APFT_NPT*2-1]={0};
//加窗数据
float wave_1_Conv[APFT_NPT*2-1]={0};
float wave_2_Conv[APFT_NPT*2-1]={0};

float Apfft_Inputbuf_1[APFT_NPT*2]={0};
float Apfft_Inputbuf_2[APFT_NPT*2]={0};
float Apfft_Outputbuf_1[APFT_NPT]={0};
float Apfft_Outputbuf_2[APFT_NPT]={0};

float Phase=0;

uint32_t wave_packed[APFT_NPT*2]={0};

//三峰谱线插值修正
WaveData Wave_1;
WaveData Wave_2;//两个结构体

arm_rfft_fast_instance_f32 Dsifft_Inst1;
arm_rfft_fast_instance_f32 Dsifft_Inst2;

float Blackman_Input[APFT_NPT*2]={0};
//实数数据存储
float Dsifft_Inputbuf_1[APFT_NPT*2]={0};
float Dsifft_Inputbuf_2[APFT_NPT*2]={0};
//实数fft运算后存储
float Dsifft_Outputbuf_1[APFT_NPT*2]={0};
float Dsifft_Outputbuf_2[APFT_NPT*2]={0};
//模值数据存储
float Dsifft_Mag1[APFT_NPT]={0};
float Dsifft_Mag2[APFT_NPT]={0};

//新版CMSIS-DSP已经提供下面两个窗函数，保留原实现作为参考但不再参与编译
#if 0
//生成APFT_NPT点汉宁窗
void arm_hanning_f32(float32_t *pDst, uint32_t blockSize)
{
	float32_t k = 2.0f / ((float32_t) blockSize);
	float32_t w;
	
	for(uint32_t i=0; i<blockSize; i++)
	{
		w = PI * i * k;
	  w = 0.5f * (1.0f - cosf (w));
		
		pDst[i] = w;
	}
}

//生成APFT_NPT点布莱克曼窗
void arm_blackman_harris_92db_f32(float32_t * pDst, uint32_t blockSize)
{
   float32_t k = 2.0f / ((float32_t) blockSize);
   float32_t w;

   for(uint32_t i=0;i<blockSize;i++)
   {
		 w = PI * i * k;
     w = 0.35875f - 0.48829f * cosf (w) +
     0.14128f * cosf (2.f * w) - 0.01168f * cosf (3.f * w);
        
     pDst[i] = w;
   }
}
#endif

//窗函数初始化
void Windows_Init(void)
{
	//生成APFT_NPT*2-1点二阶汉宁窗
	double mean=0;
	//生成一个APFT_NPT点汉宁窗
	arm_hanning_f32(Hanning_Input, APFT_NPT);
	//对汉宁窗卷积
	arm_conv_f32(Hanning_Input, APFT_NPT, Hanning_Input, APFT_NPT, Hanning_Output);
	//归一化处理
	for(uint16_t i=0; i<APFT_NPT*2-1; i++)
	{
		mean += Hanning_Output[i];
	}
	for(uint16_t i=0; i<APFT_NPT*2-1; i++)
	{
		Hanning_Output[i] /= mean;
	}
	
//	mean=0;
//	//生成APFT_NPT*2点布莱克曼窗
//	arm_blackman_harris_92db_f32(Blackman_Input, APFT_NPT*2-1);
//	for(uint16_t i=0; i<APFT_NPT*2; i++)
//	{
//		mean += Blackman_Input[i];
//	}
//	for(uint16_t i=0; i<=APFT_NPT*2-1; i++)
//	{
//		Blackman_Input[i] /= mean;
//	}
}

//相位计算函数
float Phase_atan(float32_t *inputSignal, uint32_t index)
{
	if(inputSignal[2*index+1]>=0 && inputSignal[2*index]>=0)
		return 0 + atan(inputSignal[2*index+1] / inputSignal[2*index]) / PI * 180;
	else if(inputSignal[2*index+1]>=0 && inputSignal[2*index]<=0)
		return 180 + atan(inputSignal[2*index+1] / inputSignal[2*index]) / PI * 180;
	else if(inputSignal[2*index+1]<=0 && inputSignal[2*index]<=0)
		return 180 + atan(inputSignal[2*index+1] / inputSignal[2*index]) / PI * 180;
	else if(inputSignal[2*index+1]<=0 && inputSignal[2*index]>=0)
		return 360 + atan(inputSignal[2*index+1] / inputSignal[2*index]) / PI * 180;
	else
		return 0;
}

//fft初始化
void APFT_Init(void)
{
	//基4fft初始化
  arm_cfft_radix4_init_f32(&scfft, APFT_NPT, 0, 1);
	//实数fft初始化
	arm_rfft_fast_init_f32(&Dsifft_Inst1, APFT_NPT*2);
  arm_rfft_fast_init_f32(&Dsifft_Inst2, APFT_NPT*2);
}

//冒泡排序
void BubbleSort(uint16_t arr[], uint16_t n)
{
  for (uint16_t i=0; i<n-1; i++)
	{       
		// 外层循环控制排序轮数
    for (uint16_t j=0; j<n-i-1; j++)
		{ 
			// 内层循环比较相邻元素
			if (arr[j] > arr[j+1])
			{      
				// 若前大后小则交换
        uint16_t temp = arr[j];
        arr[j] = arr[j+1];
        arr[j+1] = temp;
      }
    }
  }
}

//全相位fft
void APFT(void)
{
	uint32_t Amax_pos_1,Amax_pos_2;
	float Amax_1,Amax_2;
  //加窗
  for(int i=0; i<2*APFT_NPT-1; i++)
  {
    wave_1_Conv[i] = (uint16_t)(wave_packed[i] & 0xFFFFU) * Hanning_Output[i];
    wave_2_Conv[i] = (uint16_t)(wave_packed[i] >> 16) * Hanning_Output[i];
  }
	//重叠前半部分（反正可以消除测量相位的误差，数学原理没学应该也看不懂
  for(int i=0; i<APFT_NPT-1; i++)
  {
    wave_1_Conv[i] = wave_1_Conv[i] + wave_1_Conv[APFT_NPT+i];
    wave_2_Conv[i] = wave_2_Conv[i] + wave_2_Conv[APFT_NPT+i];
  }
	//基4fft运算
  for(int i=0; i<APFT_NPT; i++)
  {
	Apfft_Inputbuf_1[i*2] = wave_1_Conv[i];
    Apfft_Inputbuf_1[i*2+1] = 0;
    Apfft_Inputbuf_2[i*2] = wave_2_Conv[i];
    Apfft_Inputbuf_2[i*2+1] = 0;
  }
	
  arm_cfft_radix4_f32(&scfft, Apfft_Inputbuf_1);
  arm_cfft_radix4_f32(&scfft, Apfft_Inputbuf_2);
 
  arm_cmplx_mag_f32(Apfft_Inputbuf_1, Apfft_Outputbuf_1, APFT_NPT);
  arm_cmplx_mag_f32(Apfft_Inputbuf_2, Apfft_Outputbuf_2, APFT_NPT);
 
  Apfft_Outputbuf_1[0] = 0;
  Apfft_Outputbuf_2[0] = 0;
 
  Apfft_Outputbuf_1[1] = 0;
  Apfft_Outputbuf_2[1] = 0;
  //归一化，和*3.3/65535在逻辑上类似，回头可学习一下新算法
  for(int h=2; h<APFT_NPT; h++)
  {
    Apfft_Outputbuf_1[h] /= APFT_NPT / 2;
    Apfft_Outputbuf_2[h] /= APFT_NPT / 2;
  }
	//寻峰
  arm_max_f32(Apfft_Outputbuf_1, APFT_NPT/2, &Amax_1, &Amax_pos_1);//使用APFT_NPT会让频率点在后半部分，导致频率计算错误，导致ARR->0
  arm_max_f32(Apfft_Outputbuf_2, APFT_NPT/2, &Amax_2, &Amax_pos_2);//使用APFT_NPT会让频率点在后半部分，导致频率计算错误，导致ARR->0
	//相位差运算
  Wave_1.phase = Phase_atan(Apfft_Inputbuf_1, Amax_pos_1);
  Wave_2.phase = Phase_atan(Apfft_Inputbuf_2, Amax_pos_2);
  Phase = Wave_1.phase - Wave_2.phase;
	
  if(Phase>180) Phase = -360 + Phase;
  else if(Phase<-180) Phase = 360 + Phase;
}
