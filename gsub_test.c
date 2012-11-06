#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

int main(int argc, char **argv)
{
  PCRE_CONTAINER *pcre_info = pcre_container_new();
  if(!pcre_info)
  {
    fprintf(stderr,"error: malloc() pcre_container\n");
    return 1;
  }

  char *subject = strdup("The <b>quick</b> <i>brown</i> fox jumped over the <h1>lazy</h1> dog.\nThen there was a <b>NEW</b> line.");

  printf("subject:\n%s\n\n",subject);

  pcre_info->buffer = subject;
  pcre_info->buffer_length = strlen(subject);
  pcre_info->pattern = "(?P<tag><[^>]*?>)"; 
  //pcre_info->named_substring = "url";

  list_container_t *olist = list_container_new();

  if(pcre_exec_multi(pcre_info,NULL,&olist))
  {
    pcre_container_delete(pcre_info);
    return 1;
  }

  pcre_container_delete(pcre_info);

//  print_containers(olist);

  char *temp;
  int container_id;
  list_container_t *list_container = olist;
  while( list_container != NULL && list_container->val != NULL )
  {
    container_id = *(list_container->id);
    list_t *list = list_container->val;
    while( list != NULL && list->id != NULL )
    {   
      printf("container: %-3d id: %-12s val: %s\n", container_id, list->id, list->val);
      if((temp = str_replace(list->val,"",subject)) != NULL)
      {
        free(subject);
        subject = temp;
      }
      
      list = list->next;
    }   
    list_container = list_container->next;
  }

  printf("\nresult:\n%s\n",subject);
  free(subject);

  list_container_del(olist);

/*
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
*/
  return 0;
}
