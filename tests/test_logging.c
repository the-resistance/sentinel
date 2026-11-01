#include "sentinel.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <unistd.h>
int main(void) {
 char directory[]="/tmp/sentinel-log-test-XXXXXX"; assert(mkdtemp(directory));
 char path[SN_PATH],backup[SN_PATH],message[1800];
 assert(!sn_path(path,sizeof(path),"%s/diagnostics.jsonl",directory));
 assert(!sn_path(backup,sizeof(backup),"%s.1",path));
 assert(sn_log_level("trace")==SN_LOG_TRACE&&sn_log_level("bad")==-1);
 sn_log_init(SN_LOG_WARN); assert(!sn_log_open(path));
 snprintf(sn_error,sizeof(sn_error),"original error"); errno=EDOM;
 sn_log(SN_LOG_DEBUG,"test","must_be_filtered");
 sn_log(SN_LOG_WARN,"test","quotes=\" slash=\\ newline=\n");
 assert(errno==EDOM&&!strcmp(sn_error,"original error")); assert(!sn_log_close());
 FILE *f=fopen(path,"r"); assert(f); char line[12288]; assert(fgets(line,sizeof(line),f));
 assert(strstr(line,"\\\"")&&strstr(line,"\\u000a")&&strstr(line,"\"level\":\"warn\"")&&!strstr(line,"must_be_filtered")); assert(!fgets(line,sizeof(line),f)); assert(!fclose(f));
 sn_log_init(SN_LOG_TRACE); assert(!sn_log_open(path));
 memset(message,'A',sizeof(message)-1); message[sizeof(message)-1]=0;
 for(unsigned i=0;i<6000;++i) sn_log(SN_LOG_TRACE,"rotation","%u %s",i,message);
 assert(!sn_log_close());
 struct stat st; assert(!stat(path,&st)&&st.st_size>0&&st.st_size<=8*1024*1024); assert(!stat(backup,&st)&&st.st_size>0&&st.st_size<=8*1024*1024);
 f=fopen(path,"r"); assert(f); assert(fgets(line,sizeof(line),f)&&strstr(line,"\"truncated\":true")); assert(!fclose(f));
 assert(!unlink(path)&&!unlink(backup));
 assert(!mkfifo(path,0600)); sn_log_init(SN_LOG_TRACE); assert(sn_log_open(path)<0); assert(!unlink(path));
 assert(!symlink("/dev/null",path)); assert(sn_log_open(path)<0); assert(!unlink(path));
 struct rlimit original,limited; assert(!getrlimit(RLIMIT_FSIZE,&original)); limited=original; limited.rlim_cur=512;
 assert(signal(SIGXFSZ,SIG_IGN)!=SIG_ERR); assert(!setrlimit(RLIMIT_FSIZE,&limited));
 sn_log_init(SN_LOG_TRACE); assert(!sn_log_open(path)); snprintf(sn_error,sizeof(sn_error),"primary error"); errno=EDOM; sn_log(SN_LOG_TRACE,"disk_failure","%s",message);
 assert(errno==EDOM&&!strcmp(sn_error,"primary error"));
 assert(sn_log_close()<0); assert(!setrlimit(RLIMIT_FSIZE,&original)); assert(!unlink(path));
 assert(!rmdir(directory));
 puts("logging checks passed: escaping/filtering/truncation/rotation/error preservation/nonregular rejection/write failure"); return 0;
}
