#include "cvnss_engine.h"
#include "utf8.h"
#include <stdlib.h>
#include <string.h>
static int is_ascii_letter(char c){return (c>='a'&&c<='z')||(c>='A'&&c<='Z');}
static char *word_encode(const char *orig){
 char *word=utf8_case(orig,0);if(!word)return NULL;
 const char *pairs[][2]={
  {"ngh","w"},{"ng","w"},{"ph","f"},{"qu","q"},{"kh","k"},{"gi","j"},{"gh","g"},
  {"ch","ch"},{"nh","nh"},{"th","th"},{"tr","tr"},{"đ","d"},
  {"k","c"},{"d","z"},{"b","b"},{"c","c"},{"g","g"},{"h","h"},
  {"l","l"},{"m","m"},{"n","n"},{"r","r"},{"s","s"},{"t","t"},{"v","v"},{"x","x"}
 };
 const char *onset="",*vowel=word;size_t m=sizeof(pairs)/sizeof(pairs[0]);
 for(size_t i=0;i<m;i++){size_t k=strlen(pairs[i][0]);if(!strncmp(word,pairs[i][0],k)){onset=pairs[i][1];vowel=word+k;break;}}
 /* The source reference normalizes orthographic g/c/ng before front vowels. */
 if(!strcmp(onset,"g")&&cvnss_is_vowel_ie(vowel)){onset="j";}
 if(!strcmp(onset,"c")&&cvnss_is_vowel_ie(vowel)){onset="c";}
 if(!strcmp(onset,"w")&&cvnss_is_vowel_ie(vowel)){onset="w";}
 CvedStr joined;str_init(&joined);
 if(!strcmp(onset,"j") && strcmp(vowel,"a")){
  CvedStr candidate;str_init(&candidate);str_add(&candidate,"i",1);str_add(&candidate,vowel,strlen(vowel));
  const char *code=cvnss_lookup_encode(candidate.p);if(code)vowel=candidate.p;
  else{str_free(&candidate);candidate.p=NULL;}
  if(code){str_add(&joined,onset,strlen(onset));str_add(&joined,code,strlen(code));str_free(&candidate);goto built;}
 }
 if(!strcmp(onset,"q") && vowel[0]=='y'){
  CvedStr candidate;str_init(&candidate);str_add(&candidate,"u",1);str_add(&candidate,vowel,strlen(vowel));
  const char *code=cvnss_lookup_encode(candidate.p);
  if(code){str_add(&joined,onset,strlen(onset));str_add(&joined,code,strlen(code));str_free(&candidate);goto built;}
  str_free(&candidate);
 }
 {
  const char *code=cvnss_lookup_encode(vowel);
  str_add(&joined,onset,strlen(onset));str_add(&joined,code?code:vowel,strlen(code?code:vowel));
 }
built:;
 char *result=joined.p?str_take(&joined):NULL;
 int mode=2;for(size_t i=0;orig[i];i++)if(is_ascii_letter(orig[i])&&orig[i]>='a'&&orig[i]<='z'){mode=0;break;}
 if(mode==0 && orig[0]>='A'&&orig[0]<='Z')mode=1;
 if(result && (mode==2||mode==1)){
   for(size_t i=0;result[i];i++){if(result[i]>='a'&&result[i]<='z'&&(mode==2||i==0))result[i]=(char)(result[i]-'a'+'A');}
 }
 free(word);return result;
}
char *cvnss_encode(const char *uni){
 if(!uni||!utf8_valid(uni,strlen(uni)))return NULL;
 CvedStr out;str_init(&out);size_t n=strlen(uni);
 for(size_t i=0;i<n;){
  uint32_t cp;size_t z=utf8_char(uni+i,n-i,&cp);if(!z)goto fail;
  if(!utf8_isletter(cp)){
   if(cp>=128 || cp<32)goto fail; /* punctuation outside ASCII: preserve as Unicode-only */
   if(str_add(&out,uni+i,z))goto fail;
   i+=z;continue;
  }
  size_t j=i;while(j<n){uint32_t c;size_t k=utf8_char(uni+j,n-j,&c);if(!k||!utf8_isletter(c))break;j+=k;}
  char *word=malloc(j-i+1);if(!word)goto fail;memcpy(word,uni+i,j-i);word[j-i]=0;
  char *encoded=word_encode(word);free(word);if(!encoded)goto fail;
  if(str_add(&out,encoded,strlen(encoded))){free(encoded);goto fail;}free(encoded);i=j;
 }
 char *raw=str_take(&out);if(!raw)return NULL;
 char *back=cvnss_decode(raw);int exact=back&&!strcmp(back,uni);free(back);
 if(!exact){free(raw);return NULL;}return raw;
fail:str_free(&out);return NULL;
}
