#ifndef INC_UTILITY_H_
#define INC_UTILITY_H_

#include <stdbool.h>

bool threewayLightComparator(uint8_t a, uint8_t b, uint8_t c, uint32_t threshold);
bool lineDetected(uint32_t threshold);

#endif /* INC_UTILITY_H_ */
