################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../main.c 

OBJS += \
./main.o 

C_DEPS += \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/ladar/Lavabit/magma/lib/sources/freetype/include" -I"/home/ladar/Lavabit/magma/lib/sources/jpeg" -I"/home/ladar/Lavabit/magma/lib/sources/png" -I"/home/ladar/Lavabit/magma/lib/sources/gd" -I"/home/ladar/Lavabit/magma/lib/sources/jansson/src" -I"/home/ladar/Lavabit/magma/lib/sources/dspam/src" -I"/home/ladar/Lavabit/magma/lib/sources/dkim/libopendkim" -I"/home/ladar/Lavabit/magma/lib/sources/geoip/libGeoIP" -I"/home/ladar/Lavabit/magma/lib/sources/memcached" -I"/home/ladar/Lavabit/magma/lib/sources/curl/include" -I"/home/ladar/Lavabit/magma/lib/sources/curl/include/curl" -I"/home/ladar/Lavabit/magma/lib/sources/zlib" -I"/home/ladar/Lavabit/magma/lib/sources/bzip2" -I"/home/ladar/Lavabit/magma/lib/sources/lzo/include" -I"/home/ladar/Lavabit/magma/lib/sources/lzo/include/lzo" -I"/home/ladar/Lavabit/magma/lib/sources/xml2/include" -I"/home/ladar/Lavabit/magma/lib/sources/xml2/include/libxml" -I"/home/ladar/Lavabit/magma/lib/sources/spf2/src/include" -I"/home/ladar/Lavabit/magma/lib/sources/tokyocabinet" -I"/home/ladar/Lavabit/magma/lib/sources/openssl/include" -I"/home/ladar/Lavabit/magma/lib/sources/openssl/include/openssl" -I"/home/ladar/Lavabit/magma/lib/sources/mysql/include" -I"/home/ladar/Lavabit/magma/lib/sources/clamav/libclamav" -I"/home/ladar/Lavabit/magma" -I"/home/ladar/Lavabit/magma/dev/tools/testde" -O2 -g -w -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


