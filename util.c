#include <stdio.h>      // io
#include <stdlib.h>     // calloc,free,exit,etc.
#include <stdarg.h>     // va_list
#include <string.h>     // strlen
#include <iconv.h>      // utf conversion
#include <sys/types.h>  // size_t
#include <ctype.h>      // iscntrl,isxdigit
#include <pcre.h>       // libpcre
#include <curl/curl.h>  // libcurl

#include "util.h"       // WriteMemoryCallback,pcrecontainer structs

#define OVECCOUNT 30    // for libpcre, should be multiple of 3

int is_utf(char *string) {
    if (!isxdigit(string[0])) { return 0; }
    if (!isxdigit(string[1])) { return 0; }
    if (!isxdigit(string[2])) { return 0; }
    if (!isxdigit(string[3])) { return 0; }
    return 1;
}

char *parse_escaped_characters(char *string) {
  char *output_string = (char*)malloc(strlen(string) + 1);
  char *output_string_ptr = output_string;
  char *string_ptr = string;
  char current_char;
  unsigned int utf_val;
  void *reallocated_ptr;
  if (!output_string) { return NULL; }
  while (*string_ptr) {
      current_char = *string_ptr;
      if (current_char == '\\') {
          string_ptr++;
          current_char = *string_ptr;
          switch (current_char) {
              case '\"': case '\\': case '/': break;
              case 'b': current_char = '\b'; break;
              case 'f': current_char = '\f'; break;
              case 'n': current_char = '\n'; break;
              case 'r': current_char = '\r'; break;
              case 't': current_char = '\t'; break;
              case 'u':
                  string_ptr++;
                  if (!is_utf(string_ptr) ||
                          sscanf(string_ptr, "%4x", &utf_val) == EOF) {
                      free(output_string); return NULL;
                  }
                  if (utf_val < 0x80) {
                      current_char = utf_val;
                  } else if (utf_val < 0x800) {
                      *output_string_ptr++ = (utf_val >> 6) | 0xC0;
                      current_char = ((utf_val | 0x80) & 0xBF);
                  } else {
                      *output_string_ptr++ = (utf_val >> 12) | 0xE0;
                      *output_string_ptr++ = (((utf_val >> 6) | 0x80) & 0xBF);
                      current_char = ((utf_val | 0x80) & 0xBF);
                  }
                  string_ptr += 3;
                  break;
              default:
                free(output_string);
                return NULL;
                break;
          }
      } else if (iscntrl(current_char)) { /* no control characters allowed */
          free(output_string);
          return NULL;
      }
      *output_string_ptr = current_char;
      output_string_ptr++;
      string_ptr++;
  }
  *output_string_ptr = '\0';
  reallocated_ptr = realloc(output_string, strlen(output_string) + 1);
  if (!reallocated_ptr) { free(output_string); return NULL; }
  output_string = (char*)reallocated_ptr;
  return output_string;
}

list_t *list_new()
{
  list_t *list;

  if((list = malloc(sizeof(list_t))) == NULL)
    return NULL;

  list->id = NULL;
  list->val = NULL;
  list->next = NULL;

  return list;
}

list_container_t *list_container_new()
{
  list_container_t *list_container;

  if((list_container = malloc(sizeof(list_container_t))) == NULL)
    return NULL;

  list_container->id = NULL;
  list_container->val = NULL;
  list_container->next = NULL;

  return list_container;
}

int add_list(list_container_t **list_container)
{
  list_container_t *current_list_container = *list_container;
  if(current_list_container->id == NULL && current_list_container->next == NULL)
  {
    int *id = malloc(sizeof(int));
    *id = 1;
    current_list_container->id = id;
    current_list_container->val = list_new();
    return *id;
  }

  list_container_t *new_list_container = list_container_new();
  int *id = malloc(sizeof(int));
  *id = *(current_list_container->id)+1;
  new_list_container->id = id;
  new_list_container->val = list_new();
  new_list_container->next = current_list_container;
  *list_container = new_list_container;
  return *id;
}

list_container_t *find_container(list_container_t *list_container, int container_id)
{
  while( list_container != NULL && list_container->id != NULL)
  {
    if(container_id == *(list_container->id))  return list_container;
    list_container = list_container->next;
  }
  return NULL;
}

