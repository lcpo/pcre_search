debug:
	gcc -DDEBUG -o file_search file_search.c -lpcre
static:
	gcc -o file_search file_search.c -lpcre -lpthread -static
file_search:
	gcc -o file_search file_search.c -lpcre
clean:
	rm -rf file_search
