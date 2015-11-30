################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../main.c 

C_DEPS += \
./main.d 

OBJS += \
./main.o 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/include/mysql/ -I/usr/include/openssl/ -I/usr/include/lzo/ -I"/home/ladar/Lavabit/magma.so/sources/clamav/libclamav" -I/usr/include/libxml2/libxml/ -I/usr/include/libxml2/ -I"/home/ladar/Lavabit/magma.tools/testde" -O2 -g -w -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


