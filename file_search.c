#include <stdio.h>  // io
#include <stdlib.h> // calloc,free,etc.
#include <string.h> // strlen
#include <fcntl.h>  // lseek,open,close,etc.
#include <pcre.h>

#define OVECCOUNT 30 // should be multiple of 3

struct pcre_container {
  char *buffer;                 // subject buffer
  int buffer_length;            // subject buffer length
  char *pattern;                // pattern
  pcre *re;                     // the regex
  const char *error;            // error
  int erroroffset;              // error offset
  int namecount;                // named substring match count
  const char *named_substring;  // the named substring
  int rc;                       // result count ?
  int ovector[OVECCOUNT];       // output vector
} p;

struct pcre_container *pcre_info = &p;

int pcre_exec_single(struct pcre_container *pcre_info, void (*pcre_match_callback)())
{
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
    pcre_info->re,              // the compiled pattern
    NULL,                       // no extra data
    pcre_info->buffer,          // the buffer
    pcre_info->buffer_length,   // file length
    0,                          // start at offset 0 in file_buffer
    0,                          // default options
    pcre_info->ovector,         // output vector for substring info
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
    pcre_free(pcre_info->re);
    return 1;
  }

  (void)pcre_fullinfo(
  pcre_info->re,
  NULL,
  PCRE_INFO_NAMECOUNT,
  &(pcre_info->namecount));

  pcre_match_callback(pcre_info);

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
    if(pcre_info->namecount > 0)
    {
      int name_entry_size;

      (void)pcre_fullinfo(
      pcre_info->re,            // the compiled pattern
      NULL,                     // no extra data - we didn't study the pattern
      PCRE_INFO_NAMEENTRYSIZE,  // size of each entry in the table
      &name_entry_size);        // where to put the answer
      unsigned char *name_table;

      (void)pcre_fullinfo(
      pcre_info->re,            // the compiled pattern
      NULL,                     // no extra data - we didn't study the pattern
      PCRE_INFO_NAMETABLE,      // address of the table
      &name_table);             // where to put the answer

      unsigned char *tabptr = name_table;
      printf("Named substrings\n");
      for (i = 0; i < pcre_info->namecount; i++)
      {
        int n = (tabptr[0] << 8) | tabptr[1];
        printf("(%d) %*s: %.*s\n", n, name_entry_size - 3, tabptr + 2,
          pcre_info->ovector[2*n+1] - pcre_info->ovector[2*n], pcre_info->buffer + pcre_info->ovector[2*n]);
        tabptr += name_entry_size;
      }
    }
  #endif

  return 0;
}


