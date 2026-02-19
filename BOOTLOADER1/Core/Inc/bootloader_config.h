/*
 * bootloader_config.h
 *
 *  Created on: 31 Ara 2025
 *      Author: Oguzm
 *
 * Portable bootloader configuration types.
 * Shared between Bootloader and Application — no hardware dependencies.
 */

#ifndef INC_BOOTLOADER_CONFIG_H_
#define INC_BOOTLOADER_CONFIG_H_

#include <stdint.h>

/* Magic number written to config sector to mark it as valid */
#define CONFIG_MAGIC  0xDEADBEEF

/* System States */
typedef enum {
    STATE_NORMAL     = 4,
    STATE_UPDATE_REQ = 5,
    STATE_ROLLBACK   = 6
} BL_System_Status_t;

/* Persistent boot configuration (stored in flash config sector) */
typedef struct {
    uint32_t magic_number;    /* CONFIG_MAGIC when valid                 */
    uint32_t system_status;   /* BL_System_Status_t                     */
    uint32_t current_version; /* Currently running firmware version      */
} BootConfig_t;

#endif /* INC_BOOTLOADER_CONFIG_H_ */
