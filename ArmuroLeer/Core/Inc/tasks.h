#ifndef INC_TASKS_H_
#define INC_TASKS_H_

typedef enum {START, LINE_A, TURN_A, LINE_B, TURN_B, TURN_C, LINE_C, FINALE} ROUTINE_STATE;


void taskFollowTrajectory();
void taskFollowLine();
void taskAvoidObstacle();
void taskSearchLine();
void taskBlinkLED();
void taskOvercomeGap();
void setDriveState(double leftMotorSpeed, double rightMotorSpeed, uint32_t newDistance, ROUTINE_STATE newState);

#endif /* INC_TASKS_H_ */
