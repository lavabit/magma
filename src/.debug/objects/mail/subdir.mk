################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../objects/mail/cache.c \
../objects/mail/cleanup.c \
../objects/mail/counters.c \
../objects/mail/datatier.c \
../objects/mail/headers.c \
../objects/mail/load_message.c \
../objects/mail/mime.c \
../objects/mail/objects.c \
../objects/mail/parsing.c \
../objects/mail/paths.c \
../objects/mail/remove_message.c \
../objects/mail/signatures.c \
../objects/mail/store_message.c 

OBJS += \
./objects/mail/cache.o \
./objects/mail/cleanup.o \
./objects/mail/counters.o \
./objects/mail/datatier.o \
./objects/mail/headers.o \
./objects/mail/load_message.o \
./objects/mail/mime.o \
./objects/mail/objects.o \
./objects/mail/parsing.o \
./objects/mail/paths.o \
./objects/mail/remove_message.o \
./objects/mail/signatures.o \
./objects/mail/store_message.o 

C_DEPS += \
./objects/mail/cache.d \
./objects/mail/cleanup.d \
./objects/mail/counters.d \
./objects/mail/datatier.d \
./objects/mail/headers.d \
./objects/mail/load_message.d \
./objects/mail/mime.d \
./objects/mail/objects.d \
./objects/mail/parsing.d \
./objects/mail/paths.d \
./objects/mail/remove_message.d \
./objects/mail/signatures.d \
./objects/mail/store_message.d 


# Each subdirectory must supply rules for building sources it contributes
objects/mail/%.o: ../objects/mail/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DMAGMA_PEDANTIC -D_REENTRANT -D_GNU_SOURCE -DFORTIFY_SOURCE=2 -DHAVE_NS_TYPE -D_LARGEFILE64_SOURCE -I../../src -I../../lib/sources/clamav/libclamav -I../../lib/sources/mysql/include -I../../lib/sources/openssl/include/openssl -I../../lib/sources/openssl/include -I../../lib/sources/tokyocabinet -I../../lib/sources/spf2/src/include -I../../lib/sources/xml2/include/libxml -I../../lib/sources/xml2/include -I../../lib/sources/lzo/include/lzo -I../../lib/sources/lzo/include -I../../lib/sources/bzip2 -I../../lib/sources/zlib -I../../lib/sources/curl/include/curl -I../../lib/sources/curl/include -I../../lib/sources/memcached -I../../lib/sources/geoip/libGeoIP -I../../lib/sources/dkim/libopendkim/ -I../../lib/sources/dspam/src -I../../lib/sources/jansson/src -I../../lib/sources/gd -I../../lib/sources/jpeg -I../../lib/sources/freetype/include/ -I../../lib/sources/png -O0 -g3 -rdynamic -Wall -Werror -c -fmessage-length=0 -std=gnu99 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


