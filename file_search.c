#include <stdio.h>  // file/stream IO
#include <stdlib.h> // calloc,free
#include <string.h> // strlen
#include <pcre.h>

#define OVECCOUNT 30 // should be multiple of 3

int main(int argc, char **argv) {

  if(argc<3) { puts("usage: file_search <file> <pattern>"); return 1; }

  FILE *file = fopen(argv[1], "r");
  if(file==NULL) { fprintf(stderr, "error: could not open file"); return 1; }
  
  fseek(file, 0L, SEEK_END);        // seek to EOF
  size_t file_length = ftell(file); // store EOF pos
  rewind(file);

  // allocate and zero file buffer
  char *file_buffer = calloc(file_length + 1, sizeof(char));

  // read file into buffer
  fread(file_buffer, file_length, 1, file);

  ///////////////////////////////////////
  // Do something with the file_buffer //
  ///////////////////////////////////////

  // file_length and strlen(file_buffer) are the same see?
  // printf("strlen(file_buffer) = %d\n", (int)strlen(file_buffer));
  // printf("        file_length = %d\n", (int)file_length);

  char *pattern = argv[2];
  pcre *re;
  const char *error;
  int  erroroffset;

  re = pcre_compile(
    pattern,       // the pattern :)
    0,             // default options
    &error,        // for errror message
    &erroroffset,  // for error offset
    NULL);

  if(re==NULL) // regex compile failed, handle error case
  {
    fprintf(stderr, "error: PCRE compile failed at offset %d: %s\n", erroroffset, error);
    return 1;
  }

  int rc;
  int ovector[OVECCOUNT];
  rc = pcre_exec(
    re,            // the compiled pattern
    NULL,          // no extra data
    file_buffer,   // the file_buffer
    file_length,   // file length
    0,             // start at offset 0 in file_buffer
    0,             // default options
    ovector,       // output vector for substring info
    OVECCOUNT);

  if(rc<0) // match failed, handle error cases
  {
    switch(rc)
    {
      case PCRE_ERROR_NOMATCH: puts("No match."); break;
      default: fprintf(stderr,"error: matching error."); break;
    }
    pcre_free(re);
    return 1;
  }

  printf("Match succeeded at offset %d\n", ovector[0]);

  if(rc==0) // ovector wasn't big enough, handle error case
  {
    rc = OVECCOUNT/3;
    fprintf(stderr,"error: ovector only has room for %d substrings\n", rc-1);
  }

  // Show substrings stored in output vector
  int i;
  for(i=0;i<rc;i++)
  {
    char *substring_start = file_buffer + ovector[2*i];
    int substring_length = ovector[2*i+1] - ovector[2*i];
    printf("%2d: %.*s\n", i, substring_length, substring_start);
  }

  // cleanup
  free(file_buffer);
  close(file);

  return 0;
}
