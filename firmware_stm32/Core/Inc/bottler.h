/*
 * bottler.h
 *
 * Created on: 14 may 2026
 * Author: agust
 */

#ifndef INC_BOTTLER_H_
#define INC_BOTTLER_H_

#include "stm32f4xx_hal.h" // Incluimos la librería de HAL en lugar de bottler.h

// AVISAMOS de que esta variable existe (se creará en bottler.c)
extern volatile uint8_t forward;

void bottlerInit(void);
void conveyorForward(void);
void conveyorStop(void);
void conveyorReverse(void);
void valveOpen(void);
void valveClose(void);
float getTemperature(void);


#endif /* INC_BOTTLER_H_ */




