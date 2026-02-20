# \# ARM Cortex Secure Bootloader — Porting Guide

# 

# The bootloader logic is designed to be hardware-agnostic. All MCU-specific code lives in

# a single platform driver file.

# 

# ---

# 

# \## Architecture

# 

# ```

# ┌─────────────────────────────────────────────────────────┐

# │                    PORTABLE LAYER                        │

# │   bootloader\_core.c  →  BL\_Functions.c                  │

# │                      →  Cryptology\_Control.c             │

# │                                                          │

# │   All hardware access via:  sys->mem.\*                   │

# │                             sys->crypto.\*                │

# │                             sys->Flash\_\*() / JumpToApp() │

# ├─────────────────────────────────────────────────────────┤

# │                   system\_interface.h                     │

# │     BL\_MemoryMap\_t  +  BL\_CryptoOps\_t  +  fn pointers   │

# ├────────────────────────┬────────────────────────────────┤

# │  system\_driver\_<mcu>.c │  crypto\_driver\_sw.c            │

# │  (your platform code)  │  (TinyCrypt — any platform)    │

# └────────────────────────┴────────────────────────────────┘

# ```

# 

# ---

# 

# \## Step 1 — Memory Map

# 

# Edit `Core/Inc/mem\_layout.h` with your MCU's flash addresses:

# 

# ```c

# \#define CONFIG\_SECTOR\_ADDR       0x08010000

# \#define APP\_ACTIVE\_START\_ADDR    0x08040000

# \#define APP\_DOWNLOAD\_START\_ADDR  0x08080000

# \#define SCRATCH\_ADDR             0x080C0000

# \#define SLOT\_SIZE                0x00040000

# ```

# 

# \*\*Rules:\*\*

# \- All three slots (active, download, scratch) must be the same `SLOT\_SIZE`.

# \- Config sector must be in its own erasable sector, below all three slots.

# \- The bootloader itself must fit below `CONFIG\_SECTOR\_ADDR`.

# 

# ---

# 

# \## Step 2 — Platform Driver

# 

# Copy `Core/Src/Drivers/system\_driver\_template.c`, rename it to

# `system\_driver\_<mcu>.c`, and fill in every `TODO`.

# `system\_driver\_stm32f7.c` is a full working reference for comparison.

# 

# The file must implement these functions and expose `Sys\_GetInterface()`:

# 

# | Group | Functions to implement |

# |-------|------------------------|

# | System | `Init`, `DeInit`, `SystemReset`, `Delay`, `GetTick` |

# | UART | `UART\_Write` — used only for debug logging |

# | GPIO | `GPIO\_ReadUserButton` → return 1 if pressed; `GPIO\_ToggleLed` |

# | Flash | `Flash\_Erase(addr, len)`, `Flash\_Write(addr, data, len)` — return 0 on success |

# | Critical | `DisableIRQ`, `EnableIRQ`, `ErrorHandler` |

# | Boot | `JumpToApp` — see note below |

# 

# \*\*`JumpToApp` checklist (Cortex-M):\*\*

# 1\. Validate stack pointer (word 0 of vector table must point into RAM)

# 2\. Disable MPU, stop SysTick

# 3\. Disable all IRQs and clear pending flags

# 4\. Set `SCB->VTOR` to the app base address

# 5\. Set MSP, then call the reset handler (word 1 of vector table)

# 

# \*\*Interface struct\*\* — fill `.mem` from `mem\_layout.h` and wire `.crypto` to the

# software driver (or your hardware functions):

# 

# ```c

# static const Bootloader\_Interface\_t mcu\_interface = {

# &nbsp;   .mem    = { CONFIG\_SECTOR\_ADDR, APP\_ACTIVE\_START\_ADDR, ... },

# &nbsp;   .crypto = { SW\_AES\_EncryptBlock, SW\_AES\_DecryptBlock, SW\_SHA256, SW\_ECDSA\_Verify },

# &nbsp;   .Init = MCU\_Init, .Flash\_Erase = MCU\_Flash\_Erase, /\* ... \*/

