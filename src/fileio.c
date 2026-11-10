#include "cved.h"
#include "utf8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <windows.h>
#define PID _getpid()
#define SYNC(fd) _commit(fd)
#define FILENO(f) _fileno(f)
#define OPEN _open
#define FDOPEN _fdopen
#define CLOSE _close
#define FLAGS (_O_CREAT|_O_EXCL|_O_WRONLY|_O_BINARY)
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#define PID getpid()
#define SYNC(fd) fsync(fd)
#define FILENO(f) fileno(f)
#define OPEN open
#define FDOPEN fdopen
#define CLOSE close
#define FLAGS (O_CREAT|O_EXCL|O_WRONLY)
#endif
static char *str_copy(const char *p){size_t n=strlen(p);char *x=malloc(n+1);if(x)memcpy(x,p,n+1);return x;}
static int read_one(FILE *f,char **buf,size_t *n,CvedEol *eol){
 CvedStr s;str_init(&s);int c=0,have=0;*eol=CVED_EOL_NONE;
 while((c=fgetc(f))!=EOF){have=1;
  if(c==0||s.n>(1u<<22)){str_free(&s);errno=EILSEQ;return -1;}
  if(c=='\n'){*eol=CVED_EOL_LF;break;}
  if(c=='\r'){
   int x=fgetc(f);if(x=='\n')*eol=CVED_EOL_CRLF;
   else{*eol=CVED_EOL_CR;if(x!=EOF)ungetc(x,f);}
   break;
  }
  if(str_putc(&s,(char)c)){str_free(&s);errno=ENOMEM;return -1;}
 }
 if(ferror(f)){str_free(&s);return -1;}
 if(!have){str_free(&s);return 0;}
 *n=s.n;*buf=str_take(&s);return *buf?1:-1;
}
int buf_read(Buffer *b,const char *path,int append){
 if(!path||!*path)return -1;
 FILE *f=fopen(path,"rb");if(!f){perror(path);return -1;}
 Buffer tmp;buf_init(&tmp);int ok=0;size_t lineno=0;
 if(append&&buf_copy(&tmp,b))goto done;
 for(;;){char *bytes=NULL;size_t n=0;CvedEol eol;
  int status=read_one(f,&bytes,&n,&eol);
  if(status<0){free(bytes);goto done;}if(status==0)break;
  if(lineno==0&&!append&&n>=3&&!memcmp(bytes,"\xEF\xBB\xBF",3)){
   tmp.trailing_bom=1;memmove(bytes,bytes+3,n-2);n-=3;
  }
  Line l;line_init(&l);if(line_set_unicode(&l,bytes,n,eol)){free(bytes);line_free(&l);errno=EILSEQ;goto done;}
  free(bytes);
  if(buf_insert_at(&tmp,tmp.count,&l)){line_free(&l);errno=ENOMEM;goto done;}
  line_free(&l);lineno++;
 }
 if(!append){tmp.filename=str_copy(path);if(!tmp.filename){errno=ENOMEM;goto done;}}
 tmp.modified=append?1:0;tmp.cur=tmp.count?tmp.count-1:0;
 buf_free(b);*b=tmp;ok=1;
done:
 if(!ok){perror("cved read");buf_free(&tmp);}
 fclose(f);return ok?0:-1;
}
static int move_over(const char *from,const char *to){
#ifdef _WIN32
 return MoveFileExA(from,to,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)?0:-1;
#else
 return rename(from,to);
#endif
}
int buf_write(Buffer *b,const char *path,int force){
 (void)force;
 if(!path||!*path)return -1;
 for(size_t i=0;i<b->count;i++){
  Line *l=&b->lines[i];
  if(l->status==CVED_INVALID_RAW||!l->uni||!utf8_valid(l->uni,l->uni_len)){
   fprintf(stderr,"cved: line %zu has invalid content; refusing write\n",i+1);return -1;
  }
 }
 size_t plen=strlen(path);char *tmp=malloc(plen+64);if(!tmp)return -1;
 int fd=-1;for(unsigned attempt=0;attempt<128;attempt++){
  snprintf(tmp,plen+64,"%s.cvedtmp.%ld.%u",path,(long)PID,attempt);
  fd=OPEN(tmp,FLAGS,0600);if(fd>=0)break;
  if(errno!=EEXIST)break;
 }
 if(fd<0){perror("cved create temp");free(tmp);return -1;}
 FILE *f=FDOPEN(fd,"wb");if(!f){CLOSE(fd);remove(tmp);free(tmp);return -1;}
 int ok=1;
 if(b->trailing_bom&&fwrite("\xEF\xBB\xBF",1,3,f)!=3)ok=0;
 for(size_t i=0;i<b->count&&ok;i++){
  Line *l=&b->lines[i];
  if(fwrite(l->uni,1,l->uni_len,f)!=l->uni_len){ok=0;break;}
  const char *eol=l->eol==CVED_EOL_LF?"\n":l->eol==CVED_EOL_CRLF?"\r\n":l->eol==CVED_EOL_CR?"\r":"";
  size_t z=strlen(eol);if(fwrite(eol,1,z,f)!=z)ok=0;
 }
 if(fflush(f)!=0)ok=0;
 if(ok&&SYNC(FILENO(f))!=0)ok=0;
 if(fclose(f)!=0)ok=0;
 if(!ok){perror("cved write");remove(tmp);free(tmp);return -1;}
#ifndef _WIN32
 struct stat st;
 if(stat(path,&st)==0)chmod(tmp,st.st_mode&07777);
#endif
 if(move_over(tmp,path)!=0){perror("cved replace");remove(tmp);free(tmp);return -1;}
#ifndef _WIN32
 /* A same-directory rename is atomic; sync of parent directory is a further durability enhancement. */
#endif
 free(tmp);b->modified=0;return 0;
}
