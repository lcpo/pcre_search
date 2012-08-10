#include <stdio.h>      // io
#include <stdlib.h>     // calloc,free,etc.
#include <string.h>     // strlen
#include <pcre.h>
#include <curl/curl.h>

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
    puts("usage: curl_search <url> <pattern> <named substring>");
    return 1;
  }

  CURL *curl_handle;
  struct MemoryStruct chunk;
  struct MemoryStruct *chunk_ptr = &chunk;
  chunk.memory = malloc(1);
  chunk.size = 0;

  curl_global_init(CURL_GLOBAL_NOTHING);
  curl_handle = curl_easy_init();

  curl_easy_setopt(curl_handle, CURLOPT_URL, argv[1]);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla");

  char curl_errorbuf[CURL_ERROR_SIZE];
  curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, curl_errorbuf);

  if(curl_easy_perform(curl_handle) != 0)
  {
    fprintf(stderr, "%s\n", curl_errorbuf);
    return 1;
  }

  // fire up PCRE!
  pcre_info->buffer = chunk_ptr->memory;
  pcre_info->buffer_length = chunk_ptr->size;
  pcre_info->pattern = argv[2];
  pcre_info->named_substring = argv[3]; // set named substring


  if(pcre_exec_single(pcre_info,pcre_match_callback) > 0)
  {
    // free(pcre_info->buffer);
    return 1;
  }

  if(pcre_exec_multi(pcre_info,pcre_match_callback) > 0)
  {
    // free(pcre_info->buffer);
    return 1;
  }



  // cleanup
  //free(pcre_info->buffer);
  curl_free(chunk.memory);
  curl_easy_cleanup(curl_handle);
  curl_global_cleanup();


  pcre_free(pcre_info->re);

  return 0;
}
