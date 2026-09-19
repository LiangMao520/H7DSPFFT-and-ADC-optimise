#ifndef __APFFT_H__
#define __APFFT_H__

#include "main.h"

#define APFT_NPT 1024
#define APFT_SRAMSIZE   APFT_NPT*2*sizeof(uint32_t)//直接用这个进行SRAM移位就行了，源代码用着别扭
/*
ITCM RAM	0x00000000	64KB	CPU执行代码，不要碰喵
DTCM RAM	0x20000000	128KB	CPU√，DMA1/2×
AXI SRAM	0x24000000	512KB	CPU&&DMA
SRAM1	    0x30000000	128KB	CPU&&DMA
SRAM2	    0x30020000	128KB	CPU&&DMA
SRAM3	    0x30040000	32KB	CPU&&DMA
SRAM4	    0x38000000	64KB	CPU&&DMA
其中
0x20000000～0x2001FFFF
0x24000000～0x2407FFFF
这一段是自动存储区，但是似乎也不会存到DTCM里面，不然DMA拿不出来了
SRAM1>>4就是我的私人存储区
*/

extern uint32_t wave_packed[APFT_NPT*2];
extern float Phase;
//这是一种结构体的写法，可以命名结构体，不要用第一次校赛复数那种写法
typedef struct {
    float freq;
    float amplitude;
    float phase;
} 
WaveData;

void Windows_Init(void);
void APFT_Init(void);
void APFT(void);
//one shot的时候DMA请求和DMA搬运都要设置成非轮询，CUBEMX做的不好
#endif
