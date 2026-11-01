#include "sentinel.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#define REPEATS 7
static int order(const void *a,const void *b) { double x=*(const double *)a,y=*(const double *)b; return (x>y)-(x<y); }
static int count_event(const SnEvent *event,void *context) { (void)event; ++*(size_t *)context; return 0; }
static int metric(const char *name,const char *unit,double *times,size_t operations) {
 double total=0; for(size_t i=0;i<REPEATS;++i) { if(times[i]<=0) return sn_fail("nonpositive benchmark duration"); total+=times[i]; }
 qsort(times,REPEATS,sizeof(*times),order);
 /* Quantiles describe seven batch durations, not individual operation latency. */
 printf("{\"type\":\"benchmark\",\"name\":\"%s\",\"unit\":\"%s\",\"repeats\":%d,\"operations_per_batch\":%zu,\"batch_p50_ms\":%.6f,\"batch_p95_ms\":%.6f,\"total_seconds\":%.6f,\"operations_per_second\":%.3f}\n",name,unit,REPEATS,operations,1000*times[REPEATS/2],1000*times[REPEATS-1],total,(double)operations*REPEATS/total);
 return ferror(stdout)?sn_fail("benchmark output failed"):0;
}
static int benches(unsigned iterations) {
 double times[REPEATS];
 const char *csv="2026-10-30,12:00:00,1000000,1640000,10000,20,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-25,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90";
 for(size_t r=0;r<REPEATS;++r) {
  double start=sn_clock();
  for(unsigned i=0;i<iterations*100;++i) { SnRow row; if(sn_parse_row(csv,&row)) return -1; if(row.count!=64) { sn_row_free(&row); return sn_fail("parser benchmark mismatch"); } sn_row_free(&row); }
  times[r]=sn_clock()-start;
 }
 if(metric("parse_64_bins","rows",times,iterations*100)) return -1;
 SnBin bins[4096];
 for(size_t r=0;r<REPEATS;++r) {
  double start=sn_clock();
  for(unsigned i=0;i<iterations;++i) {
   for(size_t j=0;j<4096;++j) bins[j]=(SnBin){1000000+(double)j*10000,10000,j==2048?-25:-90};
   SnSweep sweep={.bins=bins,.count=4096,.capacity=4096}; size_t count=0;
   if(sn_detect(&sweep,10,count_event,&count)) return -1;
   if(count!=1) return sn_fail("detector benchmark mismatch");
  }
  times[r]=sn_clock()-start;
 }
 if(metric("detect_4096_bins","sweeps",times,iterations)) return -1;
 size_t samples=65536; int8_t *raw=malloc(samples*2); if(!raw) return sn_fail("benchmark IQ allocation failed");
 for(size_t i=0;i<samples;++i) { double phase=2*3.141592653589793*.125*(double)i; raw[2*i]=(int8_t)llround(70*cos(phase)); raw[2*i+1]=(int8_t)llround(70*sin(phase)); }
 for(size_t r=0;r<REPEATS;++r) {
  double start=sn_clock();
  for(unsigned i=0;i<iterations;++i) { SnFeatures f; if(sn_features(raw,samples,2000000,&f)) { free(raw); return -1; } if(f.zero_energy||f.obw<=0||f.obw>3000) { free(raw); return sn_fail("DSP benchmark mismatch"); } }
  times[r]=sn_clock()-start;
 }
 free(raw);
 if(metric("describe_65536_pairs","captures",times,iterations)) return -1;
 SnDb db={0}; SnConfig config={.replay="benchmark",.start=1000000,.stop=2000000,.bin_hz=10000,.margin=10};
 if(sn_db_open(&db,":memory:",0)) return -1;
 FILE *sink=tmpfile(); if(!sink) { sn_db_close(&db); return sn_fail("benchmark output allocation failed"); } db.events=sink;
 SnEvent event={.low=1320000,.high=1330000,.center=1325000,.bandwidth=10000,.peak=-25,.noise=-90,.contrast=65,.bin_width=10000};
 int error=0;
 for(size_t r=0;r<REPEATS&&!error;++r) {
  if(sn_db_start(&db,&config)) { error=-1; break; }
  double start=sn_clock();
  for(unsigned i=0;i<iterations*10;++i) {
   char timestamp[40]; snprintf(timestamp,sizeof(timestamp),"2026-10-30T12:%02u:%02u.%06u",i/60%60,i%60,i/3600);
   if(sn_db_event(&db,timestamp,&event)) { error=-1; break; }
  }
  if(!error&&sn_db_commit(&db)) error=-1;
  times[r]=sn_clock()-start;
  if(!error&&sn_db_finish(&db,"complete","benchmark")) error=-1;
 }
 sn_db_close(&db); if(fclose(sink)&&!error) error=sn_fail("benchmark sink close failed");
 if(error) return -1;
 return metric("sqlite_recurrence_memory","observations",times,iterations*10);
}
int main(int argc,char **argv) {
 unsigned iterations=20;
 if(argc==3&&!strcmp(argv[1],"--iterations")) { char *end; unsigned long n=strtoul(argv[2],&end,10); if(*end||n<1||n>10000) return 2; iterations=(unsigned)n; }
 else if(argc!=1) return 2;
 sn_log_init(SN_LOG_ERROR);
 struct utsname host; if(uname(&host)) return 1;
 printf("{\"type\":\"environment\",\"version\":\"%s\",\"compiler\":",SN_VERSION); sn_json(stdout,__VERSION__);
 printf(",\"system\":"); sn_json(stdout,host.sysname); printf(",\"machine\":"); sn_json(stdout,host.machine);
 printf(",\"sqlite\":\"%s\",\"hardware_measured\":false,\"iterations\":%u,\"timing_scope\":\"in_process_batches_including_validation\"}\n",sqlite3_libversion(),iterations);
 if(benches(iterations)) { fprintf(stderr,"benchmark: %s\n",sn_error); return 1; }
 struct rusage usage; if(getrusage(RUSAGE_SELF,&usage)) return 1;
 printf("{\"type\":\"resources\",\"max_rss_kib_linux\":%ld,\"user_cpu_seconds\":%.6f,\"system_cpu_seconds\":%.6f}\n",usage.ru_maxrss,(double)usage.ru_utime.tv_sec+(double)usage.ru_utime.tv_usec/1e6,(double)usage.ru_stime.tv_sec+(double)usage.ru_stime.tv_usec/1e6);
 return fflush(stdout)||sn_log_close()?1:0;
}
