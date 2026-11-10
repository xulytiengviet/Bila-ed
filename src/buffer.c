#include "cved.h"
#include "cvnss_engine.h"
#include "utf8.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
static char *dup(const char *s,size_t n){char *p=malloc(n+1);if(p){memcpy(p,s,n);p[n]=0;}return p;}
void line_init(Line *l){memset(l,0,sizeof(*l));l->eol=CVED_EOL_LF;}
void line_free(Line *l){free(l->raw);free(l->uni);line_init(l);}
int line_set_raw(Line *l,const char *raw){
 if(!raw||!cvnss_valid_raw(raw))return -1;
 Line tmp;line_init(&tmp);tmp.raw=dup(raw,strlen(raw));if(!tmp.raw)return -1;
 tmp.raw_len=strlen(raw);tmp.uni=cvnss_decode(raw);
 if(!tmp.uni){line_free(&tmp);return -1;}
 tmp.uni_len=strlen(tmp.uni);tmp.status=CVED_EXACT;tmp.eol=l->eol;
 line_free(l);*l=tmp;return 0;
}
int line_set_unicode(Line *l,const char *uni,size_t n,CvedEol eol){
 if(!uni||memchr(uni,0,n)||!utf8_valid(uni,n))return -1;
 Line tmp;line_init(&tmp);tmp.eol=eol;
 tmp.uni=dup(uni,n);if(!tmp.uni)return -1;tmp.uni_len=n;
 tmp.raw=cvnss_encode(tmp.uni);
 tmp.status=tmp.raw?CVED_EXACT:CVED_UNICODE_ONLY;
 tmp.raw_len=tmp.raw?strlen(tmp.raw):0;
 line_free(l);*l=tmp;return 0;
}
int line_clone(Line *dst,const Line *src){
 line_init(dst);dst->eol=src->eol;dst->status=src->status;
 if(src->raw){dst->raw=dup(src->raw,src->raw_len);if(!dst->raw)goto fail;dst->raw_len=src->raw_len;}
 if(src->uni){dst->uni=dup(src->uni,src->uni_len);if(!dst->uni)goto fail;dst->uni_len=src->uni_len;}
 return 0;
fail:line_free(dst);return -1;
}
void buf_init(Buffer *b){memset(b,0,sizeof(*b));}
void buf_free(Buffer *b){for(size_t i=0;i<b->count;i++)line_free(&b->lines[i]);free(b->lines);free(b->filename);buf_init(b);}
static int reserve(Buffer *b){
 if(b->count<b->cap)return 0;
 size_t c=b->cap?b->cap*2:16;
 if(c<b->cap||c>SIZE_MAX/sizeof(Line))return -1;
 Line *p=realloc(b->lines,c*sizeof(Line));if(!p)return -1;b->lines=p;b->cap=c;return 0;
}
int buf_insert_at(Buffer *b,size_t pos,const Line *src){
 if(pos>b->count||!src)return -1;
 Line clone;if(line_clone(&clone,src))return -1;
 if(reserve(b)){line_free(&clone);return -1;}
 if(pos==b->count&&pos>0&&b->lines[pos-1].eol==CVED_EOL_NONE)b->lines[pos-1].eol=CVED_EOL_LF;
 memmove(b->lines+pos+1,b->lines+pos,(b->count-pos)*sizeof(Line));b->lines[pos]=clone;
 b->count++;b->cur=pos;b->modified=1;return 0;
}
int buf_delete_range(Buffer *b,size_t from,size_t to){
 if(from>to||to>=b->count)return -1;
 for(size_t i=from;i<=to;i++)line_free(&b->lines[i]);
 memmove(b->lines+from,b->lines+to+1,(b->count-to-1)*sizeof(Line));
 b->count-=to-from+1;b->cur=b->count?(from<b->count?from:b->count-1):0;b->modified=1;return 0;
}
int buf_replace(Buffer *b,size_t index,const Line *src){
 if(index>=b->count)return -1;
 Line clone;if(line_clone(&clone,src))return -1;
 clone.eol=b->lines[index].eol;line_free(&b->lines[index]);b->lines[index]=clone;b->cur=index;b->modified=1;return 0;
}
int buf_copy(Buffer *dst,const Buffer *src){
 Buffer tmp;buf_init(&tmp);
 for(size_t i=0;i<src->count;i++)if(buf_insert_at(&tmp,tmp.count,&src->lines[i]))goto fail;
 if(src->filename){tmp.filename=dup(src->filename,strlen(src->filename));if(!tmp.filename)goto fail;}
 tmp.modified=src->modified;tmp.trailing_bom=src->trailing_bom;tmp.cur=src->cur;
 buf_free(dst);*dst=tmp;return 0;
fail:buf_free(&tmp);return -1;
}
