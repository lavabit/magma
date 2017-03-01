
all: testde

clean:
	rm --force testde

testde:
	 gcc -o testde -I../../../lib/local/include/ -I../../../lib/local/include/mysql -I../../../lib/local/include/openssl -I../../../lib/local/include/libxml2/libxml -I../../../lib/local/include/libxml2 -I../../../lib/local/include/lzo -I./ framework/append.c framework/base64.c framework/case.c framework/continue.c framework/copy.c framework/dataface.c framework/duplicate.c framework/extract.c framework/hash.c framework/hex.c framework/identical.c framework/ip.c framework/lavacache.c framework/length.c framework/logging.c framework/lzo.c framework/network.c framework/placer.c framework/print.c framework/qp.c framework/random.c framework/remove.c framework/replace.c framework/search.c framework/sql.c framework/ssl.c framework/starts.c framework/statements.c framework/stringer.c framework/tokenize.c framework/unit.c framework/utility.c framework/xml.c main.c -ldl -lpthread

.PHONY: all clean
