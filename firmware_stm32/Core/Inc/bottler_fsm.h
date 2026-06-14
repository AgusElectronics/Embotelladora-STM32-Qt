/*
 * bottler_fsm.h
 *
 * Created on: 14 may 2026
 * Author: agust
 */

#ifndef INC_BOTTLER_FSM_H_
#define INC_BOTTLER_FSM_H_

#include "bottler.h"

// AVISAMOS de que esta variable existe (se creará en bottler_fsm.c)
extern volatile uint8_t estado;

void MaquinaEstados(void);

#endif /* INC_BOTTLER_FSM_H_ */
