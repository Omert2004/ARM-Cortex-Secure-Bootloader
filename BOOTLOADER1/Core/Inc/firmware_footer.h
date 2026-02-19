/*
 * firmware_footer.h
 *
 *  Created on: Dec 2, 2025
 *      Author: mertk
 */

#ifndef INC_FIRMWARE_FOOTER_H_
#define INC_FIRMWARE_FOOTER_H_

#include <stdint.h>

/* Magic marker — bootloader scans backwards to find this */
#define FOOTER_MAGIC 0x454E4421  /* ASCII "END!" */

/* Firmware validation status codes */
typedef enum {
    BL_OK = 0,
    BL_ERR_FOOTER_NOT_FOUND,
    BL_ERR_FOOTER_BAD,
    BL_ERR_IMAGE_SIZE_BAD,
    BL_ERR_IMAGE_RANGE_BAD,
    BL_ERR_VECTOR_BAD,
    BL_ERR_HASH_FAIL,
    BL_ERR_SIG_FAIL,
} FW_Status_t;

/* Footer appended after the encrypted firmware payload */
typedef struct {
    uint32_t version;       /* Firmware Version                      */
    uint32_t size;          /* Payload Size (IV + Encrypted Data)    */
    uint8_t  signature[64]; /* ECDSA Signature (r + s, 32 bytes each)*/
    uint32_t magic;         /* FOOTER_MAGIC                          */
} fw_footer_t;

#endif /* INC_FIRMWARE_FOOTER_H_ */
