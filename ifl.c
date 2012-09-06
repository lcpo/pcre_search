#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define WEB_API "http://ajax.googleapis.com/ajax/services/search/web?v=1.0&q="

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
        printf("%s\n",ret);
        free(ret);
      } else {
        printf("%s\n",matched_substring);
      }   
      pcre_free_substring(matched_substring);
    }   
  }
}

int main(int argc, char **argv)
{
  // check cli args
  if(argc<2)
  {
    puts("Usage: ifl [query]");
    return 1;
  }

  char *url = malloc(snprintf(NULL, 0, "%s%s", WEB_API, argv[1])+1);
  sprintf(url, "%s%s", WEB_API, argv[1]);
  int i;
  for(i=2; i<argc; i++)
  {
    url = realloc(url, snprintf(NULL, 0, "%s+%s", url, argv[i])+1);
    sprintf(url, "%s+%s", url, argv[i]);
  }

  //printf("url: %s\n",url);

  CURL_BUFFER *curl_buffer = request(url);
  if(!curl_buffer)
    return 1;

  PCRE_CONTAINER *pcre_info = pcre_container_new();
  if(!pcre_info)
  {
    fprintf(stderr,"Error: malloc() pcre_container\n");
    curl_buffer_delete(curl_buffer);
    return 1;
  }

  pcre_info->buffer = curl_buffer->memory;
  pcre_info->buffer_length = curl_buffer->size;
  pcre_info->pattern = "titleNoFormatting\":\"(?P<title>[^\"]+)\"";
  pcre_info->named_substring = "title";

  if(pcre_exec_single(pcre_info,pcre_match_callback))
  {
    curl_buffer_delete(curl_buffer);
    pcre_container_delete(pcre_info);
    return 1;
  }

  pcre_info->pattern = "unescapedUrl\":\"(?P<url>[^\"]+)\"";
  pcre_info->named_substring = "url";

  if(pcre_exec_single(pcre_info,pcre_match_callback))
  {
    curl_buffer_delete(curl_buffer);
    pcre_container_delete(pcre_info);
    return 1;
  }

  // cleanup and return normal
  curl_buffer_delete(curl_buffer);
  pcre_container_delete(pcre_info);
  return 0;
}
