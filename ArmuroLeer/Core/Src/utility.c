#include <stdbool.h>
#include <stdio.h>
#include "sensors.h"

// compares if all 3 values from the light sensor (left, middle, right) are smaller/larger than the threshold
// values are != 0 if the test should be val < threshold -> 0 means value is on line to be checked
bool threewayLightComparator(uint8_t a, uint8_t b, uint8_t c, uint32_t threshold){
	uint32_t a1 = adc[5];
	uint32_t b1 = adc[0];
	uint32_t c1 = adc[2];
	uint32_t a2 = threshold;
	uint32_t b2 = threshold;
	uint32_t c2 = threshold;

	if(a != 0){
		a1 = threshold;
		a2 = adc[5];
	}

	if(b != 0){
		b1 = threshold;
		b2 = adc[0];
	}

	if(c != 0){
		c1 = threshold;
		c2 = adc[2];
	}

	if((a1 > a2) && (b1 > b2) && (c1 > c2)){
		return true;
	}else{
		return false;
	}
}


bool lineDetected(uint32_t threshold){
	if (threewayLightComparator(1, 0, 1, threshold) ||
	    threewayLightComparator(0, 0, 1, threshold) ||
	    threewayLightComparator(0, 1, 1, threshold) ||
	    threewayLightComparator(1, 1, 0, threshold) ||
	    threewayLightComparator(1, 0, 0, threshold)){
		return true;
	}else{
		return false;
	}
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
