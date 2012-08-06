#include <stdio.h>      // io
#include <stdlib.h>     // calloc,free,etc.
#include <string.h>     // strlen
#include <fcntl.h>      // lseek,open,close,etc.
#include <pcre.h>

#include "util.h"

struct pcre_container p;
struct pcre_container *pcre_info = &p;

// custom callback function to exec on each match
void pcre_match_callback(struct pcre_container *pcre_info)
{
  // get_named_substring if it exists
  if(pcre_info->namecount > 0)
  {
    const char *matched_substring = NULL;

    if((fetch_named_substring(pcre_info->named_substring, pcre_info, &matched_substring)) >= 0)
    {
      printf("substring match for %s: %s\n",pcre_info->named_substring,matched_substring);
      pcre_free_substring(matched_substring);
    }
  }
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

  // fire up PCRE!
  pcre_info->buffer = file_buffer;
  pcre_info->buffer_length = file_length;
  pcre_info->pattern = argv[2];
  pcre_info->named_substring = argv[3]; // set named substring


  if(pcre_exec_single(pcre_info,pcre_match_callback) > 0)
  {
    free(file_buffer);
    return 1;
  }

  if(pcre_exec_multi(pcre_info,pcre_match_callback) > 0)
  {
    free(file_buffer);
    return 1;
  }



  // cleanup
  free(pcre_info->buffer);
  pcre_free(pcre_info->re);

  return 0;
}
