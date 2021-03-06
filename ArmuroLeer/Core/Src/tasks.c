#include <stdio.h>
#include <stdbool.h>

#include "motor.h"
#include "gpio.h"
#include "sensors.h"
#include "utility.h"

TASK_STATE robotState;
uint32_t ticksRight;
uint32_t ticksLeft;

const uint32_t WHITE_THRESHOLD = 2000;
const uint32_t TRIGGER_PER_CM = 2;
const double TRIGGER_PER_DEGREE_RIGHT = 0.125;
const double TRIGGER_PER_DEGREE_LEFT = 0.125;

ROUTINE_STATE routineState = START;
uint32_t routinePersistent = 0;
uint32_t distanceToCover = 0;

enum LED_STATE {LED_ON, LED_OFF};
enum LED_STATE ledState = LED_ON;
uint32_t ledPersistent = 0;

void setDriveState(double leftMotorSpeed, double rightMotorSpeed, uint32_t newDistance, ROUTINE_STATE newState){
	if(ticksRight > (routinePersistent + distanceToCover)){
		routineState = newState;
		setMotorSpeed(leftMotorSpeed, rightMotorSpeed);
		routinePersistent = ticksRight;
		distanceToCover = newDistance;
	}
}


void taskFollowLine() {
	routineState = START;

	if (threewayLightComparator(1, 0, 1, WHITE_THRESHOLD)) {
		setMotorSpeed(0.5f, 0.5f);
	} else if ((threewayLightComparator(0, 0, 1, WHITE_THRESHOLD))
			|| (threewayLightComparator(0, 1, 1, WHITE_THRESHOLD))) {
		setMotorSpeed(-0.5f, 0.5f);
	} else if ((threewayLightComparator(1, 1, 0, WHITE_THRESHOLD))
			|| (threewayLightComparator(1, 0, 0, WHITE_THRESHOLD))) {
		setMotorSpeed(0.5f, -0.5f);
	} else {
		robotState = SEARCH_LINE;
		setMotorSpeed(0.5f, 0.5f);
	}

}

void taskFollowTrajectory(){

	switch(routineState) {
		case START:
			routineState = LINE_A;
			setMotorSpeed(0.5f, 0.5f);
			routinePersistent = ticksRight;
			distanceToCover = 47 * TRIGGER_PER_CM;
			break;

		case LINE_A:
			setDriveState(0.5f, -0.5f, (uint32_t) (150 * TRIGGER_PER_DEGREE_RIGHT), TURN_A);
			break;

		case TURN_A:
			setDriveState(0.5f, 0.5f, (uint32_t) 35.5f * TRIGGER_PER_CM, LINE_B);
			break;

		case LINE_B:
			setDriveState(-0.5f, 0.5f, (uint32_t) (90 * TRIGGER_PER_DEGREE_LEFT), TURN_B);
			break;

		case TURN_B:
			setDriveState(0.5f, 0.5f, 29 * TRIGGER_PER_CM, FINALE);
			break;

		case FINALE:
			if(ticksRight > (routinePersistent + distanceToCover)){
				robotState = FOLLOW_LINE;
				routineState = START;
			}
			break;
	}
}


void taskAvoidObstacle(){

	switch(routineState) {

		case START:
			routineState = LINE_C;
			setMotorSpeed(-0.5f, -0.5f);
			routinePersistent = ticksRight;
			distanceToCover = (8 * TRIGGER_PER_CM);
			break;

		case LINE_C:
			setDriveState(0.5f, -0.5f, (uint32_t) (90 * TRIGGER_PER_DEGREE_RIGHT), TURN_A);

		case TURN_A:
			setDriveState(0.5f, 0.5f, (8 * TRIGGER_PER_CM), LINE_A);
			break;

		case LINE_A:
			setDriveState(-0.5f, 0.5f, (uint32_t) (90 * TRIGGER_PER_DEGREE_LEFT), TURN_B);
			break;

		case TURN_B:
			setDriveState(0.5f, 0.5f, (30 * TRIGGER_PER_CM), LINE_B);
			break;

		case LINE_B:
			setDriveState(-0.5f, 0.5f, (uint32_t) (90 * TRIGGER_PER_DEGREE_LEFT), TURN_C);
			break;

		case TURN_C:
			setDriveState(0.5f, 0.5f, (8 * TRIGGER_PER_CM), FINALE);
			break;

		case FINALE:
			if(ticksRight > (routinePersistent + distanceToCover)){
				routineState = START;
				robotState = FOLLOW_LINE;
			}
			break;
	}

}





void taskSearchLine(){

	switch(routineState){
		case START:
			routineState = TURN_A;
			setMotorSpeed(0.5f, -0.5f);
			routinePersistent = ticksRight;
			distanceToCover = (uint32_t) (100 * TRIGGER_PER_DEGREE_RIGHT);
			break;

		case TURN_A:
			setDriveState(-0.5f, 0.5f, (uint32_t) (200 * TRIGGER_PER_DEGREE_LEFT), TURN_B);
			break;

		case TURN_B:
			setDriveState(0.5f, -0.5f, (uint32_t) (100 * TRIGGER_PER_DEGREE_RIGHT), FINALE);
			break;

		case FINALE:
			if(ticksRight > (routinePersistent + distanceToCover)){
				robotState = OVERCOME_GAP;
				routineState = START;
			}
			break;
	}

}


void taskOvercomeGap(){
	if(routineState == START){
		setMotorSpeed(0.5f, 0.5f);
		routinePersistent = ticksRight;
		distanceToCover = (uint32_t) (30 * TRIGGER_PER_CM);
	}

	if(ticksRight > (routinePersistent + distanceToCover)){
		robotState = FOLLOW_LINE;
	}
}

void taskBlinkLED() {
	switch (ledState) {
		case LED_ON:
			if (HAL_GetTick() > (ledPersistent + 500)) {
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
				ledPersistent = HAL_GetTick();
				ledState = LED_OFF;
			} break;

		case LED_OFF:
			if (HAL_GetTick() > (ledPersistent + 500)) {
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
				ledPersistent = HAL_GetTick();
				ledState = LED_ON;
			} break;
	}
}

