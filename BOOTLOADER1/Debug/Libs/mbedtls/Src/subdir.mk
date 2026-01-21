################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libs/mbedtls/Src/aes.c \
../Libs/mbedtls/Src/asn1parse.c \
../Libs/mbedtls/Src/bignum.c \
../Libs/mbedtls/Src/cipher.c \
../Libs/mbedtls/Src/cipher_wrap.c \
../Libs/mbedtls/Src/constant_time.c \
../Libs/mbedtls/Src/ecdsa.c \
../Libs/mbedtls/Src/ecp.c \
../Libs/mbedtls/Src/ecp_curves.c \
../Libs/mbedtls/Src/md.c \
../Libs/mbedtls/Src/oid.c \
../Libs/mbedtls/Src/platform.c \
../Libs/mbedtls/Src/platform_util.c \
../Libs/mbedtls/Src/sha256.c 

OBJS += \
./Libs/mbedtls/Src/aes.o \
./Libs/mbedtls/Src/asn1parse.o \
./Libs/mbedtls/Src/bignum.o \
./Libs/mbedtls/Src/cipher.o \
./Libs/mbedtls/Src/cipher_wrap.o \
./Libs/mbedtls/Src/constant_time.o \
./Libs/mbedtls/Src/ecdsa.o \
./Libs/mbedtls/Src/ecp.o \
./Libs/mbedtls/Src/ecp_curves.o \
./Libs/mbedtls/Src/md.o \
./Libs/mbedtls/Src/oid.o \
./Libs/mbedtls/Src/platform.o \
./Libs/mbedtls/Src/platform_util.o \
./Libs/mbedtls/Src/sha256.o 

C_DEPS += \
./Libs/mbedtls/Src/aes.d \
./Libs/mbedtls/Src/asn1parse.d \
./Libs/mbedtls/Src/bignum.d \
./Libs/mbedtls/Src/cipher.d \
./Libs/mbedtls/Src/cipher_wrap.d \
./Libs/mbedtls/Src/constant_time.d \
./Libs/mbedtls/Src/ecdsa.d \
./Libs/mbedtls/Src/ecp.d \
./Libs/mbedtls/Src/ecp_curves.d \
./Libs/mbedtls/Src/md.d \
./Libs/mbedtls/Src/oid.d \
./Libs/mbedtls/Src/platform.d \
./Libs/mbedtls/Src/platform_util.d \
./Libs/mbedtls/Src/sha256.d 


# Each subdirectory must supply rules for building sources it contributes
Libs/mbedtls/Src/%.o Libs/mbedtls/Src/%.su Libs/mbedtls/Src/%.cyclo: ../Libs/mbedtls/Src/%.c Libs/mbedtls/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F746xx -c -I"C:/Users/mertk/STM32CubeIDE/workspace_1.18.0/CS350_Project/BOOTLOADER1/Libs/lz4/Inc" -I"C:/Users/mertk/STM32CubeIDE/workspace_1.18.0/CS350_Project/BOOTLOADER1/Libs/mbedtls/Inc" -I../Core/Inc -I"C:/Users/mertk/STM32CubeIDE/workspace_1.18.0/CS350_Project/BOOTLOADER1/Libs/tinycrypt/Inc" -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Libs-2f-mbedtls-2f-Src

clean-Libs-2f-mbedtls-2f-Src:
	-$(RM) ./Libs/mbedtls/Src/aes.cyclo ./Libs/mbedtls/Src/aes.d ./Libs/mbedtls/Src/aes.o ./Libs/mbedtls/Src/aes.su ./Libs/mbedtls/Src/asn1parse.cyclo ./Libs/mbedtls/Src/asn1parse.d ./Libs/mbedtls/Src/asn1parse.o ./Libs/mbedtls/Src/asn1parse.su ./Libs/mbedtls/Src/bignum.cyclo ./Libs/mbedtls/Src/bignum.d ./Libs/mbedtls/Src/bignum.o ./Libs/mbedtls/Src/bignum.su ./Libs/mbedtls/Src/cipher.cyclo ./Libs/mbedtls/Src/cipher.d ./Libs/mbedtls/Src/cipher.o ./Libs/mbedtls/Src/cipher.su ./Libs/mbedtls/Src/cipher_wrap.cyclo ./Libs/mbedtls/Src/cipher_wrap.d ./Libs/mbedtls/Src/cipher_wrap.o ./Libs/mbedtls/Src/cipher_wrap.su ./Libs/mbedtls/Src/constant_time.cyclo ./Libs/mbedtls/Src/constant_time.d ./Libs/mbedtls/Src/constant_time.o ./Libs/mbedtls/Src/constant_time.su ./Libs/mbedtls/Src/ecdsa.cyclo ./Libs/mbedtls/Src/ecdsa.d ./Libs/mbedtls/Src/ecdsa.o ./Libs/mbedtls/Src/ecdsa.su ./Libs/mbedtls/Src/ecp.cyclo ./Libs/mbedtls/Src/ecp.d ./Libs/mbedtls/Src/ecp.o ./Libs/mbedtls/Src/ecp.su ./Libs/mbedtls/Src/ecp_curves.cyclo ./Libs/mbedtls/Src/ecp_curves.d ./Libs/mbedtls/Src/ecp_curves.o ./Libs/mbedtls/Src/ecp_curves.su ./Libs/mbedtls/Src/md.cyclo ./Libs/mbedtls/Src/md.d ./Libs/mbedtls/Src/md.o ./Libs/mbedtls/Src/md.su ./Libs/mbedtls/Src/oid.cyclo ./Libs/mbedtls/Src/oid.d ./Libs/mbedtls/Src/oid.o ./Libs/mbedtls/Src/oid.su ./Libs/mbedtls/Src/platform.cyclo ./Libs/mbedtls/Src/platform.d ./Libs/mbedtls/Src/platform.o ./Libs/mbedtls/Src/platform.su ./Libs/mbedtls/Src/platform_util.cyclo ./Libs/mbedtls/Src/platform_util.d ./Libs/mbedtls/Src/platform_util.o ./Libs/mbedtls/Src/platform_util.su ./Libs/mbedtls/Src/sha256.cyclo ./Libs/mbedtls/Src/sha256.d ./Libs/mbedtls/Src/sha256.o ./Libs/mbedtls/Src/sha256.su

.PHONY: clean-Libs-2f-mbedtls-2f-Src

