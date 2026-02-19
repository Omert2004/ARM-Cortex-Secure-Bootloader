/*
 * crypto_driver_sw.c
 *
 * Software cryptographic operations backed by TinyCrypt.
 * Platform-independent — reusable on any MCU.
 *
 * Return convention: 0 = success, -1 = error.
 *
 * The AES key schedule is cached so that repeated calls with the
 * same key (e.g. 16K blocks during a firmware decrypt) do not
 * re-compute the schedule on every block.
 *
 *  Created on: Feb 18, 2026
 *      Author: mertk
 */

#include "crypto_driver_sw.h"
#include "aes.h"
#include "sha256.h"
#include "ecc_dsa.h"
#include <string.h>

/* Cached key schedules — avoids recomputing for every block */
static struct tc_aes_key_sched_struct enc_sched;
static uint8_t enc_cached_key[16];
static uint8_t enc_key_valid = 0;

static struct tc_aes_key_sched_struct dec_sched;
static uint8_t dec_cached_key[16];
static uint8_t dec_key_valid = 0;

int SW_AES_EncryptBlock(const uint8_t key[16], const uint8_t in[16], uint8_t out[16]) {
    if (!enc_key_valid || memcmp(key, enc_cached_key, 16) != 0) {
        if (tc_aes128_set_encrypt_key(&enc_sched, key) == 0)
            return -1;
        memcpy(enc_cached_key, key, 16);
        enc_key_valid = 1;
    }

    if (tc_aes_encrypt(out, in, &enc_sched) == 0)
        return -1;

    return 0;
}

int SW_AES_DecryptBlock(const uint8_t key[16], const uint8_t in[16], uint8_t out[16]) {
    if (!dec_key_valid || memcmp(key, dec_cached_key, 16) != 0) {
        if (tc_aes128_set_decrypt_key(&dec_sched, key) == 0)
            return -1;
        memcpy(dec_cached_key, key, 16);
        dec_key_valid = 1;
    }

    if (tc_aes_decrypt(out, in, &dec_sched) == 0)
        return -1;

    return 0;
}

int SW_SHA256(const uint8_t *data, uint32_t len, uint8_t digest[32]) {
    struct tc_sha256_state_struct state;

    if (tc_sha256_init(&state) != 1)
        return -1;
    if (tc_sha256_update(&state, data, len) != 1)
        return -1;
    if (tc_sha256_final(digest, &state) != 1)
        return -1;

    return 0;
}

int SW_ECDSA_Verify(const uint8_t *pub_key, const uint8_t *hash,
                    uint32_t hash_len, const uint8_t *sig) {
    if (uECC_verify(pub_key, hash, hash_len, sig, uECC_secp256r1()) == 1)
        return 0;

    return -1;
}
