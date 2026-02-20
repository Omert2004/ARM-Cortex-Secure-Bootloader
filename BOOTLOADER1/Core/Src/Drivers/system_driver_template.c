/*
 * system_driver_template.c
 *
 * Platform driver template — copy this file, rename it to
 * system_driver_<your_mcu>.c, and fill in every TODO section.
 *
 * See system_driver_stm32f7.c for a complete reference implementation.
 */

#include "system_interface.h"
#include "mem_layout.h"
#include "crypto_driver_sw.h"
/* TODO: include your MCU HAL / SDK headers here */

/* ===== System Control ===== */

static void MCU_Init(void) {
    /* TODO: init UART/RTT for printf, configure clocks if needed */
}

static void MCU_DeInit(void) {
    /* TODO: de-initialize peripherals before jumping to the app */
}

static void MCU_SystemReset(void) {
    /* TODO: e.g. NVIC_SystemReset() */
}

static void MCU_Delay(uint32_t ms) {
    /* TODO: blocking millisecond delay */
}

static uint32_t MCU_GetTick(void) {
    /* TODO: return a millisecond tick counter */
    return 0;
}

/* ===== Communications ===== */

static void MCU_UART_Write(const uint8_t *data, uint16_t size) {
    /* TODO: transmit `size` bytes from `data` over UART/RTT/SWO */
    (void)data; (void)size;
}

/* ===== GPIO ===== */

static uint8_t MCU_GPIO_ReadUserButton(void) {
    /* TODO: return 1 if the trigger button is pressed, 0 otherwise */
    return 0;
}

static void MCU_GPIO_ToggleLed(void) {
    /* TODO: toggle a status LED (optional) */
}

/* ===== Flash ===== */

static uint32_t GetSector(uint32_t address) {
    /* TODO: map a byte address to a sector / page / block number.
     * Return the sector index used by your HAL erase API.
     * Example: STM32F7 maps 0x08040000 → FLASH_SECTOR_5 */
    (void)address;
    return 0;
}

static int MCU_Flash_Erase(uint32_t address, uint32_t length) {
    /* TODO: erase all sectors that cover [address, address + length).
     *
     * Steps:
     *   1. Unlock flash
     *   2. Find first and last sector using GetSector()
     *   3. Erase sectors
     *   4. Lock flash
     *   5. Return 0 on success, -1 on failure
     */
    (void)address; (void)length;
    return -1;
}

static int MCU_Flash_Write(uint32_t address, const uint8_t *data, uint32_t length) {
    /* TODO: write `length` bytes to flash at `address`.
     *
     * Steps:
     *   1. Unlock flash
     *   2. Program byte-by-byte (or word-by-word if your HAL requires alignment)
     *   3. Lock flash
     *   4. Return 0 on success, -1 on failure
     */
    (void)address; (void)data; (void)length;
    return -1;
}

/* ===== Critical ===== */

static void MCU_ErrorHandler(void) {
    /* TODO: disable IRQs and halt — or trigger a watchdog reset */
    __disable_irq();
    while (1) {}
}

static void MCU_DisableIRQ(void) {
    __disable_irq();  /* CMSIS — works on all Cortex-M */
}

static void MCU_EnableIRQ(void) {
    __enable_irq();
}

static void MCU_JumpToApp(void) {
    /* TODO: jump to the application.
     *
     * Required steps (Cortex-M):
     *   1. Read stack pointer  = *(uint32_t *)(APP_ACTIVE_START_ADDR + 0)
     *   2. Read reset handler  = *(uint32_t *)(APP_ACTIVE_START_ADDR + 4)
     *   3. Validate SP — upper byte must match your MCU's RAM base
     *   4. Disable MPU (if used), stop SysTick
     *   5. Disable all IRQs: NVIC->ICER[0..7] = 0xFFFFFFFF
     *   6. Clear pending IRQs: NVIC->ICPR[0..7] = 0xFFFFFFFF
     *   7. Call HAL_DeInit() or equivalent
     *   8. SCB->VTOR = APP_ACTIVE_START_ADDR
     *   9. __set_MSP(stack_pointer)
     *  10. Call reset_handler()
     *
     * See STM32_JumpToApp() in system_driver_stm32f7.c for a full example.
     */
}

/* ===== Interface Definition ===== */

static const Bootloader_Interface_t mcu_interface = {
    /* --- Memory layout (values from mem_layout.h) --- */
    .mem = {
        .config_addr       = CONFIG_SECTOR_ADDR,
        .app_active_addr   = APP_ACTIVE_START_ADDR,
        .app_download_addr = APP_DOWNLOAD_START_ADDR,
        .scratch_addr      = SCRATCH_ADDR,
        .slot_size         = SLOT_SIZE,
        .flash_base        = 0x00000000,   /* TODO: your MCU's flash base address */
        .ram_base          = 0x00000000,   /* TODO: your MCU's RAM  base address  */
    },

    /* --- Cryptography ---
     * Software (TinyCrypt) is used by default for every operation.
     * Replace individual entries with hardware-accelerated functions
     * if your MCU has AES / SHA / ECC peripherals.
     * Each replacement must match the signature in crypto_driver_sw.h.
     */
    .crypto = {
        .AES_EncryptBlock = SW_AES_EncryptBlock,
        .AES_DecryptBlock = SW_AES_DecryptBlock,
        .SHA256           = SW_SHA256,
        .ECDSA_Verify     = SW_ECDSA_Verify,
    },

    /* --- Function pointers --- */
    .Init              = MCU_Init,
    .DeInit            = MCU_DeInit,
    .SystemReset       = MCU_SystemReset,
    .Delay             = MCU_Delay,
    .GetTick           = MCU_GetTick,

    .UART_Write        = MCU_UART_Write,

    .GPIO_ReadUserButton = MCU_GPIO_ReadUserButton,
    .GPIO_ToggleLed    = MCU_GPIO_ToggleLed,

    .Flash_Unlock      = NULL,   /* unlock/lock are called internally by Erase/Write */
    .Flash_Lock        = NULL,
    .Flash_Erase       = MCU_Flash_Erase,
    .Flash_Write       = MCU_Flash_Write,

    .ErrorHandler      = MCU_ErrorHandler,
    .JumpToApp         = MCU_JumpToApp,
    .DisableIRQ        = MCU_DisableIRQ,
    .EnableIRQ         = MCU_EnableIRQ,
};

const Bootloader_Interface_t* Sys_GetInterface(void) {
    return &mcu_interface;
}
