/*
 * crypto_driver_sw.h
 *
 * Software cryptographic operations backed by TinyCrypt.
 * Platform-independent — any MCU can use these as a baseline.
 * MCUs with hardware accelerators can replace individual ops.
 *
 *  Created on: Feb 18, 2026
 *      Author: mertk
 */

#ifndef INC_CRYPTO_DRIVER_SW_H_
#define INC_CRYPTO_DRIVER_SW_H_

#include <stdint.h>

int SW_AES_EncryptBlock(const uint8_t key[16], const uint8_t in[16], uint8_t out[16]);
int SW_AES_DecryptBlock(const uint8_t key[16], const uint8_t in[16], uint8_t out[16]);
int SW_SHA256(const uint8_t *data, uint32_t len, uint8_t digest[32]);
int SW_ECDSA_Verify(const uint8_t *pub_key, const uint8_t *hash,
                    uint32_t hash_len, const uint8_t *sig);

#endif /* INC_CRYPTO_DRIVER_SW_H_ */
