#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define WEB_API "http://ajax.googleapis.com/ajax/services/search/web?v=1.0&q="

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

  list_container_t *list;

  list = curl_pcre_search(url, "unescapedUrl\":\"(?P<url>[^\"]+)\".*?titleNoFormatting\":\"(?P<title>[^\"]+)\",\"content\":\"(?P<content>[^\"]+)\"");

  printf("\n");
  print_containers(list);

  list_container_del(list);
  free(url);

  return 0;
}
