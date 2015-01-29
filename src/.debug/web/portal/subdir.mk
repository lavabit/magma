################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../web/portal/config.c \
../web/portal/contacts.c \
../web/portal/endpoint.c \
../web/portal/flags.c \
../web/portal/folders.c \
../web/portal/mail.c \
../web/portal/messages.c \
../web/portal/parse.c \
../web/portal/portal.c 

OBJS += \
./web/portal/config.o \
./web/portal/contacts.o \
./web/portal/endpoint.o \
./web/portal/flags.o \
./web/portal/folders.o \
./web/portal/mail.o \
./web/portal/messages.o \
./web/portal/parse.o \
./web/portal/portal.o 

C_DEPS += \
./web/portal/config.d \
./web/portal/contacts.d \
./web/portal/endpoint.d \
./web/portal/flags.d \
./web/portal/folders.d \
./web/portal/mail.d \
./web/portal/messages.d \
./web/portal/parse.d \
./web/portal/portal.d 


# Each subdirectory must supply rules for building sources it contributes
web/portal/%.o: ../web/portal/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DMAGMA_PEDANTIC -D_REENTRANT -D_GNU_SOURCE -DFORTIFY_SOURCE=2 -DHAVE_NS_TYPE -D_LARGEFILE64_SOURCE -I../../src -I../../lib/sources/clamav/libclamav -I../../lib/sources/mysql/include -I../../lib/sources/openssl/include/openssl -I../../lib/sources/openssl/include -I../../lib/sources/tokyocabinet -I../../lib/sources/spf2/src/include -I../../lib/sources/xml2/include/libxml -I../../lib/sources/xml2/include -I../../lib/sources/lzo/include/lzo -I../../lib/sources/lzo/include -I../../lib/sources/bzip2 -I../../lib/sources/zlib -I../../lib/sources/curl/include/curl -I../../lib/sources/curl/include -I../../lib/sources/memcached -I../../lib/sources/geoip/libGeoIP -I../../lib/sources/dkim/libopendkim/ -I../../lib/sources/dspam/src -I../../lib/sources/jansson/src -I../../lib/sources/gd -I../../lib/sources/jpeg -I../../lib/sources/freetype/include/ -I../../lib/sources/png -O0 -g3 -rdynamic -Wall -Werror -c -fmessage-length=0 -std=gnu99 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


