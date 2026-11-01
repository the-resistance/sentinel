#include "sentinel.h"
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <locale.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>
static void interrupted(int number) { (void)number; sn_cancelled=1; }
static void usage(FILE *f) {
 fprintf(f,"Sentinel %s: native C, receive only\n"
 "Usage: sentinel run|scan [--replay sweep.csv] [--capture] [options]\n"
 "       sentinel analyze --input capture.cs8 [--sample-rate Hz --freq-mhz MHz]\n"
 "       sentinel analyze --captures-dir DIR [--workdir DIR]\n"
 "       sentinel capture --freq-mhz MHz [options]\n"
 "       sentinel report [--workdir DIR]\n"
 "       sentinel doctor\n"
 "Options: --workdir DIR --start-mhz N --stop-mhz N --bin-hz N --margin-db N\n"
 "         --sweeps N --timeout seconds --lna N --vga N --serial ID\n"
 "         --sample-rate Hz --duration seconds --max-targets N --captures-dir DIR\n"
 "Exit: 0 complete, 1 operational failure, 2 partial/invalid arguments, 130 interrupted.\n",SN_VERSION);
}
static int numeric(const char *text,double *out) { char *end; errno=0; *out=strtod(text,&end); return errno||end==text||*end||!isfinite(*out)?-1:0; }
static int parse(int argc,char **argv,SnConfig *c) {
 *c=(SnConfig){.command=argc>1?argv[1]:"",.workdir="runtime",.start=1e6,.stop=6e9,.bin_hz=100000,.margin=10,.timeout=10,.duration=1,.sample_rate=argc>1&&!strcmp(argv[1],"analyze")?0:8000000,.sweeps=3,.lna=16,.vga=20,.max_targets=10};
 if(argc<2) return sn_fail("a command is required");
 if(!strcmp(argv[1],"--help")||!strcmp(argv[1],"-h")) { usage(stdout); return 1; }
 if(strcmp(c->command,"run")&&strcmp(c->command,"scan")&&strcmp(c->command,"analyze")&&strcmp(c->command,"capture")&&strcmp(c->command,"report")&&strcmp(c->command,"doctor")) return sn_fail("unknown command");
 static const struct option opts[]={
  {"workdir",1,0,'w'},{"replay",1,0,'r'},{"input",1,0,'i'},{"capture",0,0,'c'},
  {"start-mhz",1,0,'a'},{"stop-mhz",1,0,'b'},{"bin-hz",1,0,'n'},{"margin-db",1,0,'m'},
  {"sweeps",1,0,'s'},{"timeout",1,0,'t'},{"lna",1,0,'l'},{"vga",1,0,'g'},
  {"duration",1,0,'d'},{"sample-rate",1,0,'f'},{"freq-mhz",1,0,'q'},{"max-targets",1,0,'x'},
  {"serial",1,0,'z'},{"captures-dir",1,0,'p'},{"help",0,0,'h'},{0,0,0,0}};
 optind=1; opterr=0; int opt;
 while((opt=getopt_long(argc-1,argv+1,"",opts,NULL))!=-1) {
  double value=0; if(strchr("abnmstlgdfqx",opt)&&numeric(optarg,&value)) return sn_fail("invalid numeric argument: %s",optarg);
  switch(opt) {
   case 'w': c->workdir=optarg; break; case 'r': c->replay=optarg; break;
   case 'i': c->input=optarg; break; case 'c': c->capture=1; break;
   case 'p': c->captures_dir=optarg; break; case 'z': c->serial=optarg; break;
   case 'a': c->start=value*1e6; break; case 'b': c->stop=value*1e6; break;
   case 'n': c->bin_hz=value; break; case 'm': c->margin=value; break;
   case 't': c->timeout=value; break; case 'd': c->duration=value; break;
   case 'f': c->sample_rate=value; break; case 'q': c->frequency=value*1e6; break;
   case 's': case 'l': case 'g': case 'x':
    if(value<0||value>1000000||floor(value)!=value) return sn_fail("count/gain must be a bounded nonnegative integer");
    if(opt=='s') c->sweeps=(unsigned)value;
    if(opt=='l') c->lna=(unsigned)value;
    if(opt=='g') c->vga=(unsigned)value;
    if(opt=='x') c->max_targets=(unsigned)value;
    break;
   case 'h': usage(stdout); return 1;
   default: return sn_fail("unknown option; use --help");
  }
 }
 if(optind!=argc-1) return sn_fail("unexpected positional arguments");
 if(!isfinite(c->start)||!isfinite(c->stop)||c->start<1e6||c->stop>6e9||c->stop<=c->start||c->bin_hz<2445||c->bin_hz>5e6||c->margin<=0||c->timeout<=0||c->timeout>3600||c->duration<=0||c->sample_rate<0||c->sample_rate>20e6||c->lna>40||c->lna%8||c->vga>62||c->vga%2||c->max_targets<1||c->max_targets>1000) return sn_fail("invalid range, gain, resolution, timeout or target limit");
 if(strcmp(c->command,"analyze")&&(c->sample_rate<2e6||c->duration*c->sample_rate>20000000||c->duration*c->sample_rate<4096)) return sn_fail("capture sample budget must be 4096..20000000");
 if(!strcmp(c->command,"run")&&!c->sweeps&&!c->replay) return sn_fail("run needs finite sweeps; use scan --sweeps 0");
 if(c->replay&&c->capture) return sn_fail("replay cannot initiate live capture");
 if(c->serial&&(strlen(c->serial)>128||strpbrk(c->serial,"\r\n"))) return sn_fail("invalid receiver serial");
 if(!strcmp(c->command,"analyze")&&!c->input&&!c->captures_dir) return sn_fail("analyze needs --input or --captures-dir");
 if(!strcmp(c->command,"capture")&&(!isfinite(c->frequency)||c->frequency<1e6||c->frequency>6e9)) return sn_fail("capture needs --freq-mhz in 1..6000");
 return 0;
}
static int manifest(const char *dir,const char *state,const char *phase,const char *detail,const SnDb *db,const SnStats *s) {
 char final[SN_PATH],temporary[SN_PATH];
 if(sn_path(final,sizeof(final),"%s/run.json",dir)||sn_path(temporary,sizeof(temporary),"%s.tmp",final)) return -1;
 FILE *f=fopen(temporary,"w"); if(!f) return sn_fail("create manifest: %s",strerror(errno));
 fprintf(f,"{\"version\":\"%s\",\"runtime\":\"native_c\",\"session_id\":\"%s\",\"state\":",SN_VERSION,db->session); sn_json(f,state);
 fprintf(f,",\"phase\":"); sn_json(f,phase); fprintf(f,",\"detail\":"); sn_json(f,detail);
 fprintf(f,",\"rows\":%zu,\"input_errors\":%zu,\"sweeps\":%zu,\"events\":%zu,\"captures_analyzed\":%zu,\"capture_errors\":%zu}\n",s->rows,s->errors,s->sweeps,s->events,s->captures,s->capture_errors);
 return sn_atomic(f,temporary,final);
}
static int lock_receiver(void) {
 char path[128]; snprintf(path,sizeof(path),"/tmp/sentinel-rf-%lu.lock",(unsigned long)getuid());
 int flags=O_RDWR|O_CREAT|O_CLOEXEC;
#ifdef O_NOFOLLOW
 flags|=O_NOFOLLOW;
#endif
 int fd=open(path,flags,0600); if(fd<0) return sn_fail("open receiver lock: %s",strerror(errno));
 if(flock(fd,LOCK_EX|LOCK_NB)) { close(fd); return sn_fail("another Sentinel receiver operation is running"); } return fd;
}
int main(int argc,char **argv) {
 setlocale(LC_NUMERIC,"C"); struct sigaction action={0}; action.sa_handler=interrupted; sigemptyset(&action.sa_mask);
 sigaction(SIGINT,&action,NULL); sigaction(SIGTERM,&action,NULL); signal(SIGPIPE,SIG_IGN);
 SnConfig c; int result=parse(argc,argv,&c);
 if(result>0) return 0;
 if(result<0) { fprintf(stderr,"Sentinel: %s\n",sn_error); return 2; }
 if(!strcmp(c.command,"doctor")) { printf("{\"version\":\"%s\",\"runtime\":\"native_c\",\"backend\":\"%s\",\"sqlite\":\"%s\",\"hardware_tested\":false}\n",SN_VERSION,sn_receiver_backend(),sqlite3_libversion()); return fflush(stdout)?1:0; }
 SnDb db={0}; SnStats stats={0}; int error=0,lock=-1; FILE *events=NULL,*analysis=NULL;
 char database[SN_PATH],directory[SN_PATH]="",path[SN_PATH],temporary[SN_PATH],detail[512]="";
 const char *phase="startup",*state="failed";
 if(sn_path(database,sizeof(database),"%s/sentinel.sqlite",c.workdir)) goto failed;
 if(!strcmp(c.command,"report")) { if(sn_db_open(&db,database,1)||sn_db_report(&db,stdout,0)) { sn_db_close(&db); goto failed; } sn_db_close(&db); return fflush(stdout)?1:0; }
 if(sn_mkdirs(c.workdir)||sn_db_open(&db,database,0)||sn_db_start(&db,&c)) goto failed;
 if(sn_path(directory,sizeof(directory),"%s/runs/%s",c.workdir,db.session)||sn_mkdirs(directory)||manifest(directory,"running",phase,"",&db,&stats)) goto failed;
 if((!c.replay&&(!strcmp(c.command,"run")||!strcmp(c.command,"scan")))||!strcmp(c.command,"capture")) { lock=lock_receiver(); if(lock<0) goto failed; }
 if(!strcmp(c.command,"run")||!strcmp(c.command,"scan")) {
  phase="scan"; if(sn_path(path,sizeof(path),"%s/events.jsonl",directory)) goto failed;
  events=fopen(path,"w"); if(!events) { sn_fail("create event log: %s",strerror(errno)); goto failed; } db.events=events;
  error=c.replay?sn_replay(&db,&c,&stats):sn_live(&db,&c,&stats);
  if(error) goto failed;
  if(sn_cancelled) goto cancelled;
  if(manifest(directory,"running","scan_complete","",&db,&stats)) goto failed;
 }
 if(c.capture||!strcmp(c.command,"capture")||c.captures_dir||!strcmp(c.command,"analyze")) {
  phase="analysis"; if(sn_path(path,sizeof(path),"%s/analysis.jsonl",directory)) goto failed;
  analysis=fopen(path,"w"); if(!analysis) { sn_fail("create analysis log: %s",strerror(errno)); goto failed; }
  if(c.capture||!strcmp(c.command,"capture")) {
   phase="capture"; double centers[1000]; size_t count=0;
   if(!strcmp(c.command,"capture")) { centers[0]=c.frequency; count=1; } else if(sn_db_targets(&db,centers,c.max_targets,&count)) goto failed;
   SnReceiver *receiver=NULL; if(count&&sn_receiver_open(&receiver,&c)) goto failed;
   for(size_t i=0;i<count&&!sn_cancelled;++i) {
    SnFeatures f; char output[SN_PATH],captures[SN_PATH];
    if(sn_path(captures,sizeof(captures),"%s/captures",directory)) { error=-1; break; }
    if(sn_capture(receiver,&c,centers[i],captures,&f,output,sizeof(output))) { ++stats.capture_errors; fprintf(analysis,"{\"type\":\"capture_error\",\"center_hz\":%.9f,\"error\":",centers[i]); sn_json(analysis,sn_error); fprintf(analysis,"}\n"); }
    else { ++stats.captures; if(sn_write_features(analysis,output,c.sample_rate,centers[i],&f)) { error=-1; break; } }
   }
   char original[512]; memcpy(original,sn_error,sizeof(original)); if(sn_receiver_close(receiver)&&!error) error=-1; else if(error) memcpy(sn_error,original,sizeof(sn_error)); if(error) goto failed;
  }
  phase="analysis";
  if(c.captures_dir&&sn_analyze_directory(c.captures_dir,analysis,&stats)) goto failed;
  if(c.input) { SnFeatures f; if(sn_analyze_file(c.input,&c.sample_rate,&c.frequency,&f)||sn_write_features(analysis,c.input,c.sample_rate,c.frequency,&f)) goto failed; ++stats.captures; }
 }
 if(sn_cancelled) goto cancelled;
 phase="report";
 if(sn_path(path,sizeof(path),"%s/spectrum_report.csv",directory)||sn_path(temporary,sizeof(temporary),"%s.tmp",path)) goto failed;
 FILE *report=fopen(temporary,"w"); if(!report) { sn_fail("create report: %s",strerror(errno)); goto failed; }
 if(sn_db_report(&db,report,1)) { fclose(report); unlink(temporary); goto failed; }
 if(sn_atomic(report,temporary,path)) goto failed;
 state=stats.errors||stats.capture_errors?"partial":"complete"; result=stats.errors||stats.capture_errors?2:0; phase="finished"; goto done;
cancelled: state="interrupted"; result=130; goto done;
failed: snprintf(detail,sizeof(detail),"%s",sn_error); state=sn_cancelled?"interrupted":"failed"; result=sn_cancelled?130:1;
done:
 if(events&&fclose(events)) { snprintf(detail,sizeof(detail),"event log close failed"); state="failed"; result=1; }
 if(analysis&&fclose(analysis)) { snprintf(detail,sizeof(detail),"analysis log close failed"); state="failed"; result=1; }
 if(db.handle&&*db.session&&sn_db_finish(&db,state,detail)) { snprintf(detail,sizeof(detail),"%s",sn_error); state="failed"; result=1; }
 if(*directory&&manifest(directory,state,phase,detail,&db,&stats)) { fprintf(stderr,"Sentinel: final manifest unavailable: %s\n",sn_error); result=1; }
 sn_db_close(&db); if(lock>=0) { flock(lock,LOCK_UN); close(lock); }
 if(*detail) fprintf(stderr,"Sentinel: %s\n",detail);
 if(*directory) fprintf(stderr,"%s: %s/run.json\n",state,directory);
 return result;
}
