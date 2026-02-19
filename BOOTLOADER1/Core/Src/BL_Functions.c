/**
 * @file    BL_Functions.c
 * @brief   Implementation of core Bootloader logic.
 * @details Handles configuration management, AES encryption/decryption (CBC/ECB),
 * and the orchestration of Firmware Updates and Rollbacks.
 *
 * This file is fully portable — all hardware access goes through
 * Bootloader_Interface_t and all crypto through BL_CryptoOps_t.
 *
 * @version 2.0
 */

#include "BL_Functions.h"
#include "keys.h"
#include "tiny_printf.h"
#include "Cryptology_Control.h"
#include "firmware_footer.h"
#include <string.h>

extern const uint8_t AES_SECRET_KEY[];

static const Bootloader_Interface_t *sys = NULL;

void BL_SetInterface(const Bootloader_Interface_t *iface) {
    sys = iface;
}

/* ========================================================================== */
/* CONFIGURATION                                                              */
/* ========================================================================== */

uint8_t BL_ReadConfig(BootConfig_t *cfg) {
    memcpy(cfg, (void *)sys->mem.config_addr, sizeof(BootConfig_t));

    if (cfg->magic_number != CONFIG_MAGIC) {
        cfg->magic_number    = CONFIG_MAGIC;
        cfg->system_status   = STATE_NORMAL;
        cfg->current_version = 0;
        return 1;
    }
    return 0;
}

uint8_t BL_WriteConfig(BootConfig_t *cfg) {
    if (sys->Flash_Erase(sys->mem.config_addr, sizeof(BootConfig_t)) != 0)
        return 0;

    if (sys->Flash_Write(sys->mem.config_addr, (uint8_t *)cfg, sizeof(BootConfig_t)) != 0)
        return 0;

    return 1;
}

/* ========================================================================== */
/* INTERNAL HELPERS                                                           */
/* ========================================================================== */

static uint8_t BL_Raw_Copy(uint32_t src_addr, uint32_t dest_addr, uint32_t size) {
    printf("  [DEBUG] Erasing Target Area... ");
    if (sys->Flash_Erase(dest_addr, size) != 0) {
        printf("FAILED! Erase Error.\r\n");
        return 0;
    }
    printf("OK\r\n");

    printf("  [DEBUG] Writing %d bytes... ", (int)size);
    if (sys->Flash_Write(dest_addr, (uint8_t *)src_addr, size) != 0) {
        printf("FAILED! Write Error.\r\n");
        return 0;
    }
    printf("OK\r\n");
    return 1;
}

/* ========================================================================== */
/* CRYPTOGRAPHIC OPERATIONS                                                   */
/* ========================================================================== */

/**
 * @brief  Decrypts a new update image using AES-128-CBC.
 * @param  src_slot_addr Start address of the encrypted image (IV + ciphertext).
 * @param  dest_addr     Destination address (Scratchpad).
 * @param  payload_size  Total size of IV + Ciphertext.
 * @retval 1 on success, 0 on failure.
 */
static uint8_t BL_Decrypt_Update_Image(uint32_t src_slot_addr,
                                        uint32_t dest_addr,
                                        uint32_t payload_size) {
    uint8_t buffer_enc[16], buffer_dec[16], current_iv[16];

    memcpy(current_iv, (void *)src_slot_addr, 16);

    uint32_t encrypted_data_size = payload_size - 16;
    uint32_t data_start_offset   = 16;

    printf("  [DEBUG] Erasing Scratchpad Area... ");
    if (sys->Flash_Erase(dest_addr, encrypted_data_size) != 0) {
        printf("FAILED! Erase Error.\r\n");
        return 0;
    }
    printf("OK\r\n");

    for (uint32_t i = 0; i < encrypted_data_size; i += 16) {
        memcpy(buffer_enc, (void *)(src_slot_addr + data_start_offset + i), 16);

        /* AES-CBC: decrypt block, then XOR with previous ciphertext (IV) */
        if (sys->crypto.AES_DecryptBlock(AES_SECRET_KEY, buffer_enc, buffer_dec) != 0)
            return 0;

        for (int j = 0; j < 16; j++)
            buffer_dec[j] ^= current_iv[j];
        memcpy(current_iv, buffer_enc, 16);

        if (sys->Flash_Write(dest_addr + i, buffer_dec, 16) != 0) {
            printf("FAILED! Write Error at offset %d.\r\n", (int)i);
            return 0;
        }
    }
    return 1;
}

