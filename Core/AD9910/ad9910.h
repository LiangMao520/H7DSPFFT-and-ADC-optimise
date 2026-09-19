#ifndef __AD9910_H
#define __AD9910_H

#include "main.h"
#include "gpio.h"

#define AD9910_MAX_OUTPUT_FREQ_HZ  420000000UL
#define AD9910_SWEEP_DWELL_MS      1U
/* ?????x ? A : B ????????????? x ????0????? A????? B?*/
#define AD9910_PWR(x)  do{ x ? \
                                 HAL_GPIO_WritePin(GPIOI, GPIO_PIN_9, GPIO_PIN_SET) : \
                                 HAL_GPIO_WritePin(GPIOI, GPIO_PIN_9, GPIO_PIN_RESET); \
                                 }while(0)  
// 原始IO
#define PROFILE1(x) do{ x ? \
                                 HAL_GPIO_WritePin(GPIOI, GPIO_PIN_10, GPIO_PIN_SET) : \
                                 HAL_GPIO_WritePin(GPIOI, GPIO_PIN_10, GPIO_PIN_RESET); \
                                 }while(0)      
#define OSK(x) do{ x ? \
                                 HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET) : \
                                 HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_RESET); \
                                 }while(0)  
#define DRCTL(x) do{ x ? \
                                 HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_SET) : \
                                 HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET); \
                                 }while(0)  
#define AD9910_SDIO(x) do{ x ? \
                                 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET) : \
                                 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET); \
                                 }while(0)  
#define SCLK(x) do{ x ? \
                                 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET) : \
                                 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET); \
                                 }while(0)  
#define CS(x) do{ x ? \
                                 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_SET) : \
                                 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_RESET); \
                                 }while(0)  
#define MAS_REST(x) do{ x ? \
                                 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET) : \
                                 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET); \
                                 }while(0)  
#define PROFILE2(x) do{ x ? \
                                 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET) : \
                                 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET); \
                                 }while(0)  
#define PROFILE0(x) do{ x ? \
                                 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET) : \
                                 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET); \
                                 }while(0)  
#define UP_DAT(x) do{ x ? \
                                 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET) : \
                                 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); \
                                 }while(0)  
#define DROVER(x) do{ x ? \
                                 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET) : \
                                 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); \
                                 }while(0)  
#define DRHOLD(x) do{ x ? \
                                 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET) : \
                                 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET); \
                                 }while(0)  

typedef enum {
	TRIG_WAVE = 0,
	SQUARE_WAVE,
	SINC_WAVE,
} AD9910_WAVE_ENUM;

void Init_AD9910(void);
void AD9910_Sweep_Fre(uint32_t Start_Fre, uint32_t Stop_Fre, uint32_t Step_Fre);

void AD9910_RAM_WAVE_Set(AD9910_WAVE_ENUM wave);

void AD9910_TRI_FreWrite(uint32_t Freq);
void AD9910_TRI_PhyWrite(uint16_t Phy);
void AD9910_TRI_AmpWrite(uint16_t Amp);

void AD9910_SIN_FreWrite(unsigned long Freq);										//写频率
void AD9910_SIN_Float_FreWrite(double Freq);								//写可带小数的频率，单位Hz
void AD9910_SIN_PhyWrite(uint16_t Phy);
void AD9910_SIN_AmpWrite(uint16_t Amp);

void AD9910_SQUARE_FreWrite(uint32_t Freq);
void AD9910_SQUARE_PhyWrite(uint16_t Phy);
void AD9910_SQUARE_AmpWrite(uint16_t Amp);

void AD9910_SINC_FreWrite(uint32_t Freq);
void AD9910_SINC_PhyWrite(uint16_t Phy);
void AD9910_SINC_AmpWrite(uint16_t Amp);
#endif


