################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../framework/append.c \
../framework/base64.c \
../framework/case.c \
../framework/continue.c \
../framework/copy.c \
../framework/dataface.c \
../framework/duplicate.c \
../framework/extract.c \
../framework/hash.c \
../framework/hex.c \
../framework/identical.c \
../framework/ip.c \
../framework/lavacache.c \
../framework/length.c \
../framework/logging.c \
../framework/lzo.c \
../framework/network.c \
../framework/placer.c \
../framework/print.c \
../framework/qp.c \
../framework/random.c \
../framework/remove.c \
../framework/replace.c \
../framework/search.c \
../framework/sql.c \
../framework/ssl.c \
../framework/starts.c \
../framework/statements.c \
../framework/stringer.c \
../framework/tokenize.c \
../framework/unit.c \
../framework/utility.c \
../framework/xml.c 

C_DEPS += \
./framework/append.d \
./framework/base64.d \
./framework/case.d \
./framework/continue.d \
./framework/copy.d \
./framework/dataface.d \
./framework/duplicate.d \
./framework/extract.d \
./framework/hash.d \
./framework/hex.d \
./framework/identical.d \
./framework/ip.d \
./framework/lavacache.d \
./framework/length.d \
./framework/logging.d \
./framework/lzo.d \
./framework/network.d \
./framework/placer.d \
./framework/print.d \
./framework/qp.d \
./framework/random.d \
./framework/remove.d \
./framework/replace.d \
./framework/search.d \
./framework/sql.d \
./framework/ssl.d \
./framework/starts.d \
./framework/statements.d \
./framework/stringer.d \
./framework/tokenize.d \
./framework/unit.d \
./framework/utility.d \
./framework/xml.d 

OBJS += \
./framework/append.o \
./framework/base64.o \
./framework/case.o \
./framework/continue.o \
./framework/copy.o \
./framework/dataface.o \
./framework/duplicate.o \
./framework/extract.o \
./framework/hash.o \
./framework/hex.o \
./framework/identical.o \
./framework/ip.o \
./framework/lavacache.o \
./framework/length.o \
./framework/logging.o \
./framework/lzo.o \
./framework/network.o \
./framework/placer.o \
./framework/print.o \
./framework/qp.o \
./framework/random.o \
./framework/remove.o \
./framework/replace.o \
./framework/search.o \
./framework/sql.o \
./framework/ssl.o \
./framework/starts.o \
./framework/statements.o \
./framework/stringer.o \
./framework/tokenize.o \
./framework/unit.o \
./framework/utility.o \
./framework/xml.o 


# Each subdirectory must supply rules for building sources it contributes
framework/%.o: ../framework/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/include/mysql/ -I/usr/include/openssl/ -I/usr/include/lzo/ -I"/home/ladar/Lavabit/magma.so/sources/clamav/libclamav" -I/usr/include/libxml2/libxml/ -I/usr/include/libxml2/ -I"/home/ladar/Lavabit/magma.tools/testde" -O2 -g -w -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


