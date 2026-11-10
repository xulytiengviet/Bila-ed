#include "cved.h"
#include "cvnss_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef _WIN32
#include <io.h>
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
int main(int argc,char **argv){
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
