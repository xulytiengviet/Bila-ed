#include "cvnss_engine.h"
#include "utf8.h"
#include <stdlib.h>
#include <string.h>
static char *duplicate(const char *s){size_t n=strlen(s);char *d=malloc(n+1);if(d)memcpy(d,s,n+1);return d;}
static int onset_token(const char *s,size_t n){char temp[8];if(n==0||n>=sizeof(temp))return 0;memcpy(temp,s,n);temp[n]=0;return cvnss_is_onset(temp);}
static int is_i(uint32_t cp){const char *set="iìỉĩíị";size_t n=strlen(set);for(size_t i=0;i<n;){uint32_t x;size_t z=utf8_char(set+i,n-i,&x);if(x==cp)return 1;i+=z;}return 0;}
static char *decode_word(const char *word){
 size_t n=strlen(word);int upper=1,title=(n>0&&word[0]>='A'&&word[0]<='Z');
 char *lower=duplicate(word);if(!lower)return NULL;
 for(size_t i=0;i<n;i++){if(lower[i]>='A'&&lower[i]<='Z')lower[i]=(char)(lower[i]-'A'+'a');if(word[i]>='a'&&word[i]<='z')upper=0;}
 const char *v=cvnss_lookup_decode(lower);char initial[8]="";
 if(!v){
  /* longest onset first; prefer a known rime over unknown literal suffix */
  for(size_t len=2;len>=1;len--){
   if(n>len&&onset_token(lower,len)&&cvnss_lookup_decode(lower+len)){
    memcpy(initial,lower,len);initial[len]=0;v=cvnss_lookup_decode(lower+len);break;
   }
   if(len==1)break;
  }
 }
 if(!v){
   /* Same safe passthrough fallback as the JS converter for unknown tokens. */
   free(lower);return duplicate(word);
 }
 const char *onset=cvnss_restore_onset(initial,v);
 CvedStr b;str_init(&b);uint32_t cp=0;size_t vlen=strlen(v),first=utf8_char(v,vlen,&cp);
 if(!initial[0] && is_i(cp) && strncmp(v,"ia",2)){
  /* Canonical zero-onset i/y behavior from reference. */
  static const char *iarr[]={"i","ì","ỉ","ĩ","í","ị"};
  static const char *yarr[]={"y","ỳ","ỷ","ỹ","ý","ỵ"};
  for(size_t j=0;j<6;j++)if(strlen(iarr[j])==first&&!memcmp(v,iarr[j],first)){
   if(str_add(&b,yarr[j],strlen(yarr[j]))||str_add(&b,v+first,vlen-first))goto fail;
   break;
  }
 }else if(!strcmp(onset,"gi") && is_i(cp)){
  /* In the reference the leading i is preserved after g before an allowed coda. */
  const char *rest=v+first;
  int short_coda=!*rest||!strcmp(rest,"m")||!strcmp(rest,"n")||!strcmp(rest,"p")||!strcmp(rest,"t")||!strcmp(rest,"c")||!strcmp(rest,"ch")||!strcmp(rest,"ng")||!strcmp(rest,"nh");
  if(short_coda){if(str_add(&b,"g",1)||str_add(&b,v,vlen))goto fail;}
  else {if(str_add(&b,onset,strlen(onset))||str_add(&b,rest,strlen(rest)))goto fail;}
 }else if(!strcmp(onset,"qu")&&cvnss_is_vowel_u(v)&&vlen>first){
  if(str_add(&b,onset,strlen(onset))||str_add(&b,v+first,vlen-first))goto fail;
 }else{if(str_add(&b,onset,strlen(onset))||str_add(&b,v,vlen))goto fail;}
 char *joined=str_take(&b);if(!joined)goto fail;
 char *result=utf8_case(joined,upper?2:title?1:0);free(joined);free(lower);return result;
fail:str_free(&b);free(lower);return NULL;
}
char *cvnss_decode(const char *raw){
 if(!raw)return NULL;
 CvedStr b;str_init(&b);size_t n=strlen(raw);
 for(size_t i=0;i<n;){
  if((unsigned char)raw[i]>=128||(unsigned char)raw[i]<32){str_free(&b);return NULL;}
  size_t j=i;while(j<n&&((raw[j]>='A'&&raw[j]<='Z')||(raw[j]>='a'&&raw[j]<='z')))j++;
  if(j==i){if(str_add(&b,raw+i,1))goto fail;i++;continue;}
  char *token=malloc(j-i+1);if(!token)goto fail;
  memcpy(token,raw+i,j-i);token[j-i]=0;
  char *decoded=decode_word(token);free(token);if(!decoded)goto fail;
  if(str_add(&b,decoded,strlen(decoded))){free(decoded);goto fail;}free(decoded);i=j;
 }
 return str_take(&b);
fail:str_free(&b);return NULL;
}
int cvnss_valid_raw(const char *raw){if(!raw)return 0;for(const unsigned char *p=(const unsigned char*)raw;*p;p++)if(*p<32||*p>126)return 0;return 1;}
