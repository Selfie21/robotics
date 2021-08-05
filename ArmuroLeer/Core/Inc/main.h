/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

typedef enum {FOLLOW_TRAJECTORY, AVOID_OBSTACLE, FOLLOW_LINE, OVERCOME_GAP, SEARCH_LINE} TASK_STATE;
extern TASK_STATE robotState;


/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* Private defines -----------------------------------------------------------*/
#define lineSensor_middle_Pin GPIO_PIN_0
#define lineSensor_middle_GPIO_Port GPIOA
#define encoder_left_Pin GPIO_PIN_1
#define encoder_left_GPIO_Port GPIOA
#define VCP_TX_Pin GPIO_PIN_2
#define VCP_TX_GPIO_Port GPIOA
#define lineSensor_right_Pin GPIO_PIN_3
#define lineSensor_right_GPIO_Port GPIOA
#define battery_Pin GPIO_PIN_4
#define battery_GPIO_Port GPIOA
#define encoder_right_Pin GPIO_PIN_5
#define encoder_right_GPIO_Port GPIOA
#define lineSensor_left_Pin GPIO_PIN_7
#define lineSensor_left_GPIO_Port GPIOA
#define Phase1_L_CH2N_Pin GPIO_PIN_0
#define Phase1_L_CH2N_GPIO_Port GPIOB
#define Phase1_R_CH3N_Pin GPIO_PIN_1
#define Phase1_R_CH3N_GPIO_Port GPIOB
#define switch_right_Pin GPIO_PIN_8
#define switch_right_GPIO_Port GPIOA
#define switch_middle_Pin GPIO_PIN_9
#define switch_middle_GPIO_Port GPIOA
#define switch_left_Pin GPIO_PIN_11
#define switch_left_GPIO_Port GPIOA
#define phase2_L_Pin GPIO_PIN_12
#define phase2_L_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define VCP_RX_Pin GPIO_PIN_15
#define VCP_RX_GPIO_Port GPIOA
#define LD3_Pin GPIO_PIN_3
#define LD3_GPIO_Port GPIOB
#define LED_right_Pin GPIO_PIN_4
#define LED_right_GPIO_Port GPIOB
#define LED_left_Pin GPIO_PIN_5
#define LED_left_GPIO_Port GPIOB
#define phase2_R_Pin GPIO_PIN_7
#define phase2_R_GPIO_Port GPIOB


#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
