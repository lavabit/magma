################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../providers/compress_check.c \
../providers/digest_check.c \
../providers/dspam_check.c \
../providers/ecies_check.c \
../providers/provide_check.c \
../providers/rand_check.c \
../providers/scramble_check.c \
../providers/symmetric_check.c \
../providers/tank_check.c \
../providers/virus_check.c 

OBJS += \
./providers/compress_check.o \
./providers/digest_check.o \
./providers/dspam_check.o \
./providers/ecies_check.o \
./providers/provide_check.o \
./providers/rand_check.o \
./providers/scramble_check.o \
./providers/symmetric_check.o \
./providers/tank_check.o \
./providers/virus_check.o 

C_DEPS += \
./providers/compress_check.d \
./providers/digest_check.d \
./providers/dspam_check.d \
./providers/ecies_check.d \
./providers/provide_check.d \
./providers/rand_check.d \
./providers/scramble_check.d \
./providers/symmetric_check.d \
./providers/tank_check.d \
./providers/virus_check.d 


# Each subdirectory must supply rules for building sources it contributes
providers/%.o: ../providers/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DMAGMA_PEDANTIC -D_REENTRANT -D_GNU_SOURCE -DFORTIFY_SOURCE=2 -DHAVE_NS_TYPE -D_LARGEFILE64_SOURCE -I../../src -I/usr/include/valgrind -I../../check -I../../lib/sources/clamav/libclamav -I../../lib/sources/mysql/include -I../../lib/sources/openssl/include/openssl -I../../lib/sources/openssl/include -I../../lib/sources/tokyocabinet -I../../lib/sources/spf2/src/include -I../../lib/sources/xml2/include/libxml -I../../lib/sources/lzo/include/lzo -I../../lib/sources/xml2/include -I../../lib/sources/lzo/include -I../../lib/sources/bzip2 -I../../lib/sources/zlib -I../../lib/sources/curl/include/curl -I../../lib/sources/curl/include -I../../lib/sources/memcached -I../../lib/sources/geoip/libGeoIP -I../../lib/sources/dkim/libopendkim/ -I../../lib/sources/dspam/src -I../../lib/sources/jansson/src -I../../lib/sources/gd -I../../lib/sources/png -I../../lib/sources/jpeg -I../../lib/sources/freetype/include/ -include"/home/ladar/git/magma.distribution/src/magma.h" -O0 -g3 -rdynamic -Wall -Werror -c -fmessage-length=0 -std=gnu99 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


