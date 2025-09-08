################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/ap/ap.c \
../App/ap/ap_isr.c 

OBJS += \
./App/ap/ap.o \
./App/ap/ap_isr.o 

C_DEPS += \
./App/ap/ap.d \
./App/ap/ap_isr.d 


# Each subdirectory must supply rules for building sources it contributes
App/ap/%.o App/ap/%.su App/ap/%.cyclo: ../App/ap/%.c App/ap/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32WB50xx -c -I../Core/Inc -I../Drivers/STM32WBxx_HAL_Driver/Inc -I../Drivers/STM32WBxx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../Drivers/CMSIS/Include -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/ap" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/ir" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/stepper" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/common" -I"C:/Workspace/STM32WB50_HANGIL/STM32WB_Hangil_restructure/App/led" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-ap

clean-App-2f-ap:
	-$(RM) ./App/ap/ap.cyclo ./App/ap/ap.d ./App/ap/ap.o ./App/ap/ap.su ./App/ap/ap_isr.cyclo ./App/ap/ap_isr.d ./App/ap/ap_isr.o ./App/ap/ap_isr.su

.PHONY: clean-App-2f-ap

