CC = gcc
CFLAGS = -DMAGMA_PEDANTIC -D_REENTRANT -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -DHAVE_NS_TYPE -D_LARGEFILE64_SOURCE -O0 -g3 -Wall -Werror -fmessage-length=0 -std=gnu99
MAGMA_SO_PATH = ../../lib
MAGMA_PATH = ../../src
APPNAME = pwtool 
LIBRARIES = -ldl -lrt -lpthread

all:	pwtool.c
	$(CC) $(CFLAGS) -I"$(MAGMA_PATH)" -I"$(MAGMA_SO_PATH)/sources/clamav/libclamav" -I"$(MAGMA_SO_PATH)/sources/mysql/include" -I"$(MAGMA_SO_PATH)/sources/openssl/include/openssl" -I"$(MAGMA_SO_PATH)/sources/openssl/include" -I"$(MAGMA_SO_PATH)/sources/tokyocabinet" -I"$(MAGMA_SO_PATH)/sources/spf2/src/include" -I"$(MAGMA_SO_PATH)/sources/xml2/include/libxml" -I"$(MAGMA_SO_PATH)/sources/xml2/include" -I"$(MAGMA_SO_PATH)/sources/lzo/include/lzo" -I"$(MAGMA_SO_PATH)/sources/lzo/include" -I"$(MAGMA_SO_PATH)/sources/bzip2" -I"$(MAGMA_SO_PATH)/sources/zlib" -I"$(MAGMA_SO_PATH)/sources/memcached" -I"$(MAGMA_SO_PATH)/sources/dkim/libopendkim/" -I"$(MAGMA_SO_PATH)/sources/dspam/src" -I"$(MAGMA_SO_PATH)/sources/jansson/src" -I"$(MAGMA_SO_PATH)/sources/gd" -I"$(MAGMA_SO_PATH)/sources/freetype/include/freetype" -I"$(MAGMA_SO_PATH)/sources/freetype/include/" -include"$(MAGMA_PATH)/magma.h" -o $(APPNAME) pwtool.c $(MAGMA_PATH)/.check/libmagma.a $(LIBRARIES)

clean:	$(APPNAME)
	rm $(APPNAME)
