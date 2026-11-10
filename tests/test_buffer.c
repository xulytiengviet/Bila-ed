#include "cved.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
int main(void){
 Buffer b;buf_init(&b);Line line;line_init(&line);
 assert(line_set_raw(&line,"toiy iwy")==0);
 assert(!buf_insert_at(&b,0,&line));assert(b.count==1&&b.cur==0);
 assert(!strcmp(b.lines[0].uni,"tôi yêu"));
 assert(!buf_insert_at(&b,0,&line));assert(b.count==2&&b.cur==0);
 assert(!buf_insert_at(&b,2,&line));assert(b.count==3&&b.cur==2);
 assert(!buf_delete_range(&b,1,1));assert(b.count==2);
 assert(!buf_replace(&b,0,&line));assert(b.count==2);
 Buffer c;buf_init(&c);assert(!buf_copy(&c,&b));
 assert(c.count==b.count);assert(!strcmp(c.lines[0].uni,b.lines[0].uni));
 assert(!buf_delete_range(&b,0,1));assert(b.count==0);
 assert(buf_delete_range(&b,0,0)<0);
 line_free(&line);buf_free(&b);buf_free(&c);puts("test_buffer: OK");return 0;
}
