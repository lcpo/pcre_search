#include <stdio.h>      // io
#include <stdlib.h>     // calloc,free,exit,etc.
#include <string.h>     // strlen
#include <fcntl.h>      // lseek,open,close,etc.
#include <sys/types.h>  // size_t
#include <pcre.h>       // libpcre
#include <curl/curl.h>  // libcurl

#include "util.h"       // WriteMemoryCallback,pcrecontainer structs

#define OVECCOUNT 30    // for libpcre, should be multiple of 3

// replaces all occurences with another occurence
char *str_replace(char * t1, char * t2, char * t6){
  // count number of substrings
  int substr_count = 0;
  char *tmp = t6;
  while((*tmp != '\0') && (tmp=strstr(tmp,t1)))
  {
    substr_count++;
    tmp++;
  }
  if(substr_count > 0)
  {
    int buffer_size = strlen(t6)-(strlen(t1)*substr_count)+(strlen(t2)*substr_count)+1;
    char*t4;
    char*t5=calloc(buffer_size,sizeof(char*));
    while(strstr(t6,t1)){
      t4=strstr(t6,t1);
      strncpy(t5+strlen(t5),t6,t4-t6);
      strcat(t5,t2);
      t4+=strlen(t1);
      t6=t4;
    }
    return strcat(t5,t4);
  }
  return NULL; // no substrings found
}

// curl buffer constructor
CURL_BUFFER *curl_buffer_new()
{
  CURL_BUFFER *b; 
  if((b = malloc(sizeof *b)) == NULL)
    return NULL;
  if((b->memory = (char*)malloc(1)) == NULL)
    return NULL;
  b->size = 0;
  return b;
}

void curl_buffer_delete(CURL_BUFFER *curl_buffer)
{
  curl_free(curl_buffer->memory);
  free(curl_buffer);
}

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  CURL_BUFFER *mem = (CURL_BUFFER *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory == NULL)
  {
    fprintf(stderr, "error: not enough memory (realloc returned NULL)\n");
    exit(1);
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
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

int pcre_exec_single(struct pcre_container *pcre_info, void (*callback)())
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

  callback(pcre_info);

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

int pcre_exec_multi(struct pcre_container *pcre_info, void (*callback)())
{
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


    callback(pcre_info);

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

  return 0;
}

