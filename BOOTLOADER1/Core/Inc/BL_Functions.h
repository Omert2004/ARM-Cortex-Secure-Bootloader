/*
 * BL_Functions.h
 *
 *  Created on: 27 Ara 2025
 *      Author: Oguzm
 */

#ifndef INC_BL_FUNCTIONS_H_
#define INC_BL_FUNCTIONS_H_

#include "bootloader_config.h"
#include "system_interface.h"

void BL_SetInterface(const Bootloader_Interface_t *iface);

uint8_t BL_ReadConfig(BootConfig_t *cfg);
uint8_t BL_WriteConfig(BootConfig_t *cfg);

void BL_Swap_NoBuffer(void);
uint8_t BL_Rollback(void);

#endif /* INC_BL_FUNCTIONS_H_ */
