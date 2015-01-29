################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../core/strings/allocation.c \
../core/strings/data.c \
../core/strings/info.c \
../core/strings/length.c \
../core/strings/multi.c \
../core/strings/nuller.c \
../core/strings/opts.c \
../core/strings/print.c \
../core/strings/replace.c \
../core/strings/shortcuts.c \
../core/strings/validate.c 

OBJS += \
./core/strings/allocation.o \
./core/strings/data.o \
./core/strings/info.o \
./core/strings/length.o \
./core/strings/multi.o \
./core/strings/nuller.o \
./core/strings/opts.o \
./core/strings/print.o \
./core/strings/replace.o \
./core/strings/shortcuts.o \
./core/strings/validate.o 

C_DEPS += \
./core/strings/allocation.d \
./core/strings/data.d \
./core/strings/info.d \
./core/strings/length.d \
./core/strings/multi.d \
./core/strings/nuller.d \
./core/strings/opts.d \
./core/strings/print.d \
./core/strings/replace.d \
./core/strings/shortcuts.d \
./core/strings/validate.d 


# Each subdirectory must supply rules for building sources it contributes
core/strings/%.o: ../core/strings/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DMAGMA_PEDANTIC -D_REENTRANT -D_GNU_SOURCE -DFORTIFY_SOURCE=2 -DHAVE_NS_TYPE -D_LARGEFILE64_SOURCE -I../../src -I../../lib/sources/clamav/libclamav -I../../lib/sources/openssl/include/openssl -I../../lib/sources/openssl/include -I../../lib/sources/tokyocabinet -I../../lib/sources/spf2/src/include -I../../lib/sources/xml2/include/libxml -I../../lib/sources/xml2/include -I../../lib/sources/lzo/include/lzo -I../../lib/sources/lzo/include -I../../lib/sources/bzip2 -I../../lib/sources/zlib -I../../lib/sources/curl/include/curl -I../../lib/sources/curl/include -I../../lib/sources/memcached -I../../lib/sources/geoip/libGeoIP -I../../lib/sources/dkim/libopendkim/ -I../../lib/sources/dspam/src -I../../lib/sources/jansson/src -I../../lib/sources/gd -I../../lib/sources/jpeg -I../../lib/sources/freetype/include/ -I../../lib/sources/png -I../../lib/sources/mysql/include -O0 -g3 -rdynamic -Wall -Werror -c -fmessage-length=0 -std=gnu99 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


