#ifndef __UTIL_H__
#define __UTIL_H__

#include <sys/types.h>
#include <pcre.h>

#define OVECCOUNT 30 // should be multiple of 3

struct MemoryStruct
{
  char *memory;
  size_t size;
};

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
};

/*
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
int fetch_named_substring(const char *named_substring, struct pcre_container *pcre_info, const char **matched_substring);
int pcre_exec_single(struct pcre_container *pcre_info, void (*callback)());
int pcre_exec_multi(struct pcre_container *pcre_info, void (*callback)());
*/

#endif
