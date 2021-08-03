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
#include <stdbool.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
volatile uint32_t adc[6];
uint32_t buffer[6];

const uint32_t HIGH_THRESHOLD = 2000;
const uint32_t LOW_THRESHOLD = 900;
bool encoderStatusRight;
bool encoderStatusLeft;
uint32_t ticksLeft;
uint32_t ticksRight;

const uint32_t TRIGGER_PER_CM = 2;
const double TRIGGER_PER_DEGREE_RIGHT = 0.635;
const double TRIGGER_PER_DEGREE_LEFT = 0.13;


const uint8_t KP = 1;
double percentageDiff;
uint32_t diff;
bool notDriving = true;

enum LED_STATE {stateA, stateB};
enum LED_STATE taskLedState = stateA;
unsigned long long waitingSince = 0;

enum DRIVE_ROUTINE_STATE {start, firstStraight, firstTurn, secondStraight, secondTurn, thirdStraight};
enum DRIVE_ROUTINE_STATE driveRoutineStart = start;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* Private user code ---------------------------------------------------------*/
void controlMotor(double leftMotorSpeed, double rightMotorSpeed) {

	if((leftMotorSpeed == 0.0f) || (rightMotorSpeed == 0.0f)){
		notDriving = true;
	}else{
		notDriving = false;
	}

	if (leftMotorSpeed > 0) {
		HAL_GPIO_WritePin(GPIOA, phase2_L_Pin, GPIO_PIN_RESET);
	} else if (leftMotorSpeed < 0) {
		HAL_GPIO_WritePin(GPIOA, phase2_L_Pin, GPIO_PIN_SET);
	}
	if (rightMotorSpeed > 0) {
		HAL_GPIO_WritePin(GPIOB, phase2_R_Pin, GPIO_PIN_SET);
	} else if (rightMotorSpeed < 0) {
		HAL_GPIO_WritePin(GPIOB, phase2_R_Pin, GPIO_PIN_RESET);
	}

	if (leftMotorSpeed < -1 || leftMotorSpeed > 1) {
		leftMotorSpeed = 0.5f;
	}

	if (rightMotorSpeed < -1 || rightMotorSpeed > 1) {
		rightMotorSpeed = 0.5f;
	}

	TIM1->CCR2 = (int) (leftMotorSpeed * 65535);
	TIM1->CCR3 = (int) ((1 - rightMotorSpeed) * 65535);
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc1) {
	for (int i = 0; i < 6; i++) {
		adc[i] = buffer[i];

	}
	writeSensorUSB();
}


void writeSensorUSB() {
	char stringBuf[100];
	int len = sprintf((char*) stringBuf, "%d, %d, %d, %d, %d, %d\n",
			adc[0], adc[1], adc[2], adc[3], adc[4], adc[5]);
	HAL_UART_Transmit(&huart2, (uint8_t*) stringBuf, len, 1000000);
}


void evaluateEncoder(){
	if(adc[1] > HIGH_THRESHOLD && !encoderStatusLeft){
		ticksLeft++;
		encoderStatusLeft = true;
	}else if(adc[1] < LOW_THRESHOLD && encoderStatusLeft){
		ticksLeft++;
		encoderStatusLeft = false;
	}

	if(adc[4] > HIGH_THRESHOLD && !encoderStatusRight){
		ticksRight++;
		encoderStatusRight = true;
	}else if(adc[4] < LOW_THRESHOLD && encoderStatusRight){
		ticksRight++;
		encoderStatusRight = false;
	}
}


void taskLED() {

	switch(taskLedState) {
	case stateA:
		if(HAL_GetTick() > (waitingSince + 500)){
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
			waitingSince = HAL_GetTick();
			taskLedState = stateB;
		}
		break;

	case stateB:
		if(HAL_GetTick() > (waitingSince + 500)){
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
			waitingSince = HAL_GetTick();
			taskLedState = stateA;
		}
		break;
	}
}



// taking right encoder as as base (more accurate) so changing of speed happens to left motor
void regulateMotor(){
	if(!notDriving){
		diff =  ticksRight - ticksLeft;
		percentageDiff = 0;

		if(ticksLeft != 0){
			percentageDiff = (double) diff/ticksLeft;
		}else{
			diff = 0;
		}

		double currentSpeedLeft = (double) (TIM1->CCR2)/65536;
		double currentSpeedRight = (double) (TIM1->CCR3)/65536;
		if(diff > 0){
			controlMotor(currentSpeedLeft+(KP * percentageDiff), currentSpeedRight);
		}else if(diff < 0){
			controlMotor(currentSpeedLeft-(KP *percentageDiff), currentSpeedRight);
		}
	}
}

uint32_t triggerSinceChange = 0;
uint32_t distanceToCover = 0;
void driveTestDemo(){

	switch(driveRoutineStart) {

	case start:
		driveRoutineStart = firstStraight;
		controlMotor(0.5f, 0.5f);
		triggerSinceChange = ticksRight;
		distanceToCover = 47 * TRIGGER_PER_CM;
		break;

	case firstStraight:
		if(ticksRight > (triggerSinceChange + distanceToCover)){
			driveRoutineStart = firstTurn;
			controlMotor(0.5f, -0.5f);
			distanceToCover = (uint32_t) (30 * TRIGGER_PER_DEGREE_RIGHT);
			triggerSinceChange = ticksRight;
		}
		break;

	case firstTurn:
		if(ticksRight > (triggerSinceChange + distanceToCover)){
			driveRoutineStart = secondStraight;
			controlMotor(0.5f, 0.5f);
			distanceToCover = (uint32_t) 35.5f * TRIGGER_PER_CM;
			triggerSinceChange = ticksRight;
		}
		break;

	case secondStraight:
		if(ticksRight > (triggerSinceChange + distanceToCover)){
			driveRoutineStart = secondTurn;
			controlMotor(-0.5f, 0.5f);
			distanceToCover = (uint32_t) (90 * TRIGGER_PER_DEGREE_LEFT);
			triggerSinceChange = ticksRight;
		}
		break;

	case secondTurn:
		if(ticksRight > (triggerSinceChange + distanceToCover)){
			driveRoutineStart = thirdStraight;
			controlMotor(0.5f, 0.5f);
			distanceToCover = 32 * TRIGGER_PER_CM;
			triggerSinceChange = ticksRight;
		}
		break;

	case thirdStraight:
		if(ticksRight > (triggerSinceChange + distanceToCover)){
			controlMotor(0.0f, 0.0f);
		}
		break;
	}
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

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
	ticksLeft = 0;
	ticksRight = 0;
	controlMotor(0.5f, 0.5f);
	while (1) {
		HAL_ADC_Start_DMA(&hadc1, buffer, 6);
		taskLED();
		evaluateEncoder();
		driveTestDemo();
		regulateMotor();
		HAL_Delay(20);
	}
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	/** Configure LSE Drive Capability
	 */
	HAL_PWR_EnableBkUpAccess();
	__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
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
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
	{
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_ADC;
	PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
	PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
	PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_MSI;
	PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
	PeriphClkInit.PLLSAI1.PLLSAI1N = 16;
	PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
	PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
	PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
	PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_ADC1CLK;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		Error_Handler();
	}
	/** Configure the main internal regulator output voltage
	 */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
	{
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
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
