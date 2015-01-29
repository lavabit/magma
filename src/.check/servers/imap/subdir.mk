################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../servers/imap/commands.c \
../servers/imap/fetch.c \
../servers/imap/fetch_response.c \
../servers/imap/flags.c \
../servers/imap/folders.c \
../servers/imap/imap.c \
../servers/imap/messages.c \
../servers/imap/output.c \
../servers/imap/parse.c \
../servers/imap/parse_address.c \
../servers/imap/range.c \
../servers/imap/search.c \
../servers/imap/sessions.c 

OBJS += \
./servers/imap/commands.o \
./servers/imap/fetch.o \
./servers/imap/fetch_response.o \
./servers/imap/flags.o \
./servers/imap/folders.o \
./servers/imap/imap.o \
./servers/imap/messages.o \
./servers/imap/output.o \
./servers/imap/parse.o \
./servers/imap/parse_address.o \
./servers/imap/range.o \
./servers/imap/search.o \
./servers/imap/sessions.o 

C_DEPS += \
./servers/imap/commands.d \
./servers/imap/fetch.d \
./servers/imap/fetch_response.d \
./servers/imap/flags.d \
./servers/imap/folders.d \
./servers/imap/imap.d \
./servers/imap/messages.d \
./servers/imap/output.d \
./servers/imap/parse.d \
./servers/imap/parse_address.d \
./servers/imap/range.d \
./servers/imap/search.d \
./servers/imap/sessions.d 


# Each subdirectory must supply rules for building sources it contributes
servers/imap/%.o: ../servers/imap/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DMAGMA_PEDANTIC -D_REENTRANT -D_GNU_SOURCE -DFORTIFY_SOURCE=2 -DHAVE_NS_TYPE -D_LARGEFILE64_SOURCE -I../../src -I../../lib/sources/clamav/libclamav -I../../lib/sources/openssl/include/openssl -I../../lib/sources/openssl/include -I../../lib/sources/tokyocabinet -I../../lib/sources/spf2/src/include -I../../lib/sources/xml2/include/libxml -I../../lib/sources/xml2/include -I../../lib/sources/lzo/include/lzo -I../../lib/sources/lzo/include -I../../lib/sources/bzip2 -I../../lib/sources/zlib -I../../lib/sources/curl/include/curl -I../../lib/sources/curl/include -I../../lib/sources/memcached -I../../lib/sources/geoip/libGeoIP -I../../lib/sources/dkim/libopendkim/ -I../../lib/sources/dspam/src -I../../lib/sources/jansson/src -I../../lib/sources/gd -I../../lib/sources/jpeg -I../../lib/sources/freetype/include/ -I../../lib/sources/png -I../../lib/sources/mysql/include -O0 -g3 -rdynamic -Wall -Werror -c -fmessage-length=0 -std=gnu99 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


