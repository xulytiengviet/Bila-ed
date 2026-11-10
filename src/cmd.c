#include "cved.h"
#include "cvnss_engine.h"
#include "utf8.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <errno.h>
static char *read_line(FILE *f){
 CvedStr s;str_init(&s);int c,got=0;
 while((c=fgetc(f))!=EOF){got=1;if(c=='\n')break;if(c==0){str_free(&s);return NULL;}
  if(s.n>(1u<<22)||str_putc(&s,(char)c)){str_free(&s);return NULL;}}
 if(!got){str_free(&s);return NULL;}
 if(s.n&&s.p[s.n-1]=='\r')s.p[--s.n]=0;
 return str_take(&s);
}
static const char *skip(const char *p){while(*p==' '||*p=='\t')p++;return p;}
static void print_ascii_unicode(FILE *out,const char *uni){
 size_t n=strlen(uni);for(size_t i=0;i<n;){uint32_t cp;size_t k=utf8_char(uni+i,n-i,&cp);
  if(!k){fputs("\\x??",out);i++;continue;}
  if(cp>=32&&cp<127)fputc((char)cp,out);
  else if(cp<=0xFFFF)fprintf(out,"\\u%04X",(unsigned)cp);
  else fprintf(out,"\\U%08X",(unsigned)cp);
  i+=k;
 }
}
static void print_line(FILE *out,size_t n,const Line *l,int number,int diagnostic){
 if(number)fprintf(out,"%zu\t",n+1);
 if(l->raw)fputs(l->raw,out);else fputs("[unicode-only]",out);
 if(diagnostic){fputs("  ->  ",out);if(l->uni)print_ascii_unicode(out,l->uni);
  fprintf(out,"  [%s, %zu UTF-8 bytes]",l->status==CVED_EXACT?"EXACT":l->status==CVED_UNICODE_ONLY?"UNICODE_ONLY":"INVALID_RAW",l->uni_len);}
 fputc('\n',out);
}
static int addr(const char **p,const Buffer *b,size_t *out,int *specified){
 const char *s=*p;s=skip(s);*specified=0;
 if(*s=='.'){*out=b->cur;*specified=1;s++;}
 else if(*s=='$'){if(!b->count)return -1;*out=b->count-1;*specified=1;s++;}
 else if(isdigit((unsigned char)*s)){
  size_t x=0;do{unsigned digit=(unsigned)(*s-'0');if(x>(SIZE_MAX-digit)/10)return -1;
    x=x*10+digit;s++;}while(isdigit((unsigned char)*s));
  if(!x||x>b->count)return -1;
  *out=x-1;*specified=1;
 }
 *p=s;return 0;
}
static int get_range(const char **p,const Buffer *b,size_t *from,size_t *to,int *specified){
 int a=0,z=0;size_t f=b->cur,t=b->cur;
 if(addr(p,b,&f,&a))return -1;
 if(**p==','){
  (*p)++;if(!a)f=0;
  if(addr(p,b,&t,&z))return -1;
  if(!z)t=b->count?b->count-1:0;
  *specified=1;
 }else{t=f;*specified=a;}
 if(*specified && (!b->count||f>t||t>=b->count))return -1;
 *from=f;*to=t;return 0;
}
static int input_mode(Buffer *b,FILE *in,FILE *out,FILE *err,int interactive,size_t pos){
 for(;;){if(interactive){fputs("[input] > ",out);fflush(out);}
  char *line=read_line(in);if(!line)break;
  if(!strcmp(line,".")){free(line);break;}
  Line l;line_init(&l);int ok=line_set_raw(&l,line);
  if(ok||buf_insert_at(b,pos,&l)){fprintf(err,"cved: cannot insert line at %zu (raw ASCII required)\n",pos+1);line_free(&l);free(line);return -1;}
  pos++;line_free(&l);free(line);
 }
 return 0;
}
static char *replace_raw(const char *s,const char *old,const char *with,int global,int *num){
 size_t n=strlen(old);CvedStr out;str_init(&out);const char *p=s;*num=0;
 if(n==0)return NULL;
 while(1){const char *q=strstr(p,old);
  if(!q){if(str_add(&out,p,strlen(p)))goto fail;break;}
  if(str_add(&out,p,(size_t)(q-p))||str_add(&out,with,strlen(with)))goto fail;
  (*num)++;p=q+n;
  if(!global){if(str_add(&out,p,strlen(p)))goto fail;break;}
 }
 return str_take(&out);
fail:str_free(&out);return NULL;
}
static int subst(Buffer *b,size_t f,size_t t,const char *p,FILE *out,FILE *err){
 if(!b->count||f>t||t>=b->count||*p!='/')return -1;
 const char *a=++p;const char *slash=strchr(p,'/');if(!slash)return -1;
 const char *q=slash+1;const char *end=strchr(q,'/');if(!end)return -1;
 size_t an=(size_t)(slash-a),bn=(size_t)(end-q);if(!an)return -1;
 char *old=malloc(an+1),*replacement=malloc(bn+1);if(!old||!replacement){free(old);free(replacement);return -1;}
 memcpy(old,a,an);old[an]=0;memcpy(replacement,q,bn);replacement[bn]=0;
 int global=end[1]=='g';if(end[1]&&(!global||end[2])){free(old);free(replacement);return -1;}
 size_t total=0;
 /* Stage all new lines before changing the buffer so failure is non-destructive. */
 size_t count=t-f+1;Line *staged=calloc(count,sizeof(Line));if(!staged){free(old);free(replacement);return -1;}
 for(size_t i=0;i<count;i++){
  line_init(&staged[i]);Line *src=&b->lines[f+i];
  if(!src->raw){fprintf(err,"cved: cannot substitute Unicode-only line %zu\n",f+i+1);goto fail;}
  int replacements=0;char *raw=replace_raw(src->raw,old,replacement,global,&replacements);if(!raw)goto fail;
  if(line_set_raw(&staged[i],raw)){free(raw);goto fail;}
  staged[i].eol=src->eol;free(raw);total+=(size_t)replacements;
 }
 if(total){for(size_t i=0;i<count;i++){
  line_free(&b->lines[f+i]);b->lines[f+i]=staged[i];line_init(&staged[i]);}
  b->modified=1;b->cur=t;
 }
 for(size_t i=0;i<count;i++)line_free(&staged[i]);
 free(staged);free(old);free(replacement);
 fprintf(out,"%zu substitution(s)\n",total);return 0;
fail:for(size_t i=0;i<count;i++)line_free(&staged[i]);
 free(staged);free(old);free(replacement);return -1;
}
static int help(FILE *out){fputs("Bila-ed / CVED | ASCII terminal, UTF-8 file | Long Ngo | MIT\n"
 "[N[,M]] p/n/P/d/c | a | i | s/old/new/[g] | w [file] | wq [file] | q/q! | r file | e file | V | B | = | h\n"
 "Input lines end with a single dot. p/n print ASCII. P prints Unicode escapes. B prints Base27 trits.\n",out);return 0;}
