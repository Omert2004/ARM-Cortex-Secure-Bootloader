/*
 * system_interface.h
 *
 *  Created on: Jan 28, 2026
 *      Author: mertk
 *
 * Generic hardware abstraction interface for the Bootloader.
 * Allows decoupling bootloader logic from specific MCU implementations.
 *
 * To port to a new MCU:
 *   1. Populate BL_MemoryMap_t with your device's flash/RAM layout.
 *   2. Populate BL_CryptoOps_t — use software (crypto_driver_sw) or hardware.
 *   3. Implement all remaining function pointers (flash, GPIO, etc.).
 */

#ifndef INC_SYSTEM_INTERFACE_H_
#define INC_SYSTEM_INTERFACE_H_

#include <stdint.h>

/* Memory layout descriptor — populated by the platform driver */
typedef struct {
    uint32_t config_addr;       /* Boot config sector start address       */
    uint32_t app_active_addr;   /* Active application slot  (e.g. S5)    */
    uint32_t app_download_addr; /* Download / backup slot   (e.g. S6)    */
    uint32_t scratch_addr;      /* Scratch / buffer slot    (e.g. S7)    */
    uint32_t slot_size;         /* Size of each application slot (bytes) */
    uint32_t flash_base;        /* Flash base address (for vector check) */
    uint32_t ram_base;          /* RAM  base address (for SP validation) */
} BL_MemoryMap_t;

/*
 * Cryptographic operations — can be backed by software (TinyCrypt)
 * or hardware accelerators (CRYP, HASH peripherals, etc.).
 * Return convention: 0 = success, non-zero = error.
 */
typedef struct {
    int (*AES_EncryptBlock)(const uint8_t key[16], const uint8_t in[16], uint8_t out[16]);
    int (*AES_DecryptBlock)(const uint8_t key[16], const uint8_t in[16], uint8_t out[16]);
    int (*SHA256)(const uint8_t *data, uint32_t len, uint8_t digest[32]);
    int (*ECDSA_Verify)(const uint8_t *pub_key, const uint8_t *hash,
                        uint32_t hash_len, const uint8_t *sig);
} BL_CryptoOps_t;

/* Hardware abstraction — every platform must provide these */
typedef struct {
    /* Memory Layout */
    BL_MemoryMap_t  mem;

    /* Cryptography */
    BL_CryptoOps_t  crypto;

    /* System Control */
    void     (*Init)(void);
    void     (*DeInit)(void);
    void     (*SystemReset)(void);
    void     (*Delay)(uint32_t ms);
    uint32_t (*GetTick)(void);

    /* Communications */
    void     (*UART_Write)(const uint8_t *data, uint16_t size);

    /* GPIO */
    uint8_t  (*GPIO_ReadUserButton)(void);
    void     (*GPIO_ToggleLed)(void);

    /* Flash */
    void     (*Flash_Unlock)(void);
    void     (*Flash_Lock)(void);
    int      (*Flash_Erase)(uint32_t address, uint32_t length);
    int      (*Flash_Write)(uint32_t address, const uint8_t *data, uint32_t length);

    /* Critical */
    void     (*ErrorHandler)(void);
    void     (*JumpToApp)(void);
    void     (*DisableIRQ)(void);
    void     (*EnableIRQ)(void);
} Bootloader_Interface_t;

/* Platform driver must implement this to return its interface */
const Bootloader_Interface_t* Sys_GetInterface(void);

#endif /* INC_SYSTEM_INTERFACE_H_ */
