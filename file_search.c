#include <stdio.h>  // io
#include <stdlib.h> // calloc,free,etc.
#include <string.h> // strlen
#include <fcntl.h>  // lseek,open,close,etc.
#include <pcre.h>

#define OVECCOUNT 30 // should be multiple of 3

int main(int argc, char **argv) {

  // check cli args
  if(argc<4)
  {
    puts("usage: file_search <file> <pattern> <named substring>");
    return 1;
  }

  int fd,file_length;
  char *file_buffer;

  // open file
  if((fd = open(argv[1], O_RDONLY)) == -1)
  {
    fprintf(stderr, "error: could not open file");
    return 1;
  }

  // seek to EOF, store length
  file_length = lseek(fd, -1, SEEK_END);
  // seek back to start of file
  lseek(fd, 0, SEEK_SET);

  // allocate and zero file buffer
  file_buffer = calloc(file_length + 1, sizeof(char));

  // read file into buffer
  read(fd, file_buffer, file_length);
  close(fd);

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

  if(rc==0) // ovector wasn't big enough, handle error case
  {
    rc = OVECCOUNT/3;
    fprintf(stderr,"error: ovector only has room for %d substrings\n", rc-1);
  }

  #ifdef DEBUG
  printf("Match succeeded at offset %d\n", ovector[0]);

  // Show substrings stored in output vector
  int i;
  for(i=0;i<rc;i++)
  {
    char *substring_start = file_buffer + ovector[2*i];
    int substring_length = ovector[2*i+1] - ovector[2*i];
    printf("%2d: %.*s\n", i, substring_length, substring_start);
  }
  #endif

  //XXX get_named_substring

  const char *named_substring = argv[3];
  const char *matched_substring = NULL;

  int rs = pcre_get_named_substring(
    re,
    file_buffer,
    ovector,
    rc,
    named_substring,
    &matched_substring);

  if(rs<0) { fprintf(stderr,"error: named substring %s not found\n",named_substring); return 1; }

  printf("substring match for %s: %s\n",named_substring,matched_substring);
  pcre_free_substring(matched_substring);

  // cleanup
  free(file_buffer);
  pcre_free(re);

  return 0;
}
