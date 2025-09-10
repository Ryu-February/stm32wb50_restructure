################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/color/color.c 

OBJS += \
./App/color/color.o 

C_DEPS += \
./App/color/color.d 


# Each subdirectory must supply rules for building sources it contributes
App/color/%.o App/color/%.su App/color/%.cyclo: ../App/color/%.c App/color/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32WB50xx -c -I../Core/Inc -I../Drivers/STM32WBxx_HAL_Driver/Inc -I../Drivers/STM32WBxx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../Drivers/CMSIS/Include -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/ap" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/ir" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/common" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/led" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/rgb" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/UserDrivers/bsp/i2c" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/UserDrivers/bsp/uart" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/UserDrivers/components" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/UserDrivers/components/bh1745" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/UserDrivers/components/flash" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/color" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/input" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-color

clean-App-2f-color:
	-$(RM) ./App/color/color.cyclo ./App/color/color.d ./App/color/color.o ./App/color/color.su

.PHONY: clean-App-2f-color

