################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../engine/context/args.c \
../engine/context/process.c \
../engine/context/sanity.c \
../engine/context/signal.c \
../engine/context/system.c \
../engine/context/thread.c 

OBJS += \
./engine/context/args.o \
./engine/context/process.o \
./engine/context/sanity.o \
./engine/context/signal.o \
./engine/context/system.o \
./engine/context/thread.o 

C_DEPS += \
./engine/context/args.d \
./engine/context/process.d \
./engine/context/sanity.d \
./engine/context/signal.d \
./engine/context/system.d \
./engine/context/thread.d 


# Each subdirectory must supply rules for building sources it contributes
engine/context/%.o: ../engine/context/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DMAGMA_PEDANTIC -D_REENTRANT -D_GNU_SOURCE -DFORTIFY_SOURCE=2 -DHAVE_NS_TYPE -D_LARGEFILE64_SOURCE -I../../src -I../../lib/sources/clamav/libclamav -I../../lib/sources/mysql/include -I../../lib/sources/openssl/include/openssl -I../../lib/sources/openssl/include -I../../lib/sources/tokyocabinet -I../../lib/sources/spf2/src/include -I../../lib/sources/xml2/include/libxml -I../../lib/sources/xml2/include -I../../lib/sources/lzo/include/lzo -I../../lib/sources/lzo/include -I../../lib/sources/bzip2 -I../../lib/sources/zlib -I../../lib/sources/curl/include/curl -I../../lib/sources/curl/include -I../../lib/sources/memcached -I../../lib/sources/geoip/libGeoIP -I../../lib/sources/dkim/libopendkim/ -I../../lib/sources/dspam/src -I../../lib/sources/jansson/src -I../../lib/sources/gd -I../../lib/sources/jpeg -I../../lib/sources/freetype/include/ -I../../lib/sources/png -O0 -g3 -rdynamic -Wall -Werror -c -fmessage-length=0 -std=gnu99 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


