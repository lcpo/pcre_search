curl_search:
	gcc -Wall -o curl_search curl_search.c util.c util.h -lpcre -lcurl
file_search:
	gcc -Wall -o file_search file_search.c util.c util.h -lpcre
debug:
	gcc -Wall -DDEBUG -o file_search file_search.c -lpcre
static:
	gcc -Wall -o file_search file_search.c -lpcre -lpthread -static
clean:
	rm -rf file_search curl_search
