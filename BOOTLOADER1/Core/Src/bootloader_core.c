/*
 * bootloader_core.c
 *
 * Vendor-agnostic bootloader state machine.
 * All hardware access goes through the Bootloader_Interface_t.
 *
 *  Created on: Feb 17, 2026
 *      Author: mertk
 */

#include "bootloader_core.h"
#include "bootloader_config.h"
#include "BL_Functions.h"
#include "Cryptology_Control.h"
#include "firmware_footer.h"
#include "tiny_printf.h"

void Bootloader_Run(const Bootloader_Interface_t *sys) {
    BootConfig_t config;
    const BL_MemoryMap_t *mem = &sys->mem;

    sys->Init();
    BL_SetInterface(sys);

    printf("\r\n========================================\r\n");
    printf("Starting Bootloader Version-(%d,%d)\r\n", 1, 7);
    printf("========================================\r\n");

    if (BL_ReadConfig(&config)) {
        printf("[BL] Config Invalid/Empty. Initialized to Defaults.\r\n");
        BL_WriteConfig(&config);
    }

    /* Check User Button */
    if (sys->GPIO_ReadUserButton() == 1) {
        printf("[BL] Button Pressed! Determining Mode...\r\n");

        FW_Status_t status = Firmware_Is_Valid(mem->app_download_addr, mem->slot_size, &sys->crypto);

        if (status == BL_OK) {
            printf(" -> Valid Footer Found. Requesting UPDATE.\r\n");
            config.system_status = STATE_UPDATE_REQ;
        } else {
            uint32_t *s6_ptr = (uint32_t *)mem->app_download_addr;
            if (*s6_ptr == 0xFFFFFFFF) {
                printf(" -> Download Slot is Empty. Cannot Swap.\r\n");
                config.system_status = STATE_NORMAL;
            } else {
                printf(" -> Download Slot has data (Backup). Requesting SWAP/ROLLBACK.\r\n");
                config.system_status = STATE_ROLLBACK;
            }
        }
    }

    /* State Machine */
    switch (config.system_status) {
        case STATE_UPDATE_REQ:
            printf("[BL] State: UPDATE REQUESTED.\r\n");
            BL_Swap_NoBuffer();

            printf("[BL] Update Process Finished/Failed. Clearing state.\r\n");
            config.system_status = STATE_NORMAL;
            BL_WriteConfig(&config);
            break;

        case STATE_ROLLBACK:
            if (BL_Rollback() != BL_OK) {
                printf("[BL] Rollback Failed. Reverting state to NORMAL.\r\n");
                config.system_status = STATE_NORMAL;
                BL_WriteConfig(&config);
                sys->SystemReset();
            }
            /* fall through on success (SystemReset called inside BL_Rollback) */

        case STATE_NORMAL:
        default: {
            printf("[BL] State: NORMAL. Checking Active Application (S5)...\r\n");

            uint32_t *app_reset_vector = (uint32_t *)(mem->app_active_addr + 4);
            uint32_t app_entry_point = *app_reset_vector;

            if (app_entry_point > mem->app_active_addr &&
                app_entry_point < (mem->app_active_addr + mem->slot_size))
            {
                printf("[BL] Valid App found at 0x%X. Jumping...\r\n",
                       (unsigned int)mem->app_active_addr);
                sys->JumpToApp();
            } else {
                printf("[BL] S5 Empty or Invalid! Checking S6 for Auto-Provisioning...\r\n");

                if (Firmware_Is_Valid(mem->app_download_addr, mem->slot_size, &sys->crypto) == BL_OK) {
                    printf("[BL] Valid Image found in S6! Triggering Update...\r\n");
                    config.system_status = STATE_UPDATE_REQ;
                    BL_WriteConfig(&config);
                    sys->SystemReset();
                } else {
                    printf("[ERROR] No valid app in S5, and no update in S6.\r\n");
                    printf("[ERROR] System Halted.\r\n");
                    sys->ErrorHandler();
                }
            }
            break;
        }
    }
}
