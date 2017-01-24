
all: cryptex

cryptex:
	gcc -I../../../lib/local/include/openssl/ -lcrypto -lssl -std=gnu99 -O3 -o cryptex cryptex.c keys.c secure.c ecies.c

.PHONY: all
