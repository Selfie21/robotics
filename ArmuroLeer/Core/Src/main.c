/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include<stdbool.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
volatile uint32_t adc[6];
uint32_t buffer[6];
const uint32_t HIGH_THRESHOLD = 2000;
const uint32_t LOW_THRESHOLD = 600;
bool encoderStatusRight;
bool encoderStatusLeft;
uint32_t ticksLeft;
uint32_t ticksRight;


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* Private user code ---------------------------------------------------------*/
void motor_control(double left_motor_speed, double right_motor_speed) {
	if (left_motor_speed > 0) {
		HAL_GPIO_WritePin(GPIOA, phase2_L_Pin, GPIO_PIN_RESET);
	} else if (left_motor_speed < 0) {
		HAL_GPIO_WritePin(GPIOA, phase2_L_Pin, GPIO_PIN_SET);
	}
	if (right_motor_speed > 0) {
		HAL_GPIO_WritePin(GPIOB, phase2_R_Pin, GPIO_PIN_SET);
	} else if (right_motor_speed < 0) {
		HAL_GPIO_WritePin(GPIOB, phase2_R_Pin, GPIO_PIN_RESET);
	}

	if (left_motor_speed < -1 || left_motor_speed > 1) {
		left_motor_speed = 0.5f;
	}

	if (right_motor_speed < -1 || right_motor_speed > 1) {
		right_motor_speed = 0.5f;
	}

	TIM1->CCR2 = (int) (left_motor_speed * 65536);
	TIM1->CCR3 = (int) (right_motor_speed * 65536);
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc1) {
	for (int i = 0; i < 6; i++) {
		adc[i] = buffer[i];

	}
	writeSensorUSB();
}


void writeSensorUSB() {
	char string_buf[100];
	uint8_t len = sprintf((char*) string_buf, "%d, %d, %d, %d, %d, %d\n",
			buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
	HAL_UART_Transmit(&huart2, (uint8_t*) string_buf, len, 1000000);
}


void evaluateEncoder(){
	if(buffer[1] > HIGH_THRESHOLD && !encoderStatusLeft){
		ticksLeft++;
		encoderStatusLeft = true;
	}else if(buffer[1] < LOW_THRESHOLD && encoderStatusLeft){
		ticksLeft++;
		encoderStatusLeft = false;
	}

	if(buffer[4] > HIGH_THRESHOLD && !encoderStatusRight){
		ticksRight++;
		encoderStatusRight = true;
	}else if(buffer[4] < LOW_THRESHOLD && encoderStatusRight){
		ticksRight++;
		encoderStatusRight = false;
	}
}
/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* MCU Configuration--------------------------------------------------------*/
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_USART2_UART_Init();
	MX_ADC1_Init();
	MX_TIM1_Init();

	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);

	TIM1->CCR2 = 0;
	TIM1->CCR3 = 30000;
	motor_control(0.5f, 0.5f);

	/* Infinite loop */
	while (1) {
		HAL_ADC_Start_DMA(&hadc1, buffer, 6);
		evaluateEncoder();
		HAL_Delay(20);

	}
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1)
			!= HAL_OK) {
		Error_Handler();
	}
	/** Configure LSE Drive Capability
	 */
	HAL_PWR_EnableBkUpAccess();
	__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE
			| RCC_OSCILLATORTYPE_MSI;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	RCC_OscInitStruct.MSICalibrationValue = 0;
	RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
	RCC_OscInitStruct.PLL.PLLM = 1;
	RCC_OscInitStruct.PLL.PLLN = 16;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
		Error_Handler();
	}
	/** Enable MSI Auto calibration
	 */
	HAL_RCCEx_EnableMSIPLLMode();
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
