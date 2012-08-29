#include <stdio.h>
#include <stdlib.h>
#include <iconv.h>
#include <string.h>

char *utf8_to_ascii(char *in)
{
  size_t in_size=strlen(in)+1;
  char *out = (char*)calloc(in_size,1);

  char *in_ptr  = in;
  char *out_ptr = out;

  iconv_t cd;

  if ((iconv_t)(-1) == (cd = iconv_open("ASCII//TRANSLIT", "UTF-8"))) {
    fprintf(stderr,"Failed to iconv_open.\n");
    free(out);
    return in;
  }
  if ((size_t)(-1) == iconv(cd, &in_ptr, &in_size, &out_ptr, &in_size)) {
    fprintf(stderr,"Failed to convert characters to new code set.\n");
    free(out);
    return in;
  }

  return out;
}


int main()
{
  char *in=strdup("The bee\xE2\x80\x99s knees, is a stupid expression.");

  char *out = utf8_to_ascii(in);

  printf("in: %s\nout: %s\n",in,out);

  free(in);
  free(out);

  return 0;
}
