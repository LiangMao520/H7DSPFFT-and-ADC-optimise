#ifndef __FREQ_ADJUST_H__
#define __FREQ_ADJUST_H__

#include <stdint.h>

/*
调用顺序：
1. 设置AD9910初始频率后，调用FreqAdjust_Init()保存当前频率；
2. 每次APFT得到新的Phase后，调用FreqAdjust_Update()计算新的频率；
3. 将返回值传给AD9910_SIN_Float_FreWrite()。
注意：该模块只用于粗测频完成后的细锁频，不能单独判断较大的频率跳变。
*/
void FreqAdjust_Init(double Freq);
double FreqAdjust_Update(float PhaseNow, uint32_t TimeNow);

#endif
