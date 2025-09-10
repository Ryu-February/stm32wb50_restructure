################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/buzzer/buzzer.c 

OBJS += \
./App/buzzer/buzzer.o 

C_DEPS += \
./App/buzzer/buzzer.d 


# Each subdirectory must supply rules for building sources it contributes
App/buzzer/%.o App/buzzer/%.su App/buzzer/%.cyclo: ../App/buzzer/%.c App/buzzer/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32WB50xx -c -I../Core/Inc -I../Drivers/STM32WBxx_HAL_Driver/Inc -I../Drivers/STM32WBxx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../Drivers/CMSIS/Include -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/ap" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/ir" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/common" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/led" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/rgb" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/UserDrivers/bsp/i2c" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/UserDrivers/bsp/uart" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/UserDrivers/components" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/UserDrivers/components/bh1745" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/UserDrivers/components/flash" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/color" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/input" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-buzzer

clean-App-2f-buzzer:
	-$(RM) ./App/buzzer/buzzer.cyclo ./App/buzzer/buzzer.d ./App/buzzer/buzzer.o ./App/buzzer/buzzer.su

.PHONY: clean-App-2f-buzzer

