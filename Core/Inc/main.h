/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define GET_PSC(p) ((p)->Instance->PSC)

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define AD9910_SCLK_Pin GPIO_PIN_3
#define AD9910_SCLK_GPIO_Port GPIOE
#define AD9910_SDIO_Pin GPIO_PIN_4
#define AD9910_SDIO_GPIO_Port GPIOE
#define AD9910_MASTER_RESET_Pin GPIO_PIN_5
#define AD9910_MASTER_RESET_GPIO_Port GPIOE
#define AD9910_CS_Pin GPIO_PIN_6
#define AD9910_CS_GPIO_Port GPIOE
#define AD9910_PWR_Pin GPIO_PIN_9
#define AD9910_PWR_GPIO_Port GPIOI
#define AD9910_PROFILE1_Pin GPIO_PIN_10
#define AD9910_PROFILE1_GPIO_Port GPIOI
#define AD9910_OSK_Pin GPIO_PIN_10
#define AD9910_OSK_GPIO_Port GPIOF
#define AD9910_DRCTL_Pin GPIO_PIN_13
#define AD9910_DRCTL_GPIO_Port GPIOG
#define AD9910_DRHOLD_Pin GPIO_PIN_4
#define AD9910_DRHOLD_GPIO_Port GPIOB
#define AD9910_DROVER_Pin GPIO_PIN_5
#define AD9910_DROVER_GPIO_Port GPIOB
#define AD9910_IO_UPDATE_Pin GPIO_PIN_7
#define AD9910_IO_UPDATE_GPIO_Port GPIOB
#define AD9910_PROFILE2_Pin GPIO_PIN_8
#define AD9910_PROFILE2_GPIO_Port GPIOB
#define AD9910_PROFILE2B9_Pin GPIO_PIN_9
#define AD9910_PROFILE2B9_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
