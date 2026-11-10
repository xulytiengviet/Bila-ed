#include "cved.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
static void test(const char *bytes,size_t n){
 char path[256];snprintf(path,sizeof(path),"cved-unit-%ld-%ld.txt",(long)time(NULL),(long)rand());
 FILE *f=fopen(path,"wb");assert(f);assert(fwrite(bytes,1,n,f)==n);assert(!fclose(f));
 Buffer b;buf_init(&b);assert(!buf_read(&b,path,0));
 assert(!buf_write(&b,path,0));assert(!b.modified);
 f=fopen(path,"rb");assert(f);char *copy=malloc(n+1);assert(copy);assert(fread(copy,1,n,f)==n);assert(fgetc(f)==EOF);
 assert(!memcmp(copy,bytes,n));fclose(f);free(copy);buf_free(&b);remove(path);
}
int main(void){
 srand(1234);
 test("",0);test("abc",3);test("abc\n",4);test("\n\n",2);
 test("\xEF\xBB\xBF\xC3\xA1\r\nhi\n",10);
 test("a1\r\n\xF0\x9F\x98\x80\rxyz",12);
 test("\xC3\xA1",2);
 puts("test_roundtrip: OK");return 0;
}
