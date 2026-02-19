/**
 * @file    Cryptology_Control.c
 * @brief   Firmware Verification and Footer Management.
 * @details Locates the firmware footer in Flash, hashes the payload,
 * and verifies the ECDSA signature — all through BL_CryptoOps_t.
 */

#include "Cryptology_Control.h"
#include "firmware_footer.h"
#include "keys.h"

extern const uint8_t ECDSA_public_key_xy[];

/**
 * @brief  Scans the Flash slot backwards to locate the Firmware Footer.
 * @param  slot_start Start address of the Flash sector/slot.
 * @param  slot_size  Size of the slot in bytes.
 * @retval Address of the fw_footer_t structure, or 0 if not found.
 */
uint32_t Find_Footer_Address(uint32_t slot_start, uint32_t slot_size)
{
    uint32_t slot_end = slot_start + slot_size;

    for (uint32_t addr = slot_end - 4; addr >= slot_start; addr -= 4)
    {
        if (*(uint32_t *)addr == FOOTER_MAGIC)
        {
            uint32_t footer_start = addr - (sizeof(fw_footer_t) - 4);
            if (footer_start < slot_start) continue;
            return footer_start;
        }
    }
    return 0;
}

/**
 * @brief  Validates the integrity and authenticity of a firmware image.
 * @param  start_addr Start address of the image in Flash.
 * @param  slot_size  Maximum size of the slot.
 * @param  crypto     Pointer to the crypto operations (SHA-256, ECDSA).
 * @retval BL_OK on success, or specific error code on failure.
 */
FW_Status_t Firmware_Is_Valid(uint32_t start_addr, uint32_t slot_size,
                              const BL_CryptoOps_t *crypto)
{
    uint32_t footer_addr = Find_Footer_Address(start_addr, slot_size);
    if (footer_addr == 0)
        return BL_ERR_FOOTER_NOT_FOUND;

    fw_footer_t *footer = (fw_footer_t *)footer_addr;

    if (footer->size > slot_size)
        return BL_ERR_IMAGE_SIZE_BAD;

    /* Hash the payload */
    uint8_t digest[32];
    if (crypto->SHA256((uint8_t *)start_addr, footer->size, digest) != 0)
        return BL_ERR_HASH_FAIL;

    /* Verify ECDSA signature */
    if (crypto->ECDSA_Verify(ECDSA_public_key_xy, digest, 32, footer->signature) != 0)
        return BL_ERR_SIG_FAIL;

    return BL_OK;
}
