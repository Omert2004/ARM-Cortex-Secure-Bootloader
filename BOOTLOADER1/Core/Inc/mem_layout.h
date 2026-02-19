/*
 * mem_layout.h
 *
 * STM32F746 flash memory map.
 * Only included by the platform driver (system_driver_stm32f7.c).
 */
#ifndef INC_MEM_LAYOUT_H_
#define INC_MEM_LAYOUT_H_

#define CONFIG_SECTOR_ADDR       0x08010000  /* Sector 2  — Boot config    */
#define APP_ACTIVE_START_ADDR    0x08040000  /* Sector 5  — Active app     */
#define APP_DOWNLOAD_START_ADDR  0x08080000  /* Sector 6  — Download slot  */
#define SCRATCH_ADDR             0x080C0000  /* Sector 7  — Scratch buffer */
#define SLOT_SIZE                0x00040000  /* 256 KB per slot             */

#endif /* INC_MEM_LAYOUT_H_ */
