#include "cvnss_engine.h"
#include "utf8.h"
#include <string.h>
#include "rules_generated.h"
size_t cvnss_rule_count(void){return sizeof(CVED_DEC_RULES)/sizeof(CVED_DEC_RULES[0]);}
const char *cvnss_lookup_decode(const char *k){
 size_t a=0,b=sizeof(CVED_DEC_RULES)/sizeof(CVED_DEC_RULES[0]);
 while(a<b){size_t m=a+(b-a)/2;int c=strcmp(k,CVED_DEC_RULES[m].key);
  if(c==0)return CVED_DEC_RULES[m].value;
  if(c<0)b=m;else a=m+1;
 }return NULL;
}
const char *cvnss_lookup_encode(const char *k){
 size_t n=sizeof(CVED_ENC_RULES)/sizeof(CVED_ENC_RULES[0]);
 for(size_t i=0;i<n;i++)if(!strcmp(k,CVED_ENC_RULES[i].key))return CVED_ENC_RULES[i].value;
 return NULL;
}
/* Code onsets as specified by CVNSS4.0 v5 JavaScript reference. */
int cvnss_is_onset(const char *s){
 static const char *a[]={"ch","nh","th","tr","b","c","d","f","g","h","j","k","l","m","n","q","r","s","t","v","w","x","z"};
 for(size_t i=0;i<sizeof(a)/sizeof(a[0]);i++)if(!strcmp(s,a[i]))return 1;
 return 0;
}
static int in_cp(uint32_t cp,const char *s){size_t n=strlen(s);for(size_t i=0;i<n;){uint32_t t;size_t z=utf8_char(s+i,n-i,&t);if(!z)break;if(cp==t)return 1;i+=z;}return 0;}
int cvnss_is_vowel_ie(const char *r){uint32_t cp;return r&&*r&&utf8_char(r,strlen(r),&cp)&&in_cp(cp,"iìỉĩíịeèẻẽéẹêềểễếệ");}
int cvnss_is_vowel_u(const char *r){uint32_t cp;return r&&*r&&utf8_char(r,strlen(r),&cp)&&in_cp(cp,"uùủũúụưừửữứự");}
const char *cvnss_restore_onset(const char *code,const char *r){
 if(!strcmp(code,"q"))return "qu";
 if(!strcmp(code,"j"))return "gi";
 if(!strcmp(code,"z"))return "d";
 if(!strcmp(code,"d"))return "đ";
 if(!strcmp(code,"f"))return "ph";
 if(!strcmp(code,"k"))return "kh";
 if(!strcmp(code,"w"))return cvnss_is_vowel_ie(r)?"ngh":"ng";
 if(!strcmp(code,"g"))return cvnss_is_vowel_ie(r)?"gh":"g";
 if(!strcmp(code,"c"))return cvnss_is_vowel_ie(r)?"k":"c";
 return code;
}
/* Optional Base27 research transport: not compression, not a ternary CPU. */
int base27_encode(const char *raw,char **out){
 if(!raw||!out)return -1;
 CvedStr b;str_init(&b);
 for(const unsigned char *p=(const unsigned char *)raw;*p;p++){
  int v=*p==' '?26:*p>='A'&&*p<='Z'?*p-'A':*p>='a'&&*p<='z'?*p-'a':-1;
  if(v<0){str_free(&b);return -1;}
  if(str_putc(&b,(char)('0'+v/9))||str_putc(&b,(char)('0'+v/3%3))||str_putc(&b,(char)('0'+v%3))){str_free(&b);return -1;}
 }
 *out=str_take(&b);return *out?0:-1;
}
int base27_decode(const char *trits,char **out){
 if(!trits||!out)return -1;
 size_t n=strlen(trits);if(n%3)return -1;
 CvedStr b;str_init(&b);
 for(size_t i=0;i<n;i+=3){
  for(int j=0;j<3;j++)if(trits[i+j]<'0'||trits[i+j]>'2'){str_free(&b);return -1;}
  int v=(trits[i]-'0')*9+(trits[i+1]-'0')*3+(trits[i+2]-'0');
  if(str_putc(&b,(char)(v==26?' ':'A'+v))){str_free(&b);return -1;}
 }
 *out=str_take(&b);return *out?0:-1;
}
