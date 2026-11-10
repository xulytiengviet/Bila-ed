#include "utf8.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "utf8_case_generated.h"
void str_init(CvedStr *s){s->p=NULL;s->n=s->cap=0;}
void str_free(CvedStr *s){free(s->p);str_init(s);}
int str_add(CvedStr *s,const char *p,size_t n){
 if(n>SIZE_MAX-s->n-1)return -1;
 size_t need=s->n+n+1;
 if(need>s->cap){size_t cap=s->cap?s->cap:64;
   while(cap<need){if(cap>SIZE_MAX/2){cap=need;break;}cap*=2;}
   char *q=realloc(s->p,cap);if(!q)return -1;s->p=q;s->cap=cap;}
 if(n)memcpy(s->p+s->n,p,n);
 s->n+=n;s->p[s->n]=0;return 0;
}
int str_putc(CvedStr *s,char c){return str_add(s,&c,1);}
char *str_take(CvedStr *s){if(!s->p&&str_add(s,"",0))return NULL;
 char *p=s->p;str_init(s);return p;}
size_t utf8_char(const char *p,size_t n,uint32_t *cp){
 if(n==0)return 0;
 unsigned char x=(unsigned char)p[0];
 if(x<0x80){*cp=x;return 1;}
 size_t len=x>=0xC2&&x<=0xDF?2:x>=0xE0&&x<=0xEF?3:x>=0xF0&&x<=0xF4?4:0;
 if(!len||len>n)return 0;
 uint32_t v=x & (len==2?0x1F:len==3?0x0F:0x07);
 for(size_t i=1;i<len;i++){unsigned char y=(unsigned char)p[i];if((y&0xC0)!=0x80)return 0;v=(v<<6)|(y&0x3F);}
 if((len==2&&v<0x80)||(len==3&&v<0x800)||(len==4&&v<0x10000)||v>0x10FFFF||(v>=0xD800&&v<=0xDFFF))return 0;
 *cp=v;return len;
}
int utf8_valid(const char *p,size_t n){for(size_t i=0;i<n;){uint32_t cp;size_t k=utf8_char(p+i,n-i,&cp);if(!k||cp==0)return 0;i+=k;}return 1;}
size_t utf8_emit(uint32_t cp,char p[4]){
 if(cp<=0x7F){p[0]=(char)cp;return 1;}
 if(cp<=0x7FF){p[0]=(char)(0xC0|(cp>>6));p[1]=(char)(0x80|(cp&63));return 2;}
 if(cp<=0xFFFF){p[0]=(char)(0xE0|(cp>>12));p[1]=(char)(0x80|((cp>>6)&63));p[2]=(char)(0x80|(cp&63));return 3;}
 p[0]=(char)(0xF0|(cp>>18));p[1]=(char)(0x80|((cp>>12)&63));p[2]=(char)(0x80|((cp>>6)&63));p[3]=(char)(0x80|(cp&63));return 4;
}
uint32_t utf8_upper(uint32_t cp){if(cp>='a'&&cp<='z')return cp-32;for(size_t i=0;i<sizeof(CVED_CASES)/sizeof(CVED_CASES[0]);i++)if(CVED_CASES[i].lo==cp)return CVED_CASES[i].up;return cp;}
uint32_t utf8_lower(uint32_t cp){if(cp>='A'&&cp<='Z')return cp+32;for(size_t i=0;i<sizeof(CVED_CASES)/sizeof(CVED_CASES[0]);i++)if(CVED_CASES[i].up==cp)return CVED_CASES[i].lo;return cp;}
int utf8_isletter(uint32_t cp){return (cp>='a'&&cp<='z')||(cp>='A'&&cp<='Z')||(cp>=0xC0&&cp<=0x1EF9 && cp!=0xD7&&cp!=0xF7);}
char *utf8_case(const char *s,int mode){CvedStr buf;str_init(&buf);size_t n=strlen(s),i=0;int first=1;
 while(i<n){uint32_t cp;size_t z=utf8_char(s+i,n-i,&cp);if(!z){str_free(&buf);return NULL;}
 uint32_t mapped=(mode==2||(mode==1&&first))?utf8_upper(cp):utf8_lower(cp);
 char out[4];size_t m=utf8_emit(mapped,out);if(str_add(&buf,out,m)){str_free(&buf);return NULL;}
 i+=z;first=0;}
 return str_take(&buf);
}
int utf8_equal(const char *a,const char *b){return a&&b&&!strcmp(a,b);}
