#include <iconv.h>
#include <stdlib.h>
#include <stdio.h>

int main(void)
{
   const char   fromcode[] = "UTF-8";
   const char   tocode[] = "ASCII//TRANSLIT";
   iconv_t      cd;
   char         in[] = "blah blah blorg\xe2\x80\x99 bajeebus, yay!";
   size_t       in_size = sizeof(in);
   char         *inptr = in;
   char         out[100];
   size_t       out_size = sizeof(out);
   char         *outptr = out;
   int          i;

   if ((iconv_t)(-1) == (cd = iconv_open(tocode, fromcode))) {
      printf("Failed to iconv_open %s to %s.\n", fromcode, tocode);
      exit(EXIT_FAILURE);
   }
   if ((size_t)(-1) == iconv(cd, &inptr, &in_size, &outptr, &out_size)) {
      printf("Fail to convert characters to new code set.\n");
      exit(EXIT_FAILURE);
   }
   *outptr = '\0';
   /*
   printf("The hex representation of string %s are:\n  In codepage %s ==> ",
          in, fromcode);
   for (i = 0; in[i] != '\0'; i++) {
      printf("0x%02x ", in[i]);
   }
   printf("\n  In codepage %s ==> ", tocode);
   for (i = 0; out[i] != '\0'; i++) {
      printf("0x%02x ", out[i]);
   }
   */
   printf("out: %s\n",out);
   if (-1 == iconv_close(cd)) {
      printf("Fail to iconv_close.\n");
      exit(EXIT_FAILURE);
   }
   return 0;

}