# };

# const Bootloader\_Interface\_t\* Sys\_GetInterface(void) { return \&mcu\_interface; }

# ```

# 

# ---

# 

# \## Step 3 — Hardware Crypto (optional)

# 

# By default all crypto runs in software via `crypto\_driver\_sw.c` (TinyCrypt).

# If your MCU has AES/SHA/ECC hardware, replace individual entries in `.crypto`:

# 

# ```c

# .crypto = {

# &nbsp;   .AES\_EncryptBlock = HW\_AES\_EncryptBlock,   // e.g. STM32 CRYP peripheral

# &nbsp;   .AES\_DecryptBlock = HW\_AES\_DecryptBlock,

# &nbsp;   .SHA256           = SW\_SHA256,             // no HW SHA — keep software

# &nbsp;   .ECDSA\_Verify     = SW\_ECDSA\_Verify,

# },

# ```

# 

# Each function must follow the same signature as its `SW\_` counterpart in

# `crypto\_driver\_sw.h`. Return \*\*0 on success\*\*, non-zero on error.

# 

# ---

# 

# \## Step 4 — Key Toolchain (`Key/` folder)

# 

# ```

# Key/

# ├── keygen.py          — generate ECDSA key pair + AES key (run once)

# ├── extract\_pubkey.py  — print keys as C arrays → paste into keys.c

# └── generate\_update.py — encrypt + sign a .bin → update\_encrypted.bin

# ```

# 

# \*\*Install dependencies once:\*\*

# ```bash

# pip install ecdsa pycryptodome cryptography

# ```

# 

# \### Workflow

# 

# \*\*1. Generate keys\*\* (once per project — keep `private.pem` and `secret.key` secret):

# ```bash

# cd Key/ \&\& python keygen.py

# ```

# 

# \*\*2. Embed public keys in the bootloader:\*\*

# ```bash

# python extract\_pubkey.py

# ```

# Copy the printed arrays into `Core/Src/keys.c`, then rebuild the bootloader.

# 

# \*\*3. Package a firmware release:\*\*

# ```bash

# python generate\_update.py path/to/app.bin

# ```

# Set `FIRMWARE\_VERSION` at the top of the script before running.

# Output: `update\_encrypted.bin` — flash this into the download slot (S6).

# 

# \### CMake post-build

# 

# Add to your \*\*application's\*\* `CMakeLists.txt` to auto-generate the package

# on every build:

# 

# ```cmake

# set(KEY\_DIR "${CMAKE\_SOURCE\_DIR}/../BOOTLOADER1/Key")

# add\_custom\_command(TARGET ${CMAKE\_PROJECT\_NAME} POST\_BUILD

# &nbsp;   COMMAND ${CMAKE\_OBJCOPY} -O binary $<TARGET\_FILE:${CMAKE\_PROJECT\_NAME}>

# &nbsp;           ${CMAKE\_BINARY\_DIR}/${CMAKE\_PROJECT\_NAME}.bin

# &nbsp;   COMMAND python ${KEY\_DIR}/generate\_update.py

# &nbsp;           ${CMAKE\_BINARY\_DIR}/${CMAKE\_PROJECT\_NAME}.bin

# &nbsp;   WORKING\_DIRECTORY ${KEY\_DIR}

# &nbsp;   COMMENT "Generating encrypted update package..."

# )

# ```

# 

# > `private.pem` and `secret.key` must be in `Key/` at build time. Never commit them.

# 

# ---

# 

# \## Step 5 — Wire into `main()`

# 

# In your `main.c`, call `Bootloader\_Run` with the platform interface after

# hardware init and before anything else:

# 

# ```c

# \#include "bootloader\_core.h"

# \#include "system\_interface.h"

# 

# // Declared in your system\_driver\_<mcu>.c

# extern const Bootloader\_Interface\_t\* Sys\_GetInterface(void);

# 

# int main(void) {

# &nbsp;   /\* HAL / clock / peripheral init here \*/

# 

# &nbsp;   Bootloader\_Run(Sys\_GetInterface());

