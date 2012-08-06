curl_search:
	gcc -o curl_search curl_search.c util.c util.h -lpcre -lcurl
file_search:
	gcc -o file_search file_search.c util.c util.h -lpcre
debug:
	gcc -DDEBUG -o file_search file_search.c -lpcre
static:
	gcc -o file_search file_search.c -lpcre -lpthread -static
clean:
	rm -rf file_search curl_search