int add_node(list_container_t *list_container, int container_id, char *id, char *val)
{
  list_container_t *specified_container = find_container(list_container, container_id);
  if(specified_container == NULL || specified_container->val == NULL) return 1;

  list_t *current_list = specified_container->val;
  if(current_list->id == NULL && current_list->next == NULL)
  {
    current_list->id = strdup(id);
    current_list->val = strdup(val);
    return 0;
  }

  list_t *new_list = list_new();
  new_list->id = strdup(id);
  new_list->val = strdup(val);
  new_list->next = current_list;
  specified_container->val = new_list;
  return 0;
}

void list_del(list_t *list)
{
  list_t *temp;
  while( list != NULL )
  {
    temp = list;
    list = list->next;
    free(temp->id);
    free(temp->val);
    free(temp);
  }
}

void list_container_del(list_container_t *list_container)
{
  list_container_t *temp;
  while( list_container != NULL )
  {
    temp = list_container;
    list_container = list_container->next;
    free(temp->id);
    list_del(temp->val);
    free(temp);
  }
}

void print_container(list_container_t *list_container, int container_id)
{
  list_container_t *specified_container = find_container(list_container, container_id);
  if(specified_container == NULL || specified_container->val == NULL) return;
  list_t *list = specified_container->val;
  while( list != NULL && list->id != NULL )
  {
    printf("container: %d id: %s val: %s\n", container_id, list->id, list->val);
    list=list->next;
  }
}

void print_containers(list_container_t *list_container)
{
  int container_id;
  while( list_container != NULL && list_container->val != NULL )
  {
    container_id = *(list_container->id);
    list_t *list = list_container->val;
    while( list != NULL && list->id != NULL )
    {
      printf("container: %-3d id: %-12s val: %s\n", container_id, list->id, list->val);
      list = list->next;
    }
    list_container = list_container->next;
  }
}


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
      printf("substring match for %s: %s\n",pcre_info->named_substring,matched_substring);
      pcre_free_substring(matched_substring);
    }
  }
}
*/

// curl_pcre_search(url, re, named_subpattern, named_subpattern)
list_container_t *curl_pcre_search(char *url, char *re)
{
  // fire up CURL!
  CURL_BUFFER *curl_buffer = request(url);
  if(!curl_buffer) return NULL;

  PCRE_CONTAINER *pcre_info = pcre_container_new();
  if(!pcre_info)
  {
    fprintf(stderr,"error: malloc() pcre_container\n");
    curl_buffer_delete(curl_buffer);
    return NULL;
  }

  pcre_info->buffer = curl_buffer->memory;
  pcre_info->buffer_length = curl_buffer->size;
  pcre_info->pattern = re;
  //pcre_info->named_substring = "url";

  list_container_t *olist = list_container_new();

  if(pcre_exec_multi(pcre_info,NULL,&olist))
  {
    curl_buffer_delete(curl_buffer);
    pcre_container_delete(pcre_info);
    return NULL;
  }

  /*
  if(pcre_info->namecount <= 0)
  {
    fprintf(stderr, "error: curl_pcre_search() no named substrings in regex\n");
    curl_buffer_delete(curl_buffer);
    pcre_container_delete(pcre_info);
    return NULL;
  }
  */

  /*

  const char *matched_substring = NULL;
  char *ns = NULL;
  va_list valist;
  va_start(valist, re);
  int i;
  for(i=0;i<pcre_info->namecount;i++)
  {
    ns = va_arg(valist, char*);
    if((fetch_named_substring((const char*)ns, pcre_info, &matched_substring)) >= 0)
    {
      //printf("substring match for %s: %s\n", ns, matched_substring);
      add_node(&list, ns, (char *)matched_substring);
      pcre_free_substring(matched_substring);
    }
  }
  va_end(valist);
  */

  curl_buffer_delete(curl_buffer);
  pcre_container_delete(pcre_info);

  return olist;
}


CURL_BUFFER *request(char *url)
{
  curl_global_init(CURL_GLOBAL_SSL);
  CURL *curl_handle = curl_easy_init();
  char curl_errorbuf[CURL_ERROR_SIZE];

  CURL_BUFFER *curl_buffer;
  if((curl_buffer = curl_buffer_new()) == NULL)
  {
    fprintf(stderr,"Error: malloc() curl_buffer\n");
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
    return NULL;
  }

  curl_easy_setopt(curl_handle, CURLOPT_URL, url);
  //curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 2L);
  curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L); // this will follow 301 redirects
  curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 1L);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)curl_buffer);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Chrome");
  curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, curl_errorbuf);

  // fire up CURL!
  if(curl_easy_perform(curl_handle) != 0)
  {
    fprintf(stderr, "error: curl_error: %s\n", curl_errorbuf);
    curl_buffer_delete(curl_buffer);
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
    return NULL;
  }

  curl_easy_cleanup(curl_handle);
  curl_global_cleanup();

  return curl_buffer;
}


