################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../core/encodings/base64.c \
../core/encodings/hex.c \
../core/encodings/mappings.c \
../core/encodings/qp.c \
../core/encodings/url.c \
../core/encodings/zbase32.c 

OBJS += \
./core/encodings/base64.o \
./core/encodings/hex.o \
./core/encodings/mappings.o \
./core/encodings/qp.o \
./core/encodings/url.o \
./core/encodings/zbase32.o 

C_DEPS += \
./core/encodings/base64.d \
./core/encodings/hex.d \
./core/encodings/mappings.d \
./core/encodings/qp.d \
./core/encodings/url.d \
./core/encodings/zbase32.d 


# Each subdirectory must supply rules for building sources it contributes
core/encodings/%.o: ../core/encodings/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DMAGMA_PEDANTIC -D_REENTRANT -D_GNU_SOURCE -DFORTIFY_SOURCE=2 -DHAVE_NS_TYPE -D_LARGEFILE64_SOURCE -I../../src -I../../lib/sources/clamav/libclamav -I../../lib/sources/openssl/include/openssl -I../../lib/sources/openssl/include -I../../lib/sources/tokyocabinet -I../../lib/sources/spf2/src/include -I../../lib/sources/xml2/include/libxml -I../../lib/sources/xml2/include -I../../lib/sources/lzo/include/lzo -I../../lib/sources/lzo/include -I../../lib/sources/bzip2 -I../../lib/sources/zlib -I../../lib/sources/curl/include/curl -I../../lib/sources/curl/include -I../../lib/sources/memcached -I../../lib/sources/geoip/libGeoIP -I../../lib/sources/dkim/libopendkim/ -I../../lib/sources/dspam/src -I../../lib/sources/jansson/src -I../../lib/sources/gd -I../../lib/sources/jpeg -I../../lib/sources/freetype/include/ -I../../lib/sources/png -I../../lib/sources/mysql/include -O0 -g3 -rdynamic -Wall -Werror -c -fmessage-length=0 -std=gnu99 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


