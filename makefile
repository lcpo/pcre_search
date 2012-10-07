all: curl_search ifl
curl_search:
	gcc -Wall -o curl_search curl_search.c util.c util.h -lpcre -lcurl
ifl:
	gcc -Wall -o ifl ifl.c util.c util.h -lpcre -lcurl
clean:
	rm -rf file_search curl_search ifl

# others
file_search:
	gcc -Wall -o file_search file_search.c util.c util.h -lpcre
debug:
	gcc -Wall -DDEBUG -o ifl ifl.c util.c util.h -lpcre -lcurl
	gcc -Wall -DDEBUG -o curl_search curl_search.c util.c util.h -lpcre -lcurl
static:
	gcc -o curl_search curl_search.c util.c util.h -static-libgcc -lpcre -lcurl -lssh2 -lrt -lssl -lcrypto -lz -lpthread -ldl -static