char *utf8_to_ascii(char *in)
{
  size_t in_size=strlen(in)+1;
  char *out = (char*)calloc(in_size,1);

  char *in_ptr  = in;
  char *out_ptr = out;

  iconv_t cd;

  if ((iconv_t)(-1) == (cd = iconv_open("ASCII//TRANSLIT", "UTF-8"))) {
    //fprintf(stderr,"Failed to iconv_open.\n");
    free(out);
    return NULL;
  }
  if ((size_t)(-1) == iconv(cd, &in_ptr, &in_size, &out_ptr, &in_size)) {
    //fprintf(stderr,"Failed to convert characters to new code set.\n");
    free(out);
    return NULL;
  }

  if(-1 == iconv_close(cd))
    fprintf(stderr,"Failed to iconv_close.\n");

  return out;
}

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
    char*t5=calloc(buffer_size,1);
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

// CURL_BUFFER constructor
CURL_BUFFER *curl_buffer_new()
{
  CURL_BUFFER *b; 
  if((b = (CURL_BUFFER*)malloc(sizeof(CURL_BUFFER))) == NULL)
    return NULL;
  if((b->memory = (char*)malloc(1)) == NULL)
    return NULL;
  b->size = 0;
  return b;
}

// CURL_BUFFER destructor
void curl_buffer_delete(CURL_BUFFER *curl_buffer)
{
  curl_free(curl_buffer->memory);
  free(curl_buffer);
}

// PCRE_CONTAINER constructor
PCRE_CONTAINER *pcre_container_new()
{
  PCRE_CONTAINER *p;
  if((p = (PCRE_CONTAINER*)malloc(sizeof(PCRE_CONTAINER))) == NULL)
    return NULL;
  p->re = NULL;
  return p;
}

// PCRE_CONTAINER destructor
void pcre_container_delete(PCRE_CONTAINER *pcre_info)
{
  pcre_free(pcre_info->re);
  free(pcre_info);
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

/*
int fetch_named_substring(const char *named_substring, PCRE_CONTAINER *pcre_info, const char **matched_substring)
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
*/

int pcre_exec_single(PCRE_CONTAINER *pcre_info, void (*callback)(), list_container_t **olist)
{
  pcre_info->re = pcre_compile(
    pcre_info->pattern,            // the pattern :)
    PCRE_DOTALL|PCRE_NEWLINE_ANY,  // options
    &(pcre_info->error),           // for errror message
    &(pcre_info->erroroffset),     // for error offset
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
      default: fprintf(stderr,"error: matching error.\n"); break;
    }
    return 1;
  }

  if(pcre_info->rc==0) // ovector wasn't big enough, handle error case
  {
    pcre_info->rc = OVECCOUNT/3;
    fprintf(stderr,"error: ovector only has room for %d substrings\n", (pcre_info->rc)-1);
    return 1;
  }

  (void)pcre_fullinfo(
  pcre_info->re,
  NULL,
  PCRE_INFO_NAMECOUNT,
  &(pcre_info->namecount));

  // We have a match, run callback if one is defined
  if(callback) callback(pcre_info);
  add_list(olist);
  //printf("Container: %d allocated.\n",container_no);

  //#ifdef DEBUG
    //printf("Match succeeded at offset %d\n", pcre_info->ovector[0]);

    // Show substrings stored in output vector
    int i;
    /*
    for(i=0;i<(pcre_info->rc);i++)
    {
      char *substring_start = pcre_info->buffer + pcre_info->ovector[2*i];
      int substring_length = pcre_info->ovector[2*i+1] - pcre_info->ovector[2*i];
      printf("%2d: %.*s\n", i, substring_length, substring_start);
    }
    */
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
      //printf("Named substrings\n");
      for (i = 0; i < pcre_info->namecount; i++)
      {
        int n = (tabptr[0] << 8) | tabptr[1];

        char *ns = (char*)tabptr+2; // these are null terminated by libpcre (see man pcreapi)

        int ms_len = pcre_info->ovector[2*n+1] - pcre_info->ovector[2*n];
        char *ms = (char*)calloc(ms_len+1,1);
        strncpy(ms, (const char*)(pcre_info->buffer + pcre_info->ovector[2*n]), ms_len);

        // cleanup escaped characters from matched subpattern if found
        char *temp = ms;
        if(strstr(temp, "\\") != NULL)
        {
          ms = parse_escaped_characters(temp);
          free(temp);
        }

        //printf("ns: %s ms: %s\n", ns, ms);
        add_node(*olist, 1, ns, ms);
        free(ms);

        //printf("(%d) %s: %.*s\n", n, tabptr + 2,
        //  pcre_info->ovector[2*n+1] - pcre_info->ovector[2*n], pcre_info->buffer + pcre_info->ovector[2*n]);
        tabptr += name_entry_size;
      }
    }
  //#endif

  return 0;
}

