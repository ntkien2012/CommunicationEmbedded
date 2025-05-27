################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/MAVLink/MAVLinkV1.c 

OBJS += \
./Middlewares/MAVLink/MAVLinkV1.o 

C_DEPS += \
./Middlewares/MAVLink/MAVLinkV1.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/MAVLink/%.o Middlewares/MAVLink/%.su: ../Middlewares/MAVLink/%.c Middlewares/MAVLink/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I"E:/STM32CubeIDE/Data_communication_project/Core/Inc" -I"E:/STM32CubeIDE/Data_communication_project/Middlewares/MAVLink" -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Middlewares-2f-MAVLink

clean-Middlewares-2f-MAVLink:
	-$(RM) ./Middlewares/MAVLink/MAVLinkV1.d ./Middlewares/MAVLink/MAVLinkV1.o ./Middlewares/MAVLink/MAVLinkV1.su

.PHONY: clean-Middlewares-2f-MAVLink

