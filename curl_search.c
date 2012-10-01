#include <stdio.h>      // io
#include <stdlib.h>     // calloc,free,etc.
#include <string.h>     // strlen

#include "util.h"

// custom callback function to exec on each match
/*
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
*/

int main(int argc, char **argv) {

//  curl_pcre_search("http://thefuckingweather.com/?where=21044", "<span class=\"temperature\"[^>]+>(?P<temp>[^<]+)</span>.*?<p class=\"remark\">(?P<remark>[^<]+)</p>.*?<p class=\"flavor\">(?P<flavortext>[^<]+)</p>", "temp", "remark", "flavortext");

list_t *list;

list = curl_pcre_search("http://thefuckingweather.com/?where=21044", "<span class=\"temperature\"[^>]+>(?P<temp>[^<]+)</span>.*?<p class=\"remark\">(?P<remark>[^<]+)</p>.*?<p class=\"flavor\">(?P<flavortext>[^<]+)</p>", "temp", "remark", "flavortext");

ll_puts(list, "temp");
ll_puts(list, "remark");
ll_puts(list, "flavortext");

list_del(list);

/*
  // check cli args
  if(argc<4)
  {
    puts("Usage: curl_search [url] [pattern] [named substring]");
    return 1;
  }

  // fire up CURL!
  CURL_BUFFER *curl_buffer = request(argv[1]);
  if(!curl_buffer)
    return 1;

  // fire up PCRE!
  PCRE_CONTAINER *pcre_info = pcre_container_new();
  if(!pcre_info)
  {
    fprintf(stderr,"Error: malloc() pcre_container\n");
    curl_buffer_delete(curl_buffer);
    return 1;
  }

  pcre_info->buffer = curl_buffer->memory;
  pcre_info->buffer_length = curl_buffer->size;
  pcre_info->pattern = argv[2];
  pcre_info->named_substring = argv[3]; // set named substring

//  if(pcre_exec_multi(pcre_info,pcre_match_callback))
  if(pcre_exec_multi(pcre_info,NULL))
  {
    // cleanup and return error
    curl_buffer_delete(curl_buffer);
    pcre_container_delete(pcre_info);
    return 1;
  }

  // cleanup and return normal
  curl_buffer_delete(curl_buffer);
  pcre_container_delete(pcre_info);
*/

  return 0;
}
