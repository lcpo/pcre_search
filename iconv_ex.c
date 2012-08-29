#include <stdio.h>
#include <stdlib.h>
#include <iconv.h>
#include <string.h>

int main()
{
  char *in=strdup("The bee\xE2\x80\x99s knees, is a stupid expression.");
  size_t in_size=strlen(in);

  char *out=(char*)calloc(in_size,1);

  char *in_ptr	= in;
  char *out_ptr = out;

  iconv_t cd;

  if ((iconv_t)(-1) == (cd = iconv_open("ASCII//TRANSLIT", "UTF-8"))) {
    printf("Failed to iconv_open.\n");
    return 1;
  }
  if ((size_t)(-1) == iconv(cd, &in_ptr, &in_size, &out_ptr, &in_size)) {
    printf("Fail to convert characters to new code set.\n");
    return 1;
  }

  printf("in: %s\n",in);
  printf("out: %s\n",out);

  if (-1 == iconv_close(cd)) {
    printf("Fail to iconv_close.\n");
    return 1;
  }

  free(in);
  free(out);

  return 0;
}
