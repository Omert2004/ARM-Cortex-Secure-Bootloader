# ARM Cortex Secure Bootloader — Porting Guide

The bootloader logic is designed to be hardware-agnostic. All MCU-specific code lives in
a single platform driver file.

---

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    PORTABLE LAYER                       │
│   bootloader_core.c  →  BL_Functions.c                  │
│                      →  Cryptology_Control.c            │
│                                                         │
│   All hardware access via:  sys->mem.*                  │
│                             sys->crypto.*               │
│                             sys->Flash_*() / JumpToApp()│
├─────────────────────────────────────────────────────────┤
│                   system_interface.h                    │
│     BL_MemoryMap_t  +  BL_CryptoOps_t  +  fn pointers   │
├────────────────────────┬────────────────────────────────┤
│  system_driver_<mcu>.c │  crypto_driver_sw.c            │
│  (your platform code)  │  (TinyCrypt — any platform)    │
└────────────────────────┴────────────────────────────────┘
```

---

## Step 1 — Memory Map

Edit `Core/Inc/mem_layout.h` with your MCU's flash addresses:

```c
#define CONFIG_SECTOR_ADDR       0x08010000
#define APP_ACTIVE_START_ADDR    0x08040000
#define APP_DOWNLOAD_START_ADDR  0x08080000
#define SCRATCH_ADDR             0x080C0000
#define SLOT_SIZE                0x00040000
```

**Rules:**
- All three slots (active, download, scratch) must be the same `SLOT_SIZE`.
- Config sector must be in its own erasable sector, below all three slots.
- The bootloader itself must fit below `CONFIG_SECTOR_ADDR`.

---
## Step 2 — Linker Scripts & Vector Table

For the bootloader to successfully hand off control to the Main Application, both the Bootloader and the Application must be compiled with strict memory boundaries that match `mem_layout.h`.

### 1. The Bootloader Linker Script
The bootloader must be restricted so it does not accidentally overwrite the Config Sector or the Application slots. 

In your bootloader's `.ld` file (e.g., `STM32F746XX_FLASH.ld`), limit the `FLASH` length to fit exactly below `CONFIG_SECTOR_ADDR`:
```ld
/* Example: Bootloader gets the first 64KB */
MEMORY
{
  RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 320K
  FLASH (rx)     : ORIGIN = 0x08000000, LENGTH = 64K 
}
```

### 2. The Application Linker Script
The Main Application **must not** be linked to the default `0x08000000` address. It must be shifted to match `APP_ACTIVE_START_ADDR` (Slot 5).

In your **Application's** `.ld` file, update the `ORIGIN` and reduce the `LENGTH` so it fits perfectly inside the slot size defined in `mem_layout.h`:
```ld
/* Example: Application starts at Slot 5 and is restricted to SLOT_SIZE */
MEMORY
{
  RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 320K
  FLASH (rx)     : ORIGIN = 0x08040000, LENGTH = 256K /* 0x40000 */
}
```

### 3. Application Vector Table Offset (VTOR)
When the bootloader jumps to the application, the ARM Cortex core needs to know where the application's interrupt handlers are located. 

In your **Application's** initialization code (typically `system_<mcu>.c`), you must enable the vector table relocation and set the offset to match your `APP_ACTIVE_START_ADDR`.

**For STM32 (in `system_stm32fxxx.c`):**
```c
/* 1. Uncomment to enable vector table relocation */
#define USER_VECT_TAB_ADDRESS

