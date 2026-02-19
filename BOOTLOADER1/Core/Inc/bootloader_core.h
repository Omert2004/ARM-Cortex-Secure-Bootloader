/*
 * bootloader_core.h
 *
 * Vendor-agnostic bootloader state machine.
 * Receives all hardware access through Bootloader_Interface_t.
 *
 *  Created on: Feb 17, 2026
 *      Author: mertk
 */

#ifndef INC_BOOTLOADER_CORE_H_
#define INC_BOOTLOADER_CORE_H_

#include "system_interface.h"

void Bootloader_Run(const Bootloader_Interface_t *sys);

#endif /* INC_BOOTLOADER_CORE_H_ */
