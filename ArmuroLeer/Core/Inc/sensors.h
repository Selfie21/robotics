#ifndef INC_SENSORS_H_
#define INC_SENSORS_H_

#include "adc.h"

extern uint32_t ticksLeft;
extern uint32_t ticksRight;
extern volatile uint32_t adc[6];
extern uint32_t buffer[6];

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc1);
void writeSensorUSB();
void evaluateEncoder();

#endif /* INC_SENSORS_H_ */
