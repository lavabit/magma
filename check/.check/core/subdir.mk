################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../core/base64_check.c \
../core/core_check.c \
../core/hashed_check.c \
../core/hex_check.c \
../core/inx_check.c \
../core/linked_check.c \
../core/qp_check.c \
../core/qsort_check.c \
../core/string_check.c \
../core/system_check.c \
../core/tree_check.c \
../core/url_check.c \
../core/zbase32_check.c 

OBJS += \
./core/base64_check.o \
./core/core_check.o \
./core/hashed_check.o \
./core/hex_check.o \
./core/inx_check.o \
./core/linked_check.o \
./core/qp_check.o \
./core/qsort_check.o \
./core/string_check.o \
./core/system_check.o \
./core/tree_check.o \
./core/url_check.o \
./core/zbase32_check.o 

C_DEPS += \
./core/base64_check.d \
./core/core_check.d \
./core/hashed_check.d \
./core/hex_check.d \
./core/inx_check.d \
./core/linked_check.d \
./core/qp_check.d \
./core/qsort_check.d \
./core/string_check.d \
./core/system_check.d \
./core/tree_check.d \
./core/url_check.d \
./core/zbase32_check.d 


# Each subdirectory must supply rules for building sources it contributes
core/%.o: ../core/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DMAGMA_PEDANTIC -D_REENTRANT -D_GNU_SOURCE -DFORTIFY_SOURCE=2 -DHAVE_NS_TYPE -D_LARGEFILE64_SOURCE -I../../src -I/usr/include/valgrind -I../../check -I../../lib/sources/clamav/libclamav -I../../lib/sources/mysql/include -I../../lib/sources/openssl/include/openssl -I../../lib/sources/openssl/include -I../../lib/sources/tokyocabinet -I../../lib/sources/spf2/src/include -I../../lib/sources/xml2/include/libxml -I../../lib/sources/lzo/include/lzo -I../../lib/sources/xml2/include -I../../lib/sources/lzo/include -I../../lib/sources/bzip2 -I../../lib/sources/zlib -I../../lib/sources/curl/include/curl -I../../lib/sources/curl/include -I../../lib/sources/memcached -I../../lib/sources/geoip/libGeoIP -I../../lib/sources/dkim/libopendkim/ -I../../lib/sources/dspam/src -I../../lib/sources/jansson/src -I../../lib/sources/gd -I../../lib/sources/png -I../../lib/sources/jpeg -I../../lib/sources/freetype/include/ -include"/home/ladar/git/magma.distribution/src/magma.h" -O0 -g3 -rdynamic -Wall -Werror -c -fmessage-length=0 -std=gnu99 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


