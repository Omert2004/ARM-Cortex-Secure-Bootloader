--- START OF FILE: STM32_Development_Standard.md ---

# STM32 Secure Bootloader: Professional Development Standard

This document establishes the official coding and architectural standards for the STM32F7 Secure Bootloader project. It is designed to ensure compliance with **MISRA C:2012**, improve codebase maintainability, and enforce high-security data handling.

## Table of Contents

1. [Project Architecture & Layering](https://www.google.com/search?q=%231-project-architecture--layering)
2. [Naming Conventions: Locality & Size](https://www.google.com/search?q=%232-naming-conventions-locality--size)
3. [General Identifier Rules](https://www.google.com/search?q=%233-general-identifier-rules)
4. [Security-Specific Naming](https://www.google.com/search?q=%234-security-specific-naming)
5. [Defensive Programming & MISRA Compliance](https://www.google.com/search?q=%235-defensive-programming--misra-compliance)
6. [Documentation Standard](https://www.google.com/search?q=%236-documentation-standard)
7. [Verification & Static Analysis](https://www.google.com/search?q=%237-verification--static-analysis)

---

## 1. Project Architecture & Layering

To ensure modularity and prevent security leakage, the project follows a strict layered approach. No higher-layer module should access hardware registers directly.

* **App Layer (`/App`):** High-level bootloader state machine, update logic, and version anti-rollback checks.
* **Service Layer (`/Services`):** Cryptographic wrappers (AES/ECDSA), hashing engines, and firmware image parsing.
* **Driver Layer (`/Drivers/Custom`):** Board-specific drivers for external components such as QSPI Flash or LCDs.
* **Hardware Layer (`/Drivers/HAL`):** STM32 HAL/LL and CMSIS libraries provided by ST.

---

## 2. Naming Conventions: Locality & Size

Identifiers must communicate their **Storage Class** (locality) and their **Data Width** (size). This allows developers to understand memory footprint and scope at a glance.

### 2.1 Locality (Scope) Prefixes

| Prefix | Scope | Storage Location | Example |
| --- | --- | --- | --- |
| **`g_`** | Global | SRAM (Data/BSS) | `g_systemTick_u32` |
| **`s_`** | Static (File) | SRAM (Data/BSS) | `s_isDriverInit_b` |
| **`m_`** | Member | Struct/Class Member | `m_payloadLen_u16` |
| **`v_`** | Volatile | Shared RAM/Registers | `v_isDmaDone_b` |
| **`p_`** | Parameter | Stack (Function Arg) | `p_inputBuf_u8` |

### 2.2 Size & Type Tags

| Tag | Type | Size | Example |
| --- | --- | --- | --- |
| **`u8`** | `uint8_t` | 8-bit Unsigned | `status_u8` |
| **`u16`** | `uint16_t` | 16-bit Unsigned | `timeout_u16` |
| **`u32`** | `uint32_t` | 32-bit Unsigned | `address_u32` |
| **`i32`** | `int32_t` | 32-bit Signed | `temp_i32` |
| **`b`** | `bool` | 1-bit / Logic | `isLocked_b` |

---

## 3. General Identifier Rules

* **Public Functions:** Use `Module_ActionName` (e.g., `FLS_WritePage`).
* **Private Functions:** Use `static action_name` (e.g., `static verify_crc`).
* **Constants/Macros:** Use `ALL_CAPS` (e.g., `MAX_FLASH_PAGES`).
* **MISRA Rule 21.1:** Do not start any identifier with an underscore (`_`) or double underscore.
* **Units:** Always include units for physical quantities (e.g., `delay_ms`, `voltage_mv`).

---

## 4. Security-Specific Naming

To prevent logic errors in the bootloader, naming must reflect the **Trust Level** and **Encryption State** of the data.

### 4.1 Trust Boundary Prefixes

* **`raw_`**: Unvalidated data from external interfaces (e.g., `raw_uart_rx_buf`).
* **`enc_`**: Encrypted/Ciphertext data (e.g., `enc_firmware_img`).
* **`dec_`**: Decrypted/Plaintext data — **Sensitive** (e.g., `dec_firmware_img`).
* **`vetted_`**: Data that has passed signature verification — **Trusted**.

### 4.2 Key & Slot Management

* **Keys:** Distinguish between `KEY_PUB_ROOT` (stored in Flash) and `KEY_PRIV_SESSION` (temporary in RAM).
* **Slots:** Explicitly name memory regions (e.g., `SLOT_ACTIVE_START_ADDR`, `SLOT_DWLD_START_ADDR`).

---

## 5. Defensive Programming & MISRA Compliance

### 5.1 Type Safety (Rule 4.6)

Standard C types (`int`, `char`, `long`) are prohibited. Use `<stdint.h>` types exclusively to ensure 32-bit portability across ARM Cortex-M7 architectures.

### 5.2 Constant Correctness (Rule 8.13)

If a function reads but does not modify data, the parameter **must** be `const`. Use the `pc_` prefix (Pointer to Const).

* *Correct:* `int8_t SEC_Verify(const uint8_t *pc_data_u8);`

### 5.3 Secure Memory Wipe

Upon completion of a secure operation, sensitive buffers (marked `dec_` or `KEY_`) must be zeroed immediately using a volatile-safe memset to prevent memory-dump attacks.

---

## 6. Documentation Standard

Every public function must include a Doxygen block in the header (`.h`) file to enable IntelliSense support in STM32CubeIDE.

```c
/**
 * @brief  Verifies the ECDSA signature of a firmware block.
 * @param  pc_data_u8: Pointer to the constant decrypted firmware data.
 * @param  pc_sig_u8:  Pointer to the constant signature buffer.
 * @retval 0 if valid, -1 if invalid, -2 if hardware error.
 */
int8_t SEC_VerifySig(const uint8_t *pc_data_u8, const uint8_t *pc_sig_u8);

```

---

## 7. Verification & Static Analysis

This project requires a 100% pass rate on the following static analysis configuration:

* **Tool:** Cppcheck
* **Flags:** `--enable=all --misra-config=misra_rules.json --std=c99`
* **GCC Warning Level:** `-Wall -Wextra -Werror -Wstrict-prototypes`
* **Naming Enforcer:** Any identifier failing the [Locality & Size](https://www.google.com/search?q=%232-naming-conventions-locality--size) rules will be rejected during code review.

--- END OF FILE: STM32_Development_Standard.md ---
