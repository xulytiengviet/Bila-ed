#include "cved.h"
#include "cvnss_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#include <wchar.h>
#define ISATTY _isatty
#define FILENO _fileno
#else
#include <unistd.h>
#define ISATTY isatty
#define FILENO fileno
#endif
static void usage(void){
 puts("Bila-ed (cved) 0.1.0 | C11 | Long Ngo | MIT\n"
 "Usage: cved [FILE] | --decode RAW | --encode UTF8 | --base27 TEXT | --unbase27 TRITS | --version\n"
 "Editor is ASCII-only on stdout; UTF-8 is emitted only to files or explicitly via --decode.");
}
static int cved_main(int argc,char **argv){
 if(argc>1 && (!strcmp(argv[1],"--help")||!strcmp(argv[1],"-h"))){usage();return 0;}
 if(argc>1&&!strcmp(argv[1],"--version")){puts("Bila-ed 0.1.0 (CVNSS4.0 audit-safe tables v5.0.0)");return 0;}
 if(argc>2){char *out=NULL;int code=0;
  if(!strcmp(argv[1],"--decode"))out=cvnss_decode(argv[2]);
  else if(!strcmp(argv[1],"--encode"))out=cvnss_encode(argv[2]);
  else if(!strcmp(argv[1],"--base27"))code=base27_encode(argv[2],&out);
  else if(!strcmp(argv[1],"--unbase27"))code=base27_decode(argv[2],&out);
  else{usage();return 2;}
  if(!out||code){fputs("cved: conversion not exactly representable\n",stderr);free(out);return 1;}
  puts(out);free(out);return 0;
 }
 if(argc>2){usage();return 2;}
 Buffer b;buf_init(&b);
 if(argc==2){
  FILE *f=fopen(argv[1],"rb");if(f){fclose(f);if(buf_read(&b,argv[1],0)){buf_free(&b);return 1;}}
  else {if(errno!=ENOENT){perror(argv[1]);return 1;}
   b.filename=malloc(strlen(argv[1])+1);if(!b.filename)return 1;strcpy(b.filename,argv[1]);}
 }
 int status=cmd_run(&b,stdin,stdout,stderr,ISATTY(FILENO(stdin))!=0);
 buf_free(&b);return status;
}

#ifdef _WIN32
/* MinGW-w64: build with -municode. Preserve Unicode CLI arguments as UTF-8
   rather than relying on the active Windows OEM/ANSI code page. */
int wmain(int argc,wchar_t **wargv){
 char **argv=calloc((size_t)argc+1,sizeof(*argv));
 if(!argv)return 2;
 int rc=2;
 for(int i=0;i<argc;i++){
  int count=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,wargv[i],-1,NULL,0,NULL,NULL);
  if(count<=0)goto done;
  argv[i]=malloc((size_t)count);
  if(!argv[i])goto done;
  if(!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,wargv[i],-1,argv[i],count,NULL,NULL))goto done;
 }
 rc=cved_main(argc,argv);
done:
 for(int i=0;i<argc;i++)free(argv[i]);
 free(argv);return rc;
}
#else
int main(int argc,char **argv){return cved_main(argc,argv);}
#endif
