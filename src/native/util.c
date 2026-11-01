#include "sentinel.h"
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
volatile sig_atomic_t sn_cancelled;
char sn_error[512];
int sn_fail(const char *format,...) {
 va_list args; va_start(args,format); vsnprintf(sn_error,sizeof(sn_error),format,args); va_end(args); return -1;
}
double sn_clock(void) {
 struct timespec t; if(clock_gettime(CLOCK_MONOTONIC,&t)) return 0;
 return (double)t.tv_sec+(double)t.tv_nsec/1e9;
}
int sn_utc(char out[40]) {
 struct timespec t; struct tm utc; char date[25];
 if(clock_gettime(CLOCK_REALTIME,&t)||!gmtime_r(&t.tv_sec,&utc)||!strftime(date,sizeof(date),"%Y-%m-%dT%H:%M:%S",&utc)) return sn_fail("clock unavailable");
 snprintf(out,40,"%s.%06u",date,(unsigned)(t.tv_nsec/1000)); return 0;
}
int sn_path(char *out,size_t n,const char *format,...) {
 va_list args; va_start(args,format); int r=vsnprintf(out,n,format,args); va_end(args);
 return r<0||(size_t)r>=n?sn_fail("path exceeds supported length"):0;
}
int sn_mkdirs(const char *path) {
 char copy[SN_PATH]; if(!path||!*path||sn_path(copy,sizeof(copy),"%s",path)) return sn_fail("invalid directory path");
 for(char *p=copy+1;;++p) if(*p=='/'||!*p) {
  char saved=*p; *p=0;
  if(mkdir(copy,0700)&&errno!=EEXIST) return sn_fail("mkdir %s: %s",copy,strerror(errno));
  struct stat st; if(stat(copy,&st)||!S_ISDIR(st.st_mode)) return sn_fail("not a directory: %s",copy);
  *p=saved; if(!saved) break;
 }
 return 0;
}
int sn_id(char out[33]) {
 unsigned char raw[16]; int fd=open("/dev/urandom",O_RDONLY|O_CLOEXEC);
 if(fd<0) return sn_fail("random source unavailable");
 size_t done=0;
 while(done<sizeof(raw)) {
  ssize_t n=read(fd,raw+done,sizeof(raw)-done);
  if(n<0&&errno==EINTR&&!sn_cancelled) continue;
  if(n<=0) { close(fd); return sn_fail("random source read failed"); }
  done+=(size_t)n;
 }
 close(fd); for(size_t i=0;i<16;++i) snprintf(out+2*i,3,"%02x",raw[i]); return 0;
}
void sn_json(FILE *stream,const char *text) {
 fputc('"',stream);
 for(const unsigned char *p=(const unsigned char *)text;*p;++p) {
  if(*p=='"'||*p=='\\') { fputc('\\',stream); fputc(*p,stream); }
  else if(*p<32||*p>=127) fprintf(stream,"\\u%04x",*p);
  else fputc(*p,stream);
 }
 fputc('"',stream);
}
int sn_atomic(FILE *stream,const char *temporary,const char *final) {
 int error=ferror(stream); if(fflush(stream)||fsync(fileno(stream))) error=1;
 if(fclose(stream)) error=1;
 if(error) { unlink(temporary); return sn_fail("write failed: %s",final); }
 if(rename(temporary,final)) { unlink(temporary); return sn_fail("rename %s: %s",final,strerror(errno)); }
 return 0;
}

FILE *sn_open_regular(const char *path) {
 int fd=open(path,O_RDONLY|O_NONBLOCK|O_CLOEXEC);
 if(fd<0) { sn_fail("open %s: %s",path,strerror(errno)); return NULL; }
 struct stat st;
 if(fstat(fd,&st)||!S_ISREG(st.st_mode)) {
  close(fd); errno=EINVAL; sn_fail("input must be a regular file: %s",path); return NULL;
 }
 FILE *stream=fdopen(fd,"rb");
 if(!stream) { int saved=errno; close(fd); errno=saved; sn_fail("open stream %s: %s",path,strerror(errno)); }
 return stream;
}