/**
 * @brief  Encrypts the active application using AES-128-ECB for backup.
 * @param  src_addr  Source address (Active App).
 * @param  dest_addr Destination address (Backup Slot).
 * @retval 1 on success, 0 on failure.
 */
static uint8_t BL_Encrypt_Backup(uint32_t src_addr, uint32_t dest_addr) {
    uint8_t buffer_plain[16];
    uint8_t buffer_enc[16];
    uint32_t slot_size = sys->mem.slot_size;

    sys->DisableIRQ();

    printf("  [DEBUG] Erasing Backup Area... ");
    if (sys->Flash_Erase(dest_addr, slot_size) != 0) {
        sys->EnableIRQ();
        printf("FAILED! Erase Error.\r\n");
        return 0;
    }
    printf("OK\r\n");

    printf("  [DEBUG] Encrypting & Backing up %d bytes... \r\n", (int)slot_size);

    for (uint32_t offset = 0; offset < slot_size; offset += 16) {
        memcpy(buffer_plain, (void *)(src_addr + offset), 16);

        if (sys->crypto.AES_EncryptBlock(AES_SECRET_KEY, buffer_plain, buffer_enc) != 0) {
            sys->EnableIRQ();
            return 0;
        }

        if (sys->Flash_Write(dest_addr + offset, buffer_enc, 16) != 0) {
            sys->EnableIRQ();
            printf("Backup Write Failed at offset 0x%x\r\n", (unsigned int)offset);
            return 0;
        }
    }

    sys->EnableIRQ();
    printf("  Backup Complete.\r\n");
    return 1;
}

/**
 * @brief  Decrypts a backup image using AES-128-ECB for rollback.
 * @param  src_addr  Source address (Backup Slot).
 * @param  dest_addr Destination address (Scratchpad).
 * @retval 1 on success, 0 on failure.
 */
static uint8_t BL_Decrypt_Backup_Image(uint32_t src_addr, uint32_t dest_addr) {
    uint8_t buffer_enc[16];
    uint8_t buffer_dec[16];
    uint32_t slot_size = sys->mem.slot_size;

    sys->DisableIRQ();

    if (sys->Flash_Erase(dest_addr, slot_size) != 0) {
        sys->EnableIRQ();
        return 0;
    }

    for (uint32_t offset = 0; offset < slot_size; offset += 16) {
        memcpy(buffer_enc, (void *)(src_addr + offset), 16);

        if (sys->crypto.AES_DecryptBlock(AES_SECRET_KEY, buffer_enc, buffer_dec) != 0) {
            sys->EnableIRQ();
            return 0;
        }

        if (sys->Flash_Write(dest_addr + offset, buffer_dec, 16) != 0) {
            sys->EnableIRQ();
            return 0;
        }
    }

    sys->EnableIRQ();
    return 1;
}

/* ========================================================================== */
/* FIRMWARE UPDATE & ROLLBACK                                                 */
/* ========================================================================== */