int cmd_run(Buffer *b,FILE *in,FILE *out,FILE *err,int interactive){
 int exitcode=0;
 for(;;){if(interactive){fputs(b->modified?"* cved> ":"cved> ",out);fflush(out);}
  char *line=read_line(in);if(!line)break;
  const char *p=skip(line);if(!*p){if(b->count){if(b->cur+1<b->count)b->cur++;print_line(out,b->cur,&b->lines[b->cur],0,0);}free(line);continue;}
  if(*p=='#'){free(line);continue;}
  size_t f=b->cur,t=b->cur;int specified=0;
  if(get_range(&p,b,&f,&t,&specified)){fputs("? bad address\n",err);free(line);exitcode=1;continue;}
  char cmd=*p;if(!cmd){free(line);continue;}p++;
  int ok=0;
  switch(cmd){
  case 'a':case 'i':{
   size_t pos=cmd=='i'?(b->count?f:0):(b->count?f+1:0);
   if(specified&&!b->count)ok=-1;
   else ok=input_mode(b,in,out,err,interactive,pos);
   break;}
  case 'c':{
   if(!b->count){ok=-1;break;}
   if(buf_delete_range(b,f,t)){ok=-1;break;}
   ok=input_mode(b,in,out,err,interactive,f);break;}
  case 'd':ok=buf_delete_range(b,f,t);break;
  case 'p':case 'n':case 'P':{
   if(!b->count||t>=b->count){ok=-1;break;}
   for(size_t i=f;i<=t;i++)print_line(out,i,&b->lines[i],cmd!='p',cmd=='P');
   b->cur=t;break;}
  case 's':ok=subst(b,f,t,p,out,err);break;
  case 'V':{
   size_t mismatches=0,unicode=0;
   for(size_t i=0;i<b->count;i++){
    Line *l=&b->lines[i];if(l->status==CVED_UNICODE_ONLY){unicode++;continue;}
    char *test=l->raw?cvnss_decode(l->raw):NULL;
    if(!test||strcmp(test,l->uni)){mismatches++;fprintf(err,"cved: conversion mismatch line %zu\n",i+1);}
    free(test);
   }
   fprintf(out,"verify: %zu lines; %zu mismatches; %zu unicode-only\n",b->count,mismatches,unicode);if(mismatches)ok=-1;break;}
  case 'B':{
   if(!b->count||f>=b->count||!b->lines[f].raw){ok=-1;break;}
   char *trits=NULL;if(base27_encode(b->lines[f].raw,&trits)){ok=-1;break;}
   fprintf(out,"%s\n",trits);free(trits);break;}
  case 'w':{
   int quit=0,force=0;if(*p=='q'){quit=1;p++;}else if(*p=='!'){force=1;p++;}
   p=skip(p);const char *name=*p?p:b->filename;
   if(!name){fputs("cved: no file name\n",err);ok=-1;break;}
   ok=buf_write(b,name,force);
   if(!ok){fprintf(out,"%s: written %zu lines (UTF-8)\n",name,b->count);if(quit){free(line);return exitcode;}}
   break;}
  case 'q':{
   int force=*p=='!';if(b->modified&&!force){fputs("? buffer modified; use w or q!\n",err);ok=-1;}
   else{free(line);return exitcode;}break;}
  case 'r':p=skip(p);if(!*p)ok=-1;else ok=buf_read(b,p,1);break;
  case 'e':p=skip(p);if(!*p||b->modified){fputs("? save or q! before e\n",err);ok=-1;}
   else ok=buf_read(b,p,0);
   break;
  case '=':fprintf(out,"%zu\n",b->count?b->cur+1:0);break;
  case 'h':case '?':help(out);break;
  default:ok=-1;break;
  }
  if(ok){fputs("? command failed\n",err);exitcode=1;}
  free(line);
 }
 if(b->modified){fputs("cved: warning: unsaved buffer at end of input\n",err);return 1;}
 return exitcode;
}
