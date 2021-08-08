#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "motor.h"
#include "tasks.h"
#include "gpio.h"

float currentSpeedLeft;
float currentSpeedRight;
uint32_t ticksLeft;
uint32_t ticksRight;

int32_t deviation;
float pDeviation;
const float KP = 0.1f;
bool driving = true;

void setMotorSpeed(float leftMotorSpeed, float rightMotorSpeed) {

	driving = false ? ((leftMotorSpeed == 0.0f) || (rightMotorSpeed == 0.0f)) : true;

	currentSpeedLeft = leftMotorSpeed;
	currentSpeedRight = rightMotorSpeed;

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

	TIM1->CCR2 = (uint32_t) (fabs(leftMotorSpeed) * 65535);
	TIM1->CCR3 = (uint32_t) ((1 - fabs(rightMotorSpeed)) * 65535);
}


void calibrateMotor() {
	if (driving) {
		float oldSpeedLeft = currentSpeedLeft;
		float oldSpeedRight = currentSpeedRight;

		deviation =  ticksRight - ticksLeft;
		pDeviation = deviation * KP;
		setMotorSpeed(currentSpeedLeft + pDeviation, currentSpeedRight - pDeviation);

		// if we are calibrating we want to hover around the speed -> no changing of the fixed Speed
		currentSpeedLeft = oldSpeedLeft;
		currentSpeedRight = oldSpeedRight;
	}
}

