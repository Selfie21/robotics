#include <stdio.h>
#include <stdbool.h>

#include "sensors.h"
#include "adc.h"
#include "usart.h"

const uint32_t HIGH_THRESHOLD = 2500;
const uint32_t LOW_THRESHOLD = 1800;
bool encoderStatusLeft;
bool encoderStatusRight;

uint32_t ticksLeft = 0;
uint32_t ticksRight = 0;
volatile uint32_t adc[6];
uint32_t buffer[6];

void evaluateEncoder() {
	if (adc[1] > HIGH_THRESHOLD && !encoderStatusLeft) {
		ticksLeft++;
		encoderStatusLeft = true;
	} else if (adc[1] < LOW_THRESHOLD && encoderStatusLeft) {
		ticksLeft++;
		encoderStatusLeft = false;
	}

	if (adc[4] > HIGH_THRESHOLD && !encoderStatusRight) {
		ticksRight++;
		encoderStatusRight = true;
	} else if (adc[4] < LOW_THRESHOLD && encoderStatusRight) {
		ticksRight++;
		encoderStatusRight = false;
	}
}

void writeSensorUSB() {
	char stringBuf[100];
	int len = sprintf((char*) stringBuf, "%d, %d, %d, %d, %d, %d\n", adc[0],
			adc[1], adc[2], adc[3], adc[4], adc[5]);
	HAL_UART_Transmit(&huart2, (uint8_t*) stringBuf, len, 1000000);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc1) {
	for (int i = 0; i < 6; i++) {
		adc[i] = buffer[i];
	}
	writeSensorUSB();
}

bool objectDetected(){
	GPIO_PinState left = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11);
	GPIO_PinState middle = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9);
	GPIO_PinState right = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8);
	if((left == GPIO_PIN_RESET) || (middle == GPIO_PIN_RESET) || (right == GPIO_PIN_RESET)){
		return true;
	}else{
		return false;
	}
}
