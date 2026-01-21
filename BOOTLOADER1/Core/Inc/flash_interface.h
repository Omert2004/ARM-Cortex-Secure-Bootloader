/*
 * flash_interface.h
 *
 *  Created on: Jan 21, 2026
 *      Author: mertk
 */

#ifndef INC_FLASH_INTERFACE_H_
#define INC_FLASH_INTERFACE_H_

#include <stdint.h>

typedef enum {
    FLASH_OK,
    FLASH_ERR_LOCKED,
    FLASH_ERR_WRITE,
    FLASH_ERR_ERASE,
    FLASH_ERR_PARAM,
	FLASH_ERR_TIMEOUT
} Flash_Status_t;

Flash_Status_t Flash_Interface_Init(void);
Flash_Status_t Flash_Interface_Lock(void);
Flash_Status_t Flash_Interface_Unlock(void);

Flash_Status_t Flash_Interface_Erase(uint32_t startAddress, uint32_t length);
Flash_Status_t Flash_Interface_Write(uint32_t address, const uint8_t *data, uint32_t length);

#endif /* INC_FLASH_INTERFACE_H_ */
