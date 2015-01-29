################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../servers/smtp/accept.c \
../servers/smtp/checkers.c \
../servers/smtp/commands.c \
../servers/smtp/datatier.c \
../servers/smtp/messages.c \
../servers/smtp/parse.c \
../servers/smtp/relay.c \
../servers/smtp/session.c \
../servers/smtp/smtp.c \
../servers/smtp/transmit.c 

OBJS += \
./servers/smtp/accept.o \
./servers/smtp/checkers.o \
./servers/smtp/commands.o \
./servers/smtp/datatier.o \
./servers/smtp/messages.o \
./servers/smtp/parse.o \
./servers/smtp/relay.o \
./servers/smtp/session.o \
./servers/smtp/smtp.o \
./servers/smtp/transmit.o 

C_DEPS += \
./servers/smtp/accept.d \
./servers/smtp/checkers.d \
./servers/smtp/commands.d \
./servers/smtp/datatier.d \
./servers/smtp/messages.d \
./servers/smtp/parse.d \
./servers/smtp/relay.d \
./servers/smtp/session.d \
./servers/smtp/smtp.d \
./servers/smtp/transmit.d 


# Each subdirectory must supply rules for building sources it contributes
servers/smtp/%.o: ../servers/smtp/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DMAGMA_PEDANTIC -D_REENTRANT -D_GNU_SOURCE -DFORTIFY_SOURCE=2 -DHAVE_NS_TYPE -D_LARGEFILE64_SOURCE -I../../src -I../../lib/sources/clamav/libclamav -I../../lib/sources/mysql/include -I../../lib/sources/openssl/include/openssl -I../../lib/sources/openssl/include -I../../lib/sources/tokyocabinet -I../../lib/sources/spf2/src/include -I../../lib/sources/xml2/include/libxml -I../../lib/sources/xml2/include -I../../lib/sources/lzo/include/lzo -I../../lib/sources/lzo/include -I../../lib/sources/bzip2 -I../../lib/sources/zlib -I../../lib/sources/curl/include/curl -I../../lib/sources/curl/include -I../../lib/sources/memcached -I../../lib/sources/geoip/libGeoIP -I../../lib/sources/dkim/libopendkim/ -I../../lib/sources/dspam/src -I../../lib/sources/jansson/src -I../../lib/sources/gd -I../../lib/sources/jpeg -I../../lib/sources/freetype/include/ -I../../lib/sources/png -O0 -g3 -rdynamic -Wall -Werror -c -fmessage-length=0 -std=gnu99 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


