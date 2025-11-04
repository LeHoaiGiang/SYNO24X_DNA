/*
 * global_extern.h
 *
 *  Created on: Sep 2, 2024
 *      Author: LeHoaiGiang
 */

#ifndef GLOBAL_EXTERN_H_
#define GLOBAL_EXTERN_H_
#include "macro.h"
#include "stdbool.h"
#include "struct.h"
extern GPIO_Config GPIO_Extend[];
extern GPIO_Config SOLENOID[8];
extern volatile int Fill_Position_X[17][3];
#endif /* GLOBAL_EXTERN_H_ */
