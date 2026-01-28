################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Drivers/flash_driver_stm32f7.c \
../Core/Src/Drivers/system_driver_stm32f7.c 

OBJS += \
./Core/Src/Drivers/flash_driver_stm32f7.o \
./Core/Src/Drivers/system_driver_stm32f7.o 

C_DEPS += \
./Core/Src/Drivers/flash_driver_stm32f7.d \
./Core/Src/Drivers/system_driver_stm32f7.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Drivers/%.o Core/Src/Drivers/%.su Core/Src/Drivers/%.cyclo: ../Core/Src/Drivers/%.c Core/Src/Drivers/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F746xx -c -I"C:/Users/mertk/STM32CubeIDE/workspace_1.18.0/ARM-Cortex-Secure-Bootloader/BOOTLOADER1/Libs/lz4/Inc" -I../Core/Inc -I"C:/Users/mertk/STM32CubeIDE/workspace_1.18.0/ARM-Cortex-Secure-Bootloader/BOOTLOADER1/Libs/tinycrypt/Inc" -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Drivers

clean-Core-2f-Src-2f-Drivers:
	-$(RM) ./Core/Src/Drivers/flash_driver_stm32f7.cyclo ./Core/Src/Drivers/flash_driver_stm32f7.d ./Core/Src/Drivers/flash_driver_stm32f7.o ./Core/Src/Drivers/flash_driver_stm32f7.su ./Core/Src/Drivers/system_driver_stm32f7.cyclo ./Core/Src/Drivers/system_driver_stm32f7.d ./Core/Src/Drivers/system_driver_stm32f7.o ./Core/Src/Drivers/system_driver_stm32f7.su

.PHONY: clean-Core-2f-Src-2f-Drivers

