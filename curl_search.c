#include <stdio.h>      // io
#include <stdlib.h>     // calloc,free,etc.
#include <string.h>     // strlen
#include <pcre.h>       // libpcre
#include <curl/curl.h>  // libcurl

#include "util.h"

// custom callback function to exec on each match
void pcre_match_callback(PCRE_CONTAINER *pcre_info)
{
  // get_named_substring if it exists
  if(pcre_info->namecount > 0)
  {
    const char *matched_substring = NULL;

    if((fetch_named_substring(pcre_info->named_substring, pcre_info, &matched_substring)) >= 0)
    {
      char *ret = NULL;
      if((ret=str_replace("\\u0026#39;","\'",(char*)matched_substring)))
      {
        printf("substring match for %s: %s\n",pcre_info->named_substring,ret);
        free(ret);
      } else {
        printf("substring match for %s: %s\n",pcre_info->named_substring,matched_substring);
      }
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

  curl_global_init(CURL_GLOBAL_NOTHING);
  CURL *curl_handle = curl_easy_init();
  char curl_errorbuf[CURL_ERROR_SIZE];

  CURL_BUFFER *curl_buffer;
  if((curl_buffer = curl_buffer_new()) == NULL)
  {
    fprintf(stderr,"Error: malloc() curl_buffer\n");
    return 1;
  }

  curl_easy_setopt(curl_handle, CURLOPT_URL, argv[1]);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)curl_buffer);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Chrome");
  curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, curl_errorbuf);

  // fire up CURL!
  if(curl_easy_perform(curl_handle) != 0)
  {
    fprintf(stderr, "%s\n", curl_errorbuf);
    curl_buffer_delete(curl_buffer);
    return 1;
  }

  // fire up PCRE!
  PCRE_CONTAINER *pcre_info;
  if((pcre_info = pcre_container_new()) == NULL)
  {
    fprintf(stderr,"Error: malloc() pcre_container\n");
    curl_buffer_delete(curl_buffer);
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
    return 1;
  }

  pcre_info->buffer = curl_buffer->memory;
  pcre_info->buffer_length = curl_buffer->size;
  pcre_info->pattern = argv[2];
  pcre_info->named_substring = argv[3]; // set named substring

  if(pcre_exec_multi(pcre_info,pcre_match_callback) > 0)
  {
    // cleanup and return error
    curl_buffer_delete(curl_buffer);
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
    pcre_container_delete(pcre_info);
    return 1;
  }

  // cleanup and return normal
  curl_buffer_delete(curl_buffer);
  curl_easy_cleanup(curl_handle);
  curl_global_cleanup();
  pcre_container_delete(pcre_info);
  return 0;
}
