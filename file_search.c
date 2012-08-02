#include <stdio.h>  // io
#include <stdlib.h> // calloc,free,etc.
#include <string.h> // strlen
#include <fcntl.h>  // lseek,open,close,etc.
#include <pcre.h>

#define OVECCOUNT 30 // should be multiple of 3

struct pcre_container {
  char *buffer;
  int buffer_length;
  char *pattern;
  pcre *re;
  const char *error;
  int erroroffset;
  int rc;
  int ovector[OVECCOUNT];
} p;

struct pcre_container *pcre_info = &p;

int fetch_named_substring(const char *named_substring, struct pcre_container *pcre_info, const char **matched_substring)
{
  int rs = pcre_get_named_substring(
    pcre_info->re,
    pcre_info->buffer,
    pcre_info->ovector,
    pcre_info->rc,
    named_substring,
    matched_substring);
  return rs;
}

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

  // set our pcre_info buffer
  pcre_info->buffer = file_buffer;
  pcre_info->buffer_length = file_length;

  ///////////////////////////////////////
  // Do something with the file_buffer //
  ///////////////////////////////////////

  // file_length and strlen(file_buffer) are the same see?
  // printf("strlen(file_buffer) = %d\n", (int)strlen(file_buffer));
  // printf("        file_length = %d\n", (int)file_length);

  pcre_info->pattern = argv[2];

  pcre_info->re = pcre_compile(
    pcre_info->pattern,         // the pattern :)
    0,                          // default options
    &(pcre_info->error),        // for errror message
    &(pcre_info->erroroffset),  // for error offset
    NULL);

  if(pcre_info->re==NULL) // regex compile failed, handle error case
  {
    fprintf(stderr, "error: PCRE compile failed at offset %d: %s\n", pcre_info->erroroffset, pcre_info->error);
    return 1;
  }

  pcre_info->rc = pcre_exec(
    pcre_info->re,            // the compiled pattern
    NULL,                     // no extra data
    pcre_info->buffer,        // the file_buffer
    pcre_info->buffer_length, // file length
    0,                        // start at offset 0 in file_buffer
    0,                        // default options
    pcre_info->ovector,       // output vector for substring info
    OVECCOUNT);

  if(pcre_info->rc<0) // match failed, handle error cases
  {
    switch(pcre_info->rc)
    {
      case PCRE_ERROR_NOMATCH: puts("No match."); break;
      default: fprintf(stderr,"error: matching error."); break;
    }
    pcre_free(pcre_info->re);
    return 1;
  }

  if(pcre_info->rc==0) // ovector wasn't big enough, handle error case
  {
    pcre_info->rc = OVECCOUNT/3;
    fprintf(stderr,"error: ovector only has room for %d substrings\n", (pcre_info->rc)-1);
  }

  #ifdef DEBUG
  printf("Match succeeded at offset %d\n", pcre_info->ovector[0]);

  // Show substrings stored in output vector
  int i;
  for(i=0;i<(pcre_info->rc);i++)
  {
    char *substring_start = pcre_info->buffer + pcre_info->ovector[2*i];
    int substring_length = pcre_info->ovector[2*i+1] - pcre_info->ovector[2*i];
    printf("%2d: %.*s\n", i, substring_length, substring_start);
  }
  #endif

  //XXX get_named_substring

  const char *named_substring=argv[3];
  const char *matched_substring = NULL;

  if((fetch_named_substring(named_substring, pcre_info, &matched_substring)) < 0)
  {
    fprintf(stderr,"error: named substring %s not found\n",named_substring);
    return 1;
  }

  printf("substring match for %s: %s\n",named_substring,matched_substring);

  pcre_free_substring(matched_substring);

  //XXX

  // cleanup
  free(pcre_info->buffer);
  pcre_free(pcre_info->re);

  return 0;
}
