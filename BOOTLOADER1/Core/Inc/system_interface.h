/*
 * system_inteface.h
 *
 *  Created on: Jan 28, 2026
 *      Author: mertk
 */

#ifndef INC_SYSTEM_INTERFACE_H_
#define INC_SYSTEM_INTERFACE_H_

#include <stdint.h>

void System_Interface_Init(void);

void System_Interface_Reset(void);

void System_Interface_Delay(uint32_t ms);

uint32_t System_Interface_GetTick(void);
void System_Interface_Error_Handler(void);
void Error_Handler(void);
uint8_t System_Interface_ReadUserButton(void);
void System_Interface_ToggleHeartbeatLed(void);
#endif /* INC_SYSTEM_INTERFACE_H_ */
