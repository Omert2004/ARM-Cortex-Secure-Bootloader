#include "flash_interface.h"
#include "stm32f7xx_hal.h"

//TODO: CHANGE THE HAL LOGIC WITH LL
static uint32_t GetSector(uint32_t Address) {
    if((Address >= 0x08000000) && (Address < 0x08008000)) return FLASH_SECTOR_0;
    if((Address >= 0x08040000) && (Address < 0x08080000)) return FLASH_SECTOR_5;
    if((Address >= 0x08080000) && (Address < 0x080C0000)) return FLASH_SECTOR_6;
    if((Address >= 0x080C0000) && (Address < 0x08100000)) return FLASH_SECTOR_7;
    return FLASH_SECTOR_7;
}

Flash_Status_t Flash_Interface_Init(void) {
	// LL OLSAYDI BURDA CLOCK AYARLARI YAPACAKTIK
    return FLASH_OK;
}

Flash_Status_t Flash_Interface_Lock(void) {
    if (HAL_FLASH_Lock() != HAL_OK) {
        return FLASH_ERR_LOCKED;
    }
    return FLASH_OK;
}

Flash_Status_t Flash_Interface_Unlock(void) {
    if (HAL_FLASH_Unlock() != HAL_OK) {
        return FLASH_ERR_LOCKED;
    }
    return FLASH_OK;
}

Flash_Status_t Flash_Interface_Erase(uint32_t startAddress, uint32_t length) {
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t SectorError;

	if (Flash_Interface_Unlock() != FLASH_OK) return FLASH_ERR_LOCKED;


	uint32_t FirstSector = GetSector(startAddress);
	uint32_t NbOfSectors = GetSector(startAddress + length - 1) - FirstSector + 1;

	EraseInitStruct.TypeErase    = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Sector       = FirstSector;
	EraseInitStruct.NbSectors    = NbOfSectors;

	if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
		Flash_Interface_Lock(); // hata olsa da kilit
	    return FLASH_ERR_ERASE;
	}

	Flash_Interface_Lock();
	return FLASH_OK;
}

Flash_Status_t Flash_Interface_Write(uint32_t address, const uint8_t *data, uint32_t length) {
	if (Flash_Interface_Unlock() != FLASH_OK) return FLASH_ERR_LOCKED;

	for (uint32_t i = 0; i < length; i++) {
	 // Not: LL kullanırsak burayı da değiştireceğiz
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address + i, data[i]) != HAL_OK) {
			Flash_Interface_Lock();
			return FLASH_ERR_WRITE;
		}
	}

	Flash_Interface_Lock();
	return FLASH_OK;
}
