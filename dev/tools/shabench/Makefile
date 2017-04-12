
all: shabench

shabench: shabench.c
	gcc -lcrypto -lssl -std=gnu99 -O3 -o shabench shabench.c
	
clean:
	rm -f shabench

.PHONY: all clean
