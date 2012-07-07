#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {

  if(argc<2) return 1;

  FILE *file = fopen(argv[1], "r");
  if(file==NULL) { puts("err: could not open file"); return 1; }
  
  printf("opened: %s\n", argv[1]);

  fseek(file, 0L, SEEK_END);   // seek to EOF
  size_t file_length = ftell(file); // store EOF pos
  rewind(file);

  char *file_buffer = calloc(file_length + 1, sizeof(char));

  fread(file_buffer, file_length, 1, file);

  puts(file_buffer);

  free(file_buffer);

//  char *line = NULL;
//  size_t len = 0;

//  while(getline(&line, &len, file) != -1)
//  {
//    printf("line: %s", line);
//  }

//  free(line);

  close(file);
  printf("closed: %s\n", argv[1]);

  return 0;

}
