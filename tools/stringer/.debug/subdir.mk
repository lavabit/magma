################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../case.c \
../main.c \
../memory.c \
../nuller.c \
../stringer-allocation.c \
../stringer-check.c \
../stringer-compare.c \
../stringer-data.c \
../stringer-length.c \
../stringer-misc.c \
../stringer-secure.c 

OBJS += \
./case.o \
./main.o \
./memory.o \
./nuller.o \
./stringer-allocation.o \
./stringer-check.o \
./stringer-compare.o \
./stringer-data.o \
./stringer-length.o \
./stringer-misc.o \
./stringer-secure.o 

C_DEPS += \
./case.d \
./main.d \
./memory.d \
./nuller.d \
./stringer-allocation.d \
./stringer-check.d \
./stringer-compare.d \
./stringer-data.d \
./stringer-length.d \
./stringer-misc.d \
./stringer-secure.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -DMAGMA_PEDANTIC -O0 -g3 -pedantic -Wall -c -fmessage-length=0 -std=gnu99 -Wextra -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


