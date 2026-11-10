#ifndef CVED_H
#define CVED_H
#include <stdio.h>
#include <stddef.h>
typedef enum { CVED_EXACT=1, CVED_UNICODE_ONLY=2, CVED_INVALID_RAW=3 } CvedStatus;
typedef enum { CVED_EOL_NONE=0, CVED_EOL_LF=1, CVED_EOL_CRLF=2, CVED_EOL_CR=3 } CvedEol;
typedef struct {
    char *raw; char *uni; size_t raw_len,uni_len;
    CvedStatus status; CvedEol eol;
} Line;
typedef struct {
    Line *lines; size_t count,cap,cur; char *filename;
    int modified; int trailing_bom;
} Buffer;
void line_init(Line *l);
void line_free(Line *l);
int line_set_raw(Line *l,const char *raw);
int line_set_unicode(Line *l,const char *uni,size_t n,CvedEol eol);
int line_clone(Line *dst,const Line *src);
void buf_init(Buffer *b);
void buf_free(Buffer *b);
int buf_insert_at(Buffer *b,size_t pos,const Line *l);
int buf_delete_range(Buffer *b,size_t start,size_t end);
int buf_replace(Buffer *b,size_t at,const Line *l);
int buf_copy(Buffer *dst,const Buffer *src);
int buf_read(Buffer *b,const char *path,int append); /* transactionally reads */
int buf_write(Buffer *b,const char *path,int force);
int cmd_run(Buffer *b,FILE *in,FILE *out,FILE *err,int interactive);
#endif