# 

# &nbsp;   /\* Never reached — bootloader either jumps to app or halts \*/

# }

# ```

# 

# `Bootloader\_Run` never returns. It either jumps to the application or calls

# `ErrorHandler` if no valid image is found.

# 

# ---

# 

# \## Step 6 — CMake

# 

# Swap the STM32F7 driver for yours:

# 

# ```cmake

# target\_sources(${CMAKE\_PROJECT\_NAME} PRIVATE

# &nbsp;   Core/Src/Drivers/system\_driver\_<mcu>.c   # replace this line

# &nbsp;   Core/Src/Drivers/crypto\_driver\_sw.c

# &nbsp;   Core/Src/bootloader\_core.c

# &nbsp;   Core/Src/BL\_Functions.c

# &nbsp;   Core/Src/Cryptology\_Control.c

# &nbsp;   Core/Src/keys.c

# &nbsp;   Core/Src/tiny\_printf.c

# )

# ```

# 

# ---

# 

# \## Firmware Image Format

# 

# ```

# \[ IV 16B ]\[ Encrypted App (AES-128-CBC, PKCS7 padded) ]\[ fw\_footer\_t 76B ]

# ```

# 

# \- ECDSA signature covers `IV + Encrypted App` (`footer.size` bytes total)

# \- Footer contains: `version`, `size`, `signature\[64]`, `magic (0x454E4421)`

# \- `generate\_update.py` produces this layout automatically

# 

# ---

# 

# \## State Machine

# 

# ```

# Power On → Read Config

# &nbsp;   │

# &nbsp;   ├─ Button held → check S6 → UPDATE\_REQ / ROLLBACK / NORMAL

# &nbsp;   │

# &nbsp;   ├─ STATE\_UPDATE\_REQ   → verify sig → decrypt S6→S7 → backup S5→S6 → install S7→S5 → reset

# &nbsp;   ├─ STATE\_ROLLBACK      → decrypt S6→S7 → backup S5→S6 → install S7→S5 → reset

# &nbsp;   └─ STATE\_NORMAL        → valid app in S5? → JumpToApp : check S6 : halt

# ```

# 

# ---

# 

# \## File Reference

# 

# | File | Layer | Purpose |

# |------|-------|---------|

# | `Core/Inc/system\_interface.h` | Interface | `Bootloader\_Interface\_t`, `BL\_MemoryMap\_t`, `BL\_CryptoOps\_t` |

# | `Core/Inc/mem\_layout.h` | \*\*Edit per target\*\* | Flash addresses |

# | `Core/Inc/bootloader\_config.h` | Portable | Boot states, `BootConfig\_t` |

# | `Core/Inc/firmware\_footer.h` | Portable | `fw\_footer\_t`, status codes |

# | `Core/Src/bootloader\_core.c` | Portable | State machine |

# | `Core/Src/BL\_Functions.c` | Portable | Update, rollback, config R/W |

# | `Core/Src/Cryptology\_Control.c` | Portable | Footer scan, SHA-256, ECDSA |

# | `Core/Src/keys.c` | \*\*Replace per project\*\* | AES + ECDSA public keys |

# | `Core/Src/Drivers/system\_driver\_template.c` | Platform | \*\*Start here\*\* — empty driver template |

# | `Core/Src/Drivers/system\_driver\_stm32f7.c` | Platform | STM32F746 reference implementation |

# | `Core/Src/Drivers/crypto\_driver\_sw.c` | Driver | TinyCrypt wrappers |

# 

# ---

# 

# \## Pre-Deployment Checklist

# 

# \- \[ ] New keys generated (`keygen.py`) and embedded in `keys.c`

# \- \[ ] `mem\_layout.h` addresses match your MCU's flash layout

# \- \[ ] All `TODO` sections in your platform driver are implemented

# \- \[ ] `JumpToApp` disables all IRQs and sets `SCB->VTOR`

# \- \[ ] Tested: boot → update → rollback

# \- \[ ] `private.pem` and `secret.key` are not in version control