int pcre_exec_multi(PCRE_CONTAINER *pcre_info, void (*callback)(), list_container_t **olist)
{
  if(pcre_exec_single(pcre_info,callback,olist) > 0)
    return 1;

  int match_number;
  //XXX do we need this? -->
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
      if(options == 0)
      {
        // if options is 0 we have found all possible matches
        break;
      }
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
      return 1;
    }

    if(pcre_info->rc==0)
    {
      pcre_info->rc = OVECCOUNT/3;
      printf("ovector only has room for %d captured substrings\n", pcre_info->rc -1);
    }

    //#ifdef DEBUG
      // As before, show substrings stored in the output vector
      // by number, and then also any named substrings.

      //printf("\nMatch succeeded again at offset %d\n", pcre_info->ovector[0]);

      int i;
      /*
      for (i = 0; i < pcre_info->rc; i++)
      {
        char *substring_start = pcre_info->buffer + pcre_info->ovector[2*i];
        int substring_length = pcre_info->ovector[2*i+1] - pcre_info->ovector[2*i];
        printf("%2d: %.*s\n", i, substring_length, substring_start);
      }
      */
    //#endif

    (void)pcre_fullinfo(
    pcre_info->re,
    NULL,
    PCRE_INFO_NAMECOUNT,
    &(pcre_info->namecount));

    // We have a match, run callback if one is defined
    if(callback) callback(pcre_info);
    match_number = add_list(olist);
    //printf("Container: %d allocated.\n",match_number);

    //#ifdef DEBUG
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
        //printf("Named substrings\n");
        for (i = 0; i < pcre_info->namecount; i++)
        {
          int n = (tabptr[0] << 8) | tabptr[1];

          char *ns = (char*)tabptr+2; // these are null terminated by libpcre (see man pcreapi)

          int ms_len = pcre_info->ovector[2*n+1] - pcre_info->ovector[2*n];
          char *ms = (char*)calloc(ms_len+1,1);
          strncpy(ms, (const char*)(pcre_info->buffer + pcre_info->ovector[2*n]), ms_len);

          // cleanup escaped characters from matched subpattern
          char *temp = ms;
          if(strstr(temp, "\\") != NULL)
          {
            ms = parse_escaped_characters(temp);
            free(temp);
          }

          //printf("ns: %s ms: %s\n", ns, ms);
          add_node(*olist, match_number, ns, ms);
          free(ms);

          //printf("(%d) %*s: %.*s\n", n, name_entry_size - 3, tabptr + 2,
          //  pcre_info->ovector[2*n+1] - pcre_info->ovector[2*n], pcre_info->buffer + pcre_info->ovector[2*n]);
          tabptr += name_entry_size;
        }
      }
    //#endif
  }      /* End of loop to find second and subsequent matches */

  return 0;
}

/*
int main(int argc, char **argv)
{
  list_container_t *list_container = list_container_new();
  printf("Container: %d allocated.\n",add_list(&list_container));
  printf("Container: %d allocated.\n",add_list(&list_container));

  add_node(list_container, 1, "1", "one in container 1");
  add_node(list_container, 1, "2", "two in container 1");
  add_node(list_container, 1, "3", "three in container 1");

  add_node(list_container, 2, "1", "one in container 2");
  add_node(list_container, 2, "2", "two in container 2");
  add_node(list_container, 2, "3", "three in container 2");

  print_container(list_container, 1);
  printf("\n");
  print_container(list_container, 2);


  list_container_del(list_container);
  return 0;
}
*/
