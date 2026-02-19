/*
 * system_driver_stm32f7.c
 *
 * STM32F746 platform driver — single file implementing all of
 * Bootloader_Interface_t.  Every HAL call lives here; nothing
 * else in the bootloader touches STM32 headers.
 *
 *  Created on: Jan 28, 2026
 *      Author: mertk
 */

#include "system_interface.h"
#include "stm32f7xx_hal.h"
#include "tiny_printf.h"
#include "mem_layout.h"
#include "crypto_driver_sw.h"

extern UART_HandleTypeDef huart1;
extern void Error_Handler(void);

/* ===== System Control ===== */

static void STM32_Init(void) {
    tfp_init(&huart1);
}

static void STM32_DeInit(void) {
    HAL_DeInit();
}

static void STM32_SystemReset(void) {
    HAL_NVIC_SystemReset();
}

static void STM32_Delay(uint32_t ms) {
    HAL_Delay(ms);
}

static uint32_t STM32_GetTick(void) {
    return HAL_GetTick();
}

/* ===== Communications ===== */

static void STM32_UART_Write(const uint8_t *data, uint16_t size) {
    HAL_UART_Transmit(&huart1, data, size, HAL_MAX_DELAY);
}

/* ===== GPIO ===== */

static uint8_t STM32_GPIO_ReadUserButton(void) {
    return (HAL_GPIO_ReadPin(GPIOI, GPIO_PIN_11) == GPIO_PIN_SET) ? 1 : 0;
}

static void STM32_GPIO_ToggleLed(void) {
    HAL_GPIO_TogglePin(GPIOI, GPIO_PIN_1);
}

/* ===== Flash ===== */

static uint32_t GetSector(uint32_t address) {
    if (address >= 0x08000000 && address < 0x08008000) return FLASH_SECTOR_0;
    if (address >= 0x08040000 && address < 0x08080000) return FLASH_SECTOR_5;
    if (address >= 0x08080000 && address < 0x080C0000) return FLASH_SECTOR_6;
    if (address >= 0x080C0000 && address < 0x08100000) return FLASH_SECTOR_7;
    return FLASH_SECTOR_7;
}

static void STM32_Flash_Unlock(void) {
    HAL_FLASH_Unlock();
}

static void STM32_Flash_Lock(void) {
    HAL_FLASH_Lock();
}

static int STM32_Flash_Erase(uint32_t address, uint32_t length) {
    FLASH_EraseInitTypeDef erase_init;
    uint32_t sector_error;

    if (HAL_FLASH_Unlock() != HAL_OK)
        return -1;

    uint32_t first_sector = GetSector(address);
    uint32_t last_sector  = GetSector(address + length - 1);

    erase_init.TypeErase    = FLASH_TYPEERASE_SECTORS;
    erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase_init.Sector       = first_sector;
    erase_init.NbSectors    = last_sector - first_sector + 1;

    if (HAL_FLASHEx_Erase(&erase_init, &sector_error) != HAL_OK) {
        HAL_FLASH_Lock();
        return -1;
    }

    HAL_FLASH_Lock();
    return 0;
}

static int STM32_Flash_Write(uint32_t address, const uint8_t *data, uint32_t length) {
    if (HAL_FLASH_Unlock() != HAL_OK)
        return -1;

    for (uint32_t i = 0; i < length; i++) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address + i, data[i]) != HAL_OK) {
            HAL_FLASH_Lock();
            return -1;
        }
    }

    HAL_FLASH_Lock();
    return 0;
}

/* ===== Critical ===== */

static void STM32_ErrorHandler(void) {
    Error_Handler();
}

static void STM32_DisableIRQ(void) {
    __disable_irq();
}

static void STM32_EnableIRQ(void) {
    __enable_irq();
}

static void STM32_JumpToApp(void) {
    uint32_t app_addr = APP_ACTIVE_START_ADDR;
    uint32_t app_stack_addr = *(volatile uint32_t *)app_addr;
    uint32_t app_reset_handler = *(volatile uint32_t *)(app_addr + 4);

    if ((app_stack_addr & 0x20000000) != 0x20000000)
        return;

    HAL_MPU_Disable();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    __disable_irq();
    HAL_DeInit();

    for (int i = 0; i < 8; i++) {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    SCB->VTOR = app_addr;

    __set_MSP(app_stack_addr);
    void (*pJump)(void) = (void (*)(void))app_reset_handler;
    pJump();
}

/* ===== Interface Definition ===== */

static const Bootloader_Interface_t stm32f7_interface = {
    .mem = {
        .config_addr       = CONFIG_SECTOR_ADDR,
        .app_active_addr   = APP_ACTIVE_START_ADDR,
        .app_download_addr = APP_DOWNLOAD_START_ADDR,
        .scratch_addr      = SCRATCH_ADDR,
        .slot_size         = SLOT_SIZE,
        .flash_base        = 0x08000000,
        .ram_base          = 0x20000000,
    },

    .crypto = {
        .AES_EncryptBlock = SW_AES_EncryptBlock,
        .AES_DecryptBlock = SW_AES_DecryptBlock,
        .SHA256           = SW_SHA256,
        .ECDSA_Verify     = SW_ECDSA_Verify,
    },

    .Init              = STM32_Init,
    .DeInit            = STM32_DeInit,
    .SystemReset       = STM32_SystemReset,
    .Delay             = STM32_Delay,
    .GetTick           = STM32_GetTick,

    .UART_Write        = STM32_UART_Write,

    .GPIO_ReadUserButton = STM32_GPIO_ReadUserButton,
    .GPIO_ToggleLed    = STM32_GPIO_ToggleLed,

    .Flash_Unlock      = STM32_Flash_Unlock,
    .Flash_Lock        = STM32_Flash_Lock,
    .Flash_Erase       = STM32_Flash_Erase,
    .Flash_Write       = STM32_Flash_Write,

    .ErrorHandler      = STM32_ErrorHandler,
    .JumpToApp         = STM32_JumpToApp,
    .DisableIRQ        = STM32_DisableIRQ,
    .EnableIRQ         = STM32_EnableIRQ,
};

const Bootloader_Interface_t* Sys_GetInterface(void) {
    return &stm32f7_interface;
}