void BL_Swap_NoBuffer(void) {
    BootConfig_t cfg;
    const BL_MemoryMap_t *mem = &sys->mem;

    BL_ReadConfig(&cfg);

    uint32_t footer_addr = Find_Footer_Address(mem->app_download_addr, mem->slot_size);
    if (footer_addr == 0) {
        printf("Error: No Footer found in S6.\r\n");
        cfg.system_status = STATE_NORMAL;
        BL_WriteConfig(&cfg);
        return;
    }

    fw_footer_t footer;
    memcpy(&footer, (void *)footer_addr, sizeof(fw_footer_t));

    printf("[BL] Verifying Signature... ");
    FW_Status_t status = Firmware_Is_Valid(mem->app_download_addr, mem->slot_size, &sys->crypto);

    if (status != BL_OK) {
        printf("FAIL! Error Code: %d\r\n", status);

        sys->Flash_Erase(mem->app_download_addr, mem->slot_size);

        if (status == BL_ERR_SIG_FAIL)
            printf("Reason: ECDSA Signature Mismatch.\r\n");
        if (status == BL_ERR_FOOTER_NOT_FOUND)
            printf("Reason: Footer Missing.\r\n");

        cfg.system_status = STATE_NORMAL;
        BL_WriteConfig(&cfg);
        return;
    }
    printf("OK!\r\n");
    printf("[BL] Valid Update! Ver: %d, Payload: %d\r\n",
           (int)footer.version, (int)footer.size);

    printf("[1/3] Decrypting S6 -> S7...\r\n");
    if (!BL_Decrypt_Update_Image(mem->app_download_addr, mem->scratch_addr, footer.size)) {
        printf("Error: Decryption Failed.\r\n");
        return;
    }

    printf("[2/3] Backing up S5 -> S6...\r\n");
    if (!BL_Encrypt_Backup(mem->app_active_addr, mem->app_download_addr)) {
        printf("Error: Backup Failed.\r\n");
        return;
    }

    printf("[3/3] Installing S7 -> S5...\r\n");
    if (!BL_Raw_Copy(mem->scratch_addr, mem->app_active_addr, mem->slot_size)) {
        printf("Error: Installation Failed.\r\n");
        return;
    }

    printf("[BL] Update Successful! Setting State to NORMAL.\r\n");
    cfg.system_status   = STATE_NORMAL;
    cfg.current_version = footer.version;
    BL_WriteConfig(&cfg);

    printf("Swap Complete. Resetting...\r\n");
    sys->SystemReset();
}

uint8_t BL_Rollback(void) {
    BootConfig_t cfg;
    const BL_MemoryMap_t *mem = &sys->mem;

    BL_ReadConfig(&cfg);

    printf("\r\n[BL] Starting Rollback/Toggle...\r\n");

    printf("[1/3] Decrypting Backup (S6 -> S7)...\r\n");
    if (!BL_Decrypt_Backup_Image(mem->app_download_addr, mem->scratch_addr)) {
        printf("Error: Rollback Decryption Failed.\r\n");
        return 1;
    }

    uint32_t *pDecryptedData = (uint32_t *)mem->scratch_addr;
    uint32_t resetVector = pDecryptedData[1];

    if ((resetVector & 0xFF000000) != (mem->flash_base & 0xFF000000)) {
        printf("[ERROR] The Backup in S6 is Empty or Invalid!\r\n");
        printf("[ERROR] Reset Vector: 0x%08X. Aborting Swap to protect Active App.\r\n",
               (unsigned int)resetVector);

        cfg.system_status = STATE_NORMAL;
        BL_WriteConfig(&cfg);
        return 2;
    }

    printf("[2/3] Backing up Current App (S5 -> S6)...\r\n");
    if (!BL_Encrypt_Backup(mem->app_active_addr, mem->app_download_addr)) {
        printf("Error: Backup Failed.\r\n");
        return 3;
    }

    printf("[3/3] Restoring Old App (S7 -> S5)...\r\n");
    if (!BL_Raw_Copy(mem->scratch_addr, mem->app_active_addr, mem->slot_size)) {
        printf("Error: Installation Failed.\r\n");
        return 4;
    }

    printf("[BL] Rollback Successful! Resetting...\r\n");
    cfg.system_status = STATE_NORMAL;
    BL_WriteConfig(&cfg);
    sys->SystemReset();
    return BL_OK;
}
