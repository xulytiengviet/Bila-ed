#include "cved.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static FILE *script(const char *s){FILE *f=tmpfile();assert(f);fwrite(s,1,strlen(s),f);rewind(f);return f;}
int main(void){
 Buffer b;buf_init(&b);FILE *out=tmpfile(),*err=tmpfile();assert(out&&err);
 FILE *in=script("a\ntoiy iwy\n\n.\n,p\nV\n1s/toiy/tizb/\nq\nw q_test.txt\nq\n");
 int rc=cmd_run(&b,in,out,err,0);
 assert(rc==1); /* q refused before w and return status records it */
 assert(b.count==2);assert(!strcmp(b.lines[0].uni,"tiếng yêu"));
 assert(!strcmp(b.lines[1].uni,""));assert(!b.modified);
 FILE *f=fopen("q_test.txt","rb");assert(f);char data[120]={0};size_t n=fread(data,1,sizeof(data)-1,f);
 assert(n>0&&strstr(data,"tiếng yêu"));fclose(f);remove("q_test.txt");
 fclose(in);fclose(out);fclose(err);buf_free(&b);puts("test_commands: OK");return 0;
}
