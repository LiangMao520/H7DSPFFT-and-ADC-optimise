/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "fft.h"
#include "apfft.h"
#include "transmit.h"
#include "ad9910.h"
#include "FreqAdjust.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FREQ_ADJUST_MIN_TIME_MS  100U
#define FREQ_RECAPTURE_HZ        3.0f
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
volatile uint8_t AD1_3Flag = 0;
volatile uint8_t APFT1Flag = 0;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint8_t TIM15IsUpdate = 0;
volatile uint16_t CNT_1 = 0;
volatile uint16_t CNT_2 = 0;
volatile float TIMgetfreq = 0;
volatile uint16_t ADC1_3[NPT];
float vout,fre;
double FreqCmd = 0.0;
float FreqCoarse = 0.0f;
uint32_t AdjustTime = 0U;
uint8_t LockFlag = 0U;
uint8_t PhaseRefFlag = 0U;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */
void main_apft(void);
void ADC1_Channel3(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */
	FFT_Init();
  Windows_Init();
  APFT_Init();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_ADC2_Init();
  MX_ADC3_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(500);
  Init_AD9910();
  HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
  HAL_ADC_Start_DMA(&hadc3,(uint32_t*)ADC1_3,NPT);

	HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
	HAL_ADCEx_Calibration_Start(&hadc2, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
//ADC1数据是低十六位wave_packed[i] & 0xFFFFU
//ADC2数据是高十六位wave_packed[i] >> 16;
  HAL_ADCEx_MultiModeStart_DMA(&hadc1,wave_packed,APFT_NPT*2);

  DMA1_Stream0->CR &= ~DMA_SxCR_HTIE;
/*
阴
NORMAL要设置完成标志位，因此要调用HAL_DMA_IRQHandler的完成事件，不能手动清除TC标志位了
因此这里禁用TH中断是必要的
或者用if筛选也行
*/
  // HAL_TIM_Base_Start(&htim6);
  // HAL_TIM_Base_Start_IT(&htim8);
  // HAL_TIM_Base_Start_IT(&htim15);

    AD9910_SIN_AmpWrite(16383); 
    // AD9910_SIN_Float_FreWrite((double)26666.66);
    AD9910_SIN_FreWrite(1000000);
    AD9910_SIN_PhyWrite(0);                                                 
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // ADC1_Channel3();
		// main_apft();
    
    HAL_Delay(1000);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInitStruct.PLL2.PLL2M = 5;
  PeriphClkInitStruct.PLL2.PLL2N = 72;
  PeriphClkInitStruct.PLL2.PLL2P = 5;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void main_apft(void){
    uint32_t PhaseTime;
    float FreqNew;
    uint8_t ResetFlag = 0U;

    while(!APFT1Flag);
    PhaseTime = HAL_GetTick();
    HAL_TIM_Base_Stop(&htim6);
    HAL_ADCEx_MultiModeStop_DMA(&hadc1);
//算法
    APFT();

    //粗测频第一次有效，或者输入频率明显变化时，重新设置AD9910并开始细锁
    if(TIM15IsUpdate)
    {
        FreqNew = TIMgetfreq;
        TIM15IsUpdate = 0;
        USART1_printf("FreqTIM=%.3f\r\n", FreqNew);

        if((FreqNew > 0.0f) &&
           ((!LockFlag) ||
            (FreqNew > FreqCoarse + FREQ_RECAPTURE_HZ) ||
            (FreqNew < FreqCoarse - FREQ_RECAPTURE_HZ)))//明显变化
        {
            FreqCoarse = FreqNew;
            FreqCmd = (double)FreqCoarse;
            AD9910_SIN_Float_FreWrite(FreqCmd);
            FreqAdjust_Init(FreqCmd);//重启
            LockFlag = 1U;
            PhaseRefFlag = 0U;
            ResetFlag = 1U;
            USART1_printf("Capture=%.6f\r\n", FreqCmd);
        }
    }

    //重新设置频率后的下一帧只建立相位参考；以后实际间隔达到100ms再修正
    if(LockFlag && !ResetFlag)
    {
        if(!PhaseRefFlag)
        {
            FreqAdjust_Update(Phase, PhaseTime);
            AdjustTime = PhaseTime;
            PhaseRefFlag = 1U;
        }
        else if((PhaseTime - AdjustTime) >= FREQ_ADJUST_MIN_TIME_MS)
        {
            FreqCmd = FreqAdjust_Update(Phase, PhaseTime);//根据apft的相位和halgettick的时间先计算调整相位，等到了100ms+再进行频率调整
            AD9910_SIN_Float_FreWrite(FreqCmd);
            AdjustTime = PhaseTime;
            USART1_printf("FreqCmd=%.6f\r\n", FreqCmd);
        }
    }

    USART1_printf("Phase=%.3f\r\n", Phase);
    // for(int i = 0; i < APFT_NPT*2; i++){
    //     USART1_printf("%u,%u\r\n", (uint16_t)(wave_packed[i] & 0xFFFFU), (uint16_t)(wave_packed[i] >> 16));
    // }

    APFT1Flag = 0;
    HAL_ADCEx_MultiModeStart_DMA(&hadc1,wave_packed,APFT_NPT*2);
    DMA1_Stream0->CR &= ~DMA_SxCR_HTIE;
    HAL_TIM_Base_Start(&htim6);
}

void ADC1_Channel3(void){
    HAL_TIM_Base_Start(&htim3);     
    while(!AD1_3Flag);

    // for(int i=0;i<NPT;i++){
    // USART1_printf("%d\r\n",ADC1_3[i]);
    // }

    FFT_Calculation(ADC1_3,&vout,&fre);
    USART1_printf("V=%.3f\r\n",vout);
    USART1_printf("f=%.3f\r\n",fre);

    int wave_judging_result = Wave_Judging(ADC1_3);
    if(wave_judging_result){
        USART1_printf("tri\r\n");
    }
    else{
        USART1_printf("sin\r\n");
    }
		AD1_3Flag = 0;
}
/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
