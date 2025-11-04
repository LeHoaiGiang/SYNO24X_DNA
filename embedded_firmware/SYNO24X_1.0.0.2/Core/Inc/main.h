/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

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
#define V1_Pin GPIO_PIN_2
#define V1_GPIO_Port GPIOE
#define V2_Pin GPIO_PIN_3
#define V2_GPIO_Port GPIOE
#define V3_Pin GPIO_PIN_4
#define V3_GPIO_Port GPIOE
#define V4_Pin GPIO_PIN_5
#define V4_GPIO_Port GPIOE
#define V5_Pin GPIO_PIN_6
#define V5_GPIO_Port GPIOE
#define V6_Pin GPIO_PIN_13
#define V6_GPIO_Port GPIOC
#define V7_Pin GPIO_PIN_0
#define V7_GPIO_Port GPIOF
#define V8_Pin GPIO_PIN_1
#define V8_GPIO_Port GPIOF
#define V9_Pin GPIO_PIN_2
#define V9_GPIO_Port GPIOF
#define V10_Pin GPIO_PIN_3
#define V10_GPIO_Port GPIOF
#define V11_Pin GPIO_PIN_4
#define V11_GPIO_Port GPIOF
#define V12_Pin GPIO_PIN_5
#define V12_GPIO_Port GPIOF
#define V13_Pin GPIO_PIN_6
#define V13_GPIO_Port GPIOF
#define V14_Pin GPIO_PIN_7
#define V14_GPIO_Port GPIOF
#define V15_Pin GPIO_PIN_8
#define V15_GPIO_Port GPIOF
#define V16_Pin GPIO_PIN_9
#define V16_GPIO_Port GPIOF
#define V17_Pin GPIO_PIN_10
#define V17_GPIO_Port GPIOF
#define V20_Pin GPIO_PIN_0
#define V20_GPIO_Port GPIOC
#define V21_Pin GPIO_PIN_1
#define V21_GPIO_Port GPIOC
#define V22_Pin GPIO_PIN_2
#define V22_GPIO_Port GPIOC
#define V23_Pin GPIO_PIN_3
#define V23_GPIO_Port GPIOC
#define PWM1_Pin GPIO_PIN_9
#define PWM1_GPIO_Port GPIOE
#define PWM4_Pin GPIO_PIN_14
#define PWM4_GPIO_Port GPIOE
#define V19_Pin GPIO_PIN_13
#define V19_GPIO_Port GPIOB
#define V18_Pin GPIO_PIN_15
#define V18_GPIO_Port GPIOB
#define V39_Pin GPIO_PIN_9
#define V39_GPIO_Port GPIOD
#define V40_Pin GPIO_PIN_10
#define V40_GPIO_Port GPIOD
#define V38_Pin GPIO_PIN_11
#define V38_GPIO_Port GPIOD
#define V37_Pin GPIO_PIN_13
#define V37_GPIO_Port GPIOD
#define V36_Pin GPIO_PIN_14
#define V36_GPIO_Port GPIOD
#define V35_Pin GPIO_PIN_15
#define V35_GPIO_Port GPIOD
#define V34_Pin GPIO_PIN_2
#define V34_GPIO_Port GPIOG
#define V33_Pin GPIO_PIN_3
#define V33_GPIO_Port GPIOG
#define V32_Pin GPIO_PIN_4
#define V32_GPIO_Port GPIOG
#define V31_Pin GPIO_PIN_5
#define V31_GPIO_Port GPIOG
#define V30_Pin GPIO_PIN_6
#define V30_GPIO_Port GPIOG
#define V29_Pin GPIO_PIN_7
#define V29_GPIO_Port GPIOG
#define V28_Pin GPIO_PIN_8
#define V28_GPIO_Port GPIOG
#define V27_Pin GPIO_PIN_6
#define V27_GPIO_Port GPIOC
#define V26_Pin GPIO_PIN_7
#define V26_GPIO_Port GPIOC
#define V25_Pin GPIO_PIN_8
#define V25_GPIO_Port GPIOC
#define V24_Pin GPIO_PIN_9
#define V24_GPIO_Port GPIOC
#define X_DIR_Pin GPIO_PIN_15
#define X_DIR_GPIO_Port GPIOA
#define X_STEP_Pin GPIO_PIN_11
#define X_STEP_GPIO_Port GPIOC
#define X_EN_Pin GPIO_PIN_0
#define X_EN_GPIO_Port GPIOD
#define Y_DIR_Pin GPIO_PIN_2
#define Y_DIR_GPIO_Port GPIOD
#define Y_STEP_Pin GPIO_PIN_4
#define Y_STEP_GPIO_Port GPIOD
#define CTRL_OE2_Pin GPIO_PIN_5
#define CTRL_OE2_GPIO_Port GPIOD
#define Y_EN_Pin GPIO_PIN_6
#define Y_EN_GPIO_Port GPIOD
#define Z1_EN_Pin GPIO_PIN_7
#define Z1_EN_GPIO_Port GPIOD
#define Z1_DIR_Pin GPIO_PIN_9
#define Z1_DIR_GPIO_Port GPIOG
#define Z1_STEP_Pin GPIO_PIN_11
#define Z1_STEP_GPIO_Port GPIOG
#define CTRL_OE1_Pin GPIO_PIN_13
#define CTRL_OE1_GPIO_Port GPIOG
#define buzzer_Pin GPIO_PIN_3
#define buzzer_GPIO_Port GPIOB
#define X_LIMIT_Pin GPIO_PIN_8
#define X_LIMIT_GPIO_Port GPIOB
#define Y_LIMIT_Pin GPIO_PIN_9
#define Y_LIMIT_GPIO_Port GPIOB
#define Z1_LIMIT_Pin GPIO_PIN_0
#define Z1_LIMIT_GPIO_Port GPIOE
#define Z2_LIMIT_Pin GPIO_PIN_1
#define Z2_LIMIT_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