int fetch_named_substring(const char *named_substring, struct pcre_container *pcre_info, const char **matched_substring)
{
  int rs = pcre_get_named_substring(
    pcre_info->re,      // regex
    pcre_info->buffer,  // buffer
    pcre_info->ovector, // output vector
    pcre_info->rc,
    named_substring,    // the named substring i.e. (?P<str>)
    matched_substring); // the named substring's match
  return rs;
}

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





  //XXX search for additonal matches

  // do we need this? -->
  int utf8;
  unsigned int option_bits;
  (void)pcre_fullinfo(pcre_info->re,NULL,PCRE_INFO_OPTIONS,&option_bits);
  utf8 = option_bits & PCRE_UTF8;
  option_bits &= PCRE_NEWLINE_CR|PCRE_NEWLINE_LF|PCRE_NEWLINE_CRLF|
                 PCRE_NEWLINE_ANY|PCRE_NEWLINE_ANYCRLF;

  if(option_bits==0)
  {
    int d;
    (void)pcre_config(PCRE_CONFIG_NEWLINE,&d);
    option_bits = (d == 13)? PCRE_NEWLINE_CR :
                  (d == 10)? PCRE_NEWLINE_LF :
                  (d == (13<<8 | 10))? PCRE_NEWLINE_CRLF :
                  (d == -2)? PCRE_NEWLINE_ANYCRLF :
                  (d == -1)? PCRE_NEWLINE_ANY : 0;
  }

  int crlf_is_newline;
  crlf_is_newline =
       option_bits == PCRE_NEWLINE_ANY ||
       option_bits == PCRE_NEWLINE_CRLF ||
       option_bits == PCRE_NEWLINE_ANYCRLF;
  // <-- do we need this?

  // loop for second and subsequent matches

  for(;;)
  {
    int options = 0;
    int start_offset = pcre_info->ovector[1];

    if(pcre_info->ovector[0] == pcre_info->ovector[1])
    {
      if(pcre_info->ovector[0] == pcre_info->buffer_length) break;
      options = PCRE_NOTEMPTY_ATSTART | PCRE_ANCHORED;
    }

    // run the next matching operation
    pcre_info->rc = pcre_exec(
      pcre_info->re,
      NULL,
      pcre_info->buffer,
      pcre_info->buffer_length,
      start_offset,
      options,
      pcre_info->ovector,
      OVECCOUNT);

    if(pcre_info->rc == PCRE_ERROR_NOMATCH)
    {
      if(options == 0) break;
      pcre_info->ovector[1] = start_offset + 1; //advance one byte
      if(crlf_is_newline &&
         start_offset < pcre_info->buffer_length - 1 &&
         pcre_info->buffer[start_offset] == '\r' &&
         pcre_info->buffer[start_offset + 1] == '\n')
      {
        pcre_info->ovector[1] += 1;
      } else if(utf8) {
        while(pcre_info->ovector[1] < pcre_info->buffer_length)
        {
         if((pcre_info->buffer[pcre_info->ovector[1]] & 0xc0) != 0x80) break;
         pcre_info->ovector[1] += 1;
        }
      }
      continue;
    }

    if(pcre_info->rc < 0)
    {
      printf("Matching error %d\n", pcre_info->rc);
      pcre_free(pcre_info->re);
      return 1;
    }

    if(pcre_info->rc==0)
    {
      pcre_info->rc = OVECCOUNT/3;
      printf("ovector only has room for %d captured substrings\n", pcre_info->rc -1);
    }

    /* As before, show substrings stored in the output vector by number, and then
       also any named substrings. */

    #ifdef DEBUG
      printf("\nMatch succeeded again at offset %d\n", pcre_info->ovector[0]);

      int i;
      for (i = 0; i < pcre_info->rc; i++)
      {
        char *substring_start = pcre_info->buffer + pcre_info->ovector[2*i];
        int substring_length = pcre_info->ovector[2*i+1] - pcre_info->ovector[2*i];
        printf("%2d: %.*s\n", i, substring_length, substring_start);
      }
    #endif


    (void)pcre_fullinfo(
    pcre_info->re,
    NULL,
    PCRE_INFO_NAMECOUNT,
    &(pcre_info->namecount));


    pcre_match_callback(pcre_info);

    #ifdef DEBUG
      if(pcre_info->namecount > 0)
      {
        int name_entry_size;

        (void)pcre_fullinfo(
        pcre_info->re,            // the compiled pattern
        NULL,                     // no extra data - we didn't study the pattern
        PCRE_INFO_NAMEENTRYSIZE,  // size of each entry in the table
        &name_entry_size);        // where to put the answer
        unsigned char *name_table;

        (void)pcre_fullinfo(
        pcre_info->re,            // the compiled pattern
        NULL,                     // no extra data - we didn't study the pattern
        PCRE_INFO_NAMETABLE,      // address of the table
        &name_table);             // where to put the answer

        unsigned char *tabptr = name_table;
        printf("Named substrings\n");
        for (i = 0; i < pcre_info->namecount; i++)
        {
          int n = (tabptr[0] << 8) | tabptr[1];
          printf("(%d) %*s: %.*s\n", n, name_entry_size - 3, tabptr + 2,
            pcre_info->ovector[2*n+1] - pcre_info->ovector[2*n], pcre_info->buffer + pcre_info->ovector[2*n]);
          tabptr += name_entry_size;
        }
      }
    #endif
  }      /* End of loop to find second and subsequent matches */


  // cleanup
  free(pcre_info->buffer);
  pcre_free(pcre_info->re);

  return 0;
}