#if defined(USER_VECT_TAB_ADDRESS)
/* 2. Set the offset to match your APP_ACTIVE_START_ADDR offset from base flash */
/* Example: 0x08040000 - 0x08000000 = 0x00040000 */
#define VECT_TAB_OFFSET  0x00040000U 
#endif
```
---
## Step 3 — Platform Driver

Copy `Core/Src/Drivers/system_driver_template.c`, rename it to
`system_driver_<mcu>.c`, and fill in every `TODO`.
`system_driver_stm32f7.c` is a full working reference for comparison.

The file must implement these functions and expose `Sys_GetInterface()`:

| Group | Functions to implement |
|-------|------------------------|
| System | `Init`, `DeInit`, `SystemReset`, `Delay`, `GetTick` |
| UART | `UART_Write` — used only for debug logging |
| GPIO | `GPIO_ReadUserButton` → return 1 if pressed; `GPIO_ToggleLed` |
| Flash | `Flash_Erase(addr, len)`, `Flash_Write(addr, data, len)` — return 0 on success |
| Critical | `DisableIRQ`, `EnableIRQ`, `ErrorHandler` |
| Boot | `JumpToApp` — see note below |

**`JumpToApp` checklist (Cortex-M):**
1. Validate stack pointer (word 0 of vector table must point into RAM)
2. Disable MPU, stop SysTick
3. Disable all IRQs and clear pending flags
4. Set `SCB->VTOR` to the app base address
5. Set MSP, then call the reset handler (word 1 of vector table)

**Interface struct** — fill `.mem` from `mem_layout.h` and wire `.crypto` to the
software driver (or your hardware functions):

```c
static const Bootloader_Interface_t mcu_interface = {
    .mem    = { CONFIG_SECTOR_ADDR, APP_ACTIVE_START_ADDR, ... },
    .crypto = { SW_AES_EncryptBlock, SW_AES_DecryptBlock, SW_SHA256, SW_ECDSA_Verify },
    .Init = MCU_Init, .Flash_Erase = MCU_Flash_Erase, /* ... */
};
const Bootloader_Interface_t* Sys_GetInterface(void) { return &mcu_interface; }
```

---

## Step 4 — Hardware Crypto (optional)

By default all crypto runs in software via `crypto_driver_sw.c` (TinyCrypt).
If your MCU has AES/SHA/ECC hardware, replace individual entries in `.crypto`:

```c
.crypto = {
    .AES_EncryptBlock = HW_AES_EncryptBlock,   // e.g. STM32 CRYP peripheral
    .AES_DecryptBlock = HW_AES_DecryptBlock,
    .SHA256           = SW_SHA256,             // no HW SHA — keep software
    .ECDSA_Verify     = SW_ECDSA_Verify,
},
```

Each function must follow the same signature as its `SW_` counterpart in
`crypto_driver_sw.h`. Return **0 on success**, non-zero on error.

---

## Step 5 — Key Toolchain (`Key/` folder)

```
Key/
├── keygen.py          — generate ECDSA key pair + AES key (run once)
├── extract_pubkey.py  — print keys as C arrays → paste into keys.c
└── generate_update.py — encrypt + sign a .bin → update_encrypted.bin
```

**Install dependencies once:**
```bash
pip install ecdsa pycryptodome cryptography
```

### Workflow

**1. Generate keys** (once per project — keep `private.pem` and `secret.key` secret):
```bash
cd Key/ && python keygen.py
```

**2. Embed public keys in the bootloader:**
```bash
python extract_pubkey.py
```
Copy the printed arrays into `Core/Src/keys.c`, then rebuild the bootloader.

**3. Package a firmware release:**
```bash
python generate_update.py path/to/app.bin
```
Set `FIRMWARE_VERSION` at the top of the script before running.
Output: `update_encrypted.bin` — flash this into the download slot (S6).

### CMake post-build

Add to your **application's** `CMakeLists.txt` to auto-generate the package
on every build:

```cmake
set(KEY_DIR "${CMAKE_SOURCE_DIR}/../BOOTLOADER1/Key")
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
            ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin
    COMMAND python ${KEY_DIR}/generate_update.py
            ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin
    WORKING_DIRECTORY ${KEY_DIR}
)
```
---

## Step 5 — Wiring into `main()`

In your `main.c`, call `Bootloader_Run` with the platform interface after
hardware init and before anything else:

```c
#include "bootloader_core.h"
#include "system_interface.h"

// Declared in your system_driver_<mcu>.c
extern const Bootloader_Interface_t* Sys_GetInterface(void);

int main(void) {
    /* HAL / clock / peripheral init here */

    Bootloader_Run(Sys_GetInterface());

    /* Never reached — bootloader either jumps to app or halts */
}
```
---

## Firmware Image Format

```
[ IV 16B ][ Encrypted App (AES-128-CBC, PKCS7 padded) ][ fw_footer_t 76B ]
```

- ECDSA signature covers `IV + Encrypted App` (`footer.size` bytes total)
- Footer contains: `version`, `size`, `signature[64]`, `magic (0x454E4421)`
- `generate_update.py` produces this layout automatically

---

## State Machine

```
Power On → Read Config
    │
    ├─ Button held → check S6 → UPDATE_REQ / ROLLBACK / NORMAL
    │
    ├─ STATE_UPDATE_REQ   → verify sig → decrypt S6→S7 → backup S5→S6 → install S7→S5 → reset
    ├─ STATE_ROLLBACK      → decrypt S6→S7 → backup S5→S6 → install S7→S5 → reset
    └─ STATE_NORMAL        → valid app in S5? → JumpToApp : check S6 : halt
```

---

## File Reference

| File | Layer | Purpose |
|------|-------|---------|
| `Core/Inc/system_interface.h` | Interface | `Bootloader_Interface_t`, `BL_MemoryMap_t`, `BL_CryptoOps_t` |
| `Core/Inc/mem_layout.h` | **Edit per target** | Flash addresses |
| `Core/Inc/bootloader_config.h` | Portable | Boot states, `BootConfig_t` |
| `Core/Inc/firmware_footer.h` | Portable | `fw_footer_t`, status codes |
| `Core/Src/bootloader_core.c` | Portable | State machine |
| `Core/Src/BL_Functions.c` | Portable | Update, rollback, config R/W |
| `Core/Src/Cryptology_Control.c` | Portable | Footer scan, SHA-256, ECDSA |
| `Core/Src/keys.c` | **Replace per project** | AES + ECDSA public keys |
| `Core/Src/Drivers/system_driver_template.c` | Platform | **Start here** — empty driver template |
| `Core/Src/Drivers/system_driver_stm32f7.c` | Platform | STM32F746 HAL reference implementation |
| `Core/Src/Drivers/crypto_driver_sw.c` | Driver | TinyCrypt wrappers |

---

---

## Authors

- **Oğuz Mert Coşkun**  
  🔗 GitHub: https://github.com/Omert2004

- **Mert Kırgın**  
  🔗 GitHub: https://github.com/mertkirgin

**Electrical & Electronics Engineering**  
**Özyeğin University**

---

## Keywords

`ARM Cortex-x` `Secure Boot` `Bootloader` `Embedded Security`  
`ECDSA` `SHA-256` `AES-128` `MPU`  
`Firmware Update` `Dual Slot` `Root of Trust`

