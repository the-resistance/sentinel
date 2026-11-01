#include "sentinel.h"
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Main-thread diagnostics only: never call from the USB callback or signal handler. */
static FILE *log_stream;
static char log_path[SN_PATH];
static int threshold=SN_LOG_INFO,log_failed;
static unsigned long long sequence;
static double started;
static const char *names[]={"error","warn","info","debug","trace"};
#define LOG_LIMIT (8u*1024u*1024u)
int sn_log_level(const char *name) {
 for(int i=SN_LOG_ERROR;i<=SN_LOG_TRACE;++i) if(!strcmp(name,names[i])) return i;
 return -1;
}
void sn_log_init(int level) { threshold=level; started=sn_clock(); sequence=0; log_failed=0; }
double sn_log_elapsed(void) { return started?1000*(sn_clock()-started):0; }
static FILE *open_log(const char *path) {
 int flags=O_WRONLY|O_CREAT|O_APPEND|O_CLOEXEC|O_NONBLOCK;
#ifdef O_NOFOLLOW
 flags|=O_NOFOLLOW;
#endif
 int fd=open(path,flags,0600);
 if(fd<0) { sn_fail("open diagnostics %s: %s",path,strerror(errno)); return NULL; }
 struct stat st;
 if(fstat(fd,&st)||!S_ISREG(st.st_mode)) { close(fd); sn_fail("diagnostics must be a regular file"); return NULL; }
 FILE *f=fdopen(fd,"a");
 if(!f) { close(fd); sn_fail("diagnostics stream: %s",strerror(errno)); }
 return f;
}
int sn_log_open(const char *path) {
 if(log_stream) return sn_fail("diagnostics already open");
 if(sn_path(log_path,sizeof(log_path),"%s",path)) return -1;
 log_stream=open_log(path); return log_stream?0:-1;
}
static int rotate(size_t incoming) {
 struct stat st;
 if(fstat(fileno(log_stream),&st)) return -1;
 if((uint64_t)st.st_size+incoming<=LOG_LIMIT) return 0;
 char backup[SN_PATH];
 if(sn_path(backup,sizeof(backup),"%s.1",log_path)) return -1;
 int bad=fclose(log_stream); log_stream=NULL;
 if(bad||rename(log_path,backup)) return -1;
 log_stream=open_log(log_path); return log_stream?0:-1;
}
void sn_log(int level,const char *component,const char *format,...) {
 if(level<0||level>threshold||level>SN_LOG_TRACE) return;
 int saved_errno=errno;
 char message[1536],stamp[40]="unavailable",record[12288];
 va_list args; va_start(args,format); int n=vsnprintf(message,sizeof(message),format,args); va_end(args);
 if(n<0) { log_failed=1; errno=saved_errno; return; }
 /* Preserve the caller's operational error if timestamp collection fails. */
 char saved_error[sizeof(sn_error)]; memcpy(saved_error,sn_error,sizeof(saved_error));
 sn_utc(stamp); memcpy(sn_error,saved_error,sizeof(sn_error));
 FILE *f=fmemopen(record,sizeof(record),"w");
 if(!f) { log_failed=1; errno=saved_errno; return; }
 fprintf(f,"{\"utc\":\"%sZ\",\"elapsed_ms\":%.3f,\"pid\":%ld,\"seq\":%llu,\"level\":\"%s\",\"component\":",stamp,sn_log_elapsed(),(long)getpid(),++sequence,names[level]);
 sn_json(f,component); fprintf(f,",\"message\":"); sn_json(f,message);
 fprintf(f,",\"truncated\":%s}\n",(size_t)n>=sizeof(message)?"true":"false");
 long length=ftell(f); int bad=ferror(f); if(fclose(f)) bad=1;
 if(bad||length<=0||(size_t)length>=sizeof(record)) { log_failed=1; errno=saved_errno; return; }
 if(fwrite(record,1,(size_t)length,stderr)!=(size_t)length||fflush(stderr)) log_failed=1;
 if(log_stream) {
  if(rotate((size_t)length)) { log_failed=1; }
  else if(fwrite(record,1,(size_t)length,log_stream)!=(size_t)length||fflush(log_stream)) log_failed=1;
 }
 memcpy(sn_error,saved_error,sizeof(sn_error));
 errno=saved_errno;
}
int sn_log_close(void) {
 if(log_stream&&fclose(log_stream)) log_failed=1;
 log_stream=NULL;
 return log_failed?sn_fail("diagnostic logging failed (output/rotation/close)"):0;
}
