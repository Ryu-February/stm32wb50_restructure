################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/rgb/rgb.c 

OBJS += \
./App/rgb/rgb.o 

C_DEPS += \
./App/rgb/rgb.d 


# Each subdirectory must supply rules for building sources it contributes
App/rgb/%.o App/rgb/%.su App/rgb/%.cyclo: ../App/rgb/%.c App/rgb/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32WB50xx -c -I../Core/Inc -I../Drivers/STM32WBxx_HAL_Driver/Inc -I../Drivers/STM32WBxx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../Drivers/CMSIS/Include -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/ap" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/ir" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/stepper" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/common" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/led" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-rgb

clean-App-2f-rgb:
	-$(RM) ./App/rgb/rgb.cyclo ./App/rgb/rgb.d ./App/rgb/rgb.o ./App/rgb/rgb.su

.PHONY: clean-App-2f-rgb

