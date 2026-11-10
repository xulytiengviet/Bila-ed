#include "utf8.h"
#include "cvnss_engine.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
int main(void){
 const char *s="Tiếng Việt";assert(utf8_valid(s,strlen(s)));
 assert(!utf8_valid("\xC0\xAF",2));assert(!utf8_valid("\xE2\x82",2));
 assert(!utf8_valid("\xED\xA0\x80",3));assert(!utf8_valid("\xF4\x90\x80\x80",4));
 assert(!utf8_valid("a\0b",3));
 char *out=utf8_case("tôi",2);assert(out&&!strcmp(out,"TÔI"));free(out);
 char *trits=NULL,*back=NULL;
 assert(!base27_encode("ABC Z",&trits));assert(!base27_decode(trits,&back));
 assert(!strcmp(back,"ABC Z"));free(trits);free(back);
 assert(base27_encode("A1",&trits)<0);
 assert(base27_decode("11220",&back)<0);
 puts("test_utf8: OK");return 0;
}
