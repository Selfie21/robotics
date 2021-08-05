#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include "tasks.h"

void setMotorSpeed(float leftMotorSpeed, float rightMotorSpeed);
void calibrateMotor();
void setDriveState(double leftMotorSpeed, double rightMotorSpeed, uint32_t newDistance, ROUTINE_STATE newState);
#endif /* INC_MOTOR_H_ */
