################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cryptex.c \
../ecies.c \
../keys.c \
../secure.c 

OBJS += \
./cryptex.o \
./ecies.o \
./keys.o \
./secure.o 

C_DEPS += \
./cryptex.d \
./ecies.d \
./keys.d \
./secure.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/ladar/Lavabit/magma.so/sources/openssl/include/openssl" -I"/home/ladar/Lavabit/magma.so/sources/openssl/include" -O0 -g3 -rdynamic -Wall -Werror -c -fmessage-length=0 -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


