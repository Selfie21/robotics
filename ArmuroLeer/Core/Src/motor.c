#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "motor.h"
#include "tasks.h"
#include "gpio.h"

int32_t deviation;
float percentageDeviation;
bool driving = true;
const int8_t KP = 24;

uint32_t ticksLeft;
uint32_t ticksRight;

void setMotorSpeed(float leftMotorSpeed, float rightMotorSpeed) {

	if ((leftMotorSpeed == 0.0f) || (rightMotorSpeed == 0.0f)) {
		driving = false;
	} else {
		driving = true;
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

	leftMotorSpeed = (fabs(leftMotorSpeed) > 1) ? (leftMotorSpeed/leftMotorSpeed) : leftMotorSpeed;
	rightMotorSpeed = (fabs(rightMotorSpeed) > 1) ? (rightMotorSpeed/rightMotorSpeed) : rightMotorSpeed;

	TIM1->CCR2 = (uint32_t) (fabs(leftMotorSpeed) * 65535);
	TIM1->CCR3 = (uint32_t) ((1 - fabs(rightMotorSpeed)) * 65535);
}


void calibrateMotor() {
	if (driving) {
		deviation =  ticksRight - ticksLeft;
		percentageDeviation = 0;
		double currentSpeedLeft = (double) (TIM1->CCR2)/65536;
		double currentSpeedRight = (double) (TIM1->CCR3)/65536;

		double percentageDeviationLeft = (double) (deviation/24.0f)*currentSpeedLeft;
		double percentageDeviationRight = (double) (deviation/24.0f)*currentSpeedRight;

		setMotorSpeed(currentSpeedLeft + percentageDeviationLeft, currentSpeedRight + percentageDeviationRight);
	}
}

