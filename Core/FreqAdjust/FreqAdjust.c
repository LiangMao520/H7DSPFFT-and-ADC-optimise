#include "FreqAdjust.h"

//先沿用23H的修正增益和单次最大修正量，实测后再调整
#define FREQ_GAIN       0.5f
#define FREQ_MAX_STEP   0.5f
#define FREQ_MAX_RANGE  20.0

static double FreqStart = 0.0;
static double FreqNow = 0.0;
static float PhaseLast = 0.0f;
static uint32_t TimeLast = 0U;
static uint8_t FirstFlag = 1U;

//将相位变化限制到-180度～+180度，避免跨越边界时得到错误变化量
static float Phase_Wrap(float Phase)
{
	if(Phase > 180.0f)
	{
		Phase -= 360.0f;
	}
	else if(Phase < -180.0f)
	{
		Phase += 360.0f;
	}

	return Phase;
}

/************************************************************
** 函数名称 ：void FreqAdjust_Init(double Freq)
** 函数功能 ：保存AD9910当前频率，并清除上一次相位测量
** 入口参数 ：Freq：AD9910当前频率，单位Hz
** 出口参数 ：无
** 函数说明 ：设置或更换初始频率后调用一次
**************************************************************/
void FreqAdjust_Init(double Freq)
{
	FreqStart = Freq;
	FreqNow = Freq;
	PhaseLast = 0.0f;
	TimeLast = 0U;
	FirstFlag = 1U;
}

/************************************************************
** 函数名称 ：double FreqAdjust_Update(float PhaseNow, uint32_t TimeNow)
** 函数功能 ：根据相位差的变化速度估算频差，并修正输出频率
** 入口参数 ：PhaseNow：本次输入相位减输出相位，单位度
**            TimeNow ：本帧采样完成时刻，单位ms
** 出口参数 ：修正后的AD9910频率，单位Hz
** 函数说明 ：只计算频率，不在函数内部操作AD9910
**************************************************************/
double FreqAdjust_Update(float PhaseNow, uint32_t TimeNow)
{
	uint32_t TimeDiff;
	float PhaseDiff;
	float FreqDiff;
	float FreqStep;

	//Phase无效时放弃本次结果，下一帧重新建立相位参考
	if(!((PhaseNow >= -180.0f) && (PhaseNow <= 180.0f)))
	{
		FirstFlag = 1U;
		return FreqNow;
	}

	//第一次只有一个相位值，无法计算相位漂移率
	if(FirstFlag)
	{
		PhaseLast = PhaseNow;
		TimeLast = TimeNow;
		FirstFlag = 0U;
		return FreqNow;
	}

	TimeDiff = TimeNow - TimeLast;
	if(TimeDiff == 0U)
	{
		return FreqNow;
	}

	PhaseDiff = Phase_Wrap(PhaseNow - PhaseLast);

	//Phase=输入相位-输出相位，因此这里得到的是输入频率-输出频率
	FreqDiff = PhaseDiff * 1000.0f / (360.0f * (float)TimeDiff);
	FreqStep = FREQ_GAIN * FreqDiff;

	if(FreqStep > FREQ_MAX_STEP)
	{
		FreqStep = FREQ_MAX_STEP;
	}
	else if(FreqStep < -FREQ_MAX_STEP)
	{
		FreqStep = -FREQ_MAX_STEP;
	}

	FreqNow += (double)FreqStep;
	if(FreqNow > FreqStart + FREQ_MAX_RANGE)
	{
		FreqNow = FreqStart + FREQ_MAX_RANGE;
	}
	else if(FreqNow < FreqStart - FREQ_MAX_RANGE)
	{
		FreqNow = FreqStart - FREQ_MAX_RANGE;
	}

	PhaseLast = PhaseNow;
	TimeLast = TimeNow;

	return FreqNow;
}
