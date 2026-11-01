#include "sentinel.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static size_t events;
static SnEvent last;
static int collect(const SnEvent *e,void *context) { (void)context; last=*e; ++events; return 0; }
static void parser_tests(void) {
 SnRow row;
 assert(!sn_parse_row("2019-01-03,11:57:34.967805,2400000000,2405000000,1000000.00,20,-64.72,-63.36,-60.91,-61.74,-58.58",&row));
 assert(row.count==5&&row.width==1000000&&!strcmp(row.timestamp,"2019-01-03T11:57:34.967805")); sn_row_free(&row);
 assert(!sn_parse_row("2024-02-29,00:00:00,1,2,1,1,-90",&row)); sn_row_free(&row);
 const char *bad[]={"bad","2026-02-30,12:00:00,1,2,1,20,-90","2026-10-30,25:00:00,1,2,1,20,-90",
  "2026-10-30,12:00:00,1,3,1,20,-90","2026-10-30,12:00:00,1,2,1,20,nan","2026-10-30,12:00:00,1,2,1,20,",
  "2026-10-30,12:00:00,1,2,1,20,-90,","2026-10-30,12:00:00,1,2,1,20,inf","2026-10-30,12:00:00,1,2,0,20,-90",
  "2026-10-30,12:00:00,1,2,1,20,-90junk","2026-10-30,12:00:00,1,2,1,,20,-90"};
 for(size_t i=0;i<sizeof(bad)/sizeof(*bad);++i) assert(sn_parse_row(bad[i],&row)<0);
 unsigned seed=7;
 for(size_t i=0;i<2000;++i) {
  char junk[96]; for(size_t j=0;j<sizeof(junk)-1;++j) { seed=1664525u*seed+1013904223u; junk[j]="0123456789,-:. abcdef"[seed%19]; } junk[sizeof(junk)-1]=0;
  if(!sn_parse_row(junk,&row)) sn_row_free(&row);
 }
}
static void detector_tests(void) {
 double power[64]; for(size_t i=0;i<64;++i) power[i]=-90; power[32]=-5;
 SnRow row={.low=1000000,.high=1640000,.width=10000,.samples=20,.power=power,.count=64}; SnSweep sweep={0};
 assert(!sn_sweep_append(&sweep,&row,1000000,2000000)); assert(!sn_sweep_append(&sweep,&row,1000000,2000000));
 events=0; assert(!sn_detect(&sweep,10,collect,NULL)); assert(events==1&&last.center==1325000&&last.bandwidth==10000&&last.peak==-5); sn_sweep_free(&sweep);
 power[33]=-20; assert(!sn_sweep_append(&sweep,&row,1000000,2000000)); events=0; assert(!sn_detect(&sweep,10,collect,NULL)); assert(events==1&&last.bandwidth==20000); sn_sweep_free(&sweep);
 power[32]=power[33]=-90; assert(!sn_sweep_append(&sweep,&row,1000000,2000000)); events=0; assert(!sn_detect(&sweep,10,collect,NULL)); assert(events==0); sn_sweep_free(&sweep);
 row.low=10000000; power[32]=-30; assert(!sn_sweep_append(&sweep,&row,0,20000000)); row.low=1000000; assert(!sn_sweep_append(&sweep,&row,0,20000000));
 events=0; assert(!sn_detect(&sweep,10,collect,NULL)); assert(events==2); sn_sweep_free(&sweep);
}
static void feature_tests(void) {
 double p[]={1,30,38,30,1}; assert(sn_obw(p,5,1,.95)==3);
 double two[]={0,1,0,0,0,1,0}; assert(sn_obw(two,7,10000,.99)==50000&&sn_obw(two,7,10000,1)==50000);
 double zero[64]={0}; assert(sn_obw(zero,64,10,.99)==0); p[0]=-1; assert(isnan(sn_obw(p,5,1,.99)));
 size_t n=65536; int8_t *raw=calloc(n,2); assert(raw); SnFeatures f;
 assert(!sn_features(raw,n,1048576,&f)&&f.zero_energy&&f.obw==0);
 for(size_t i=0;i<n;++i) { double phase=2*3.141592653589793*100000*(double)i/1048576; raw[2*i]=(int8_t)llround(70*cos(phase)); raw[2*i+1]=(int8_t)llround(70*sin(phase)); }
 assert(!sn_features(raw,n,1048576,&f)&&f.obw>0&&f.obw<3000);
 double sum=0; for(size_t i=0;i<32;++i) sum+=f.shape[i]; assert(fabs(sum-1)<1e-9);
 for(size_t i=0;i<n;++i) { double phase=2*3.141592653589793*100000*(double)i/1048576; raw[2*i]=(int8_t)llround(90*cos(phase)); raw[2*i+1]=0; }
 assert(!sn_features(raw,n,1048576,&f)&&fabs(f.obw-200000)<3000); assert(sn_features(raw,100,1048576,&f)<0); free(raw);
}
static void database_tests(void) {
 SnDb db; SnConfig c={.replay="fixture",.start=1000000,.stop=2000000,.bin_hz=10000,.margin=10}; assert(!sn_db_open(&db,":memory:",0)); assert(!sn_db_start(&db,&c));
 SnEvent e={.low=1320000,.high=1330000,.center=1325000,.bandwidth=10000,.peak=-5,.noise=-90,.contrast=85,.bin_width=10000};
 FILE *log=tmpfile(); assert(log); db.events=log;
 assert(!sn_db_event(&db,"2026-10-30T12:00:00.000000",&e)); assert(!sn_db_event(&db,"2026-10-30T12:00:01.000000",&e)); assert(!sn_db_commit(&db));
 sqlite3_stmt *s; assert(sqlite3_prepare_v2(db.handle,"SELECT count(*),max(observed_sweeps) FROM signal_groups",-1,&s,NULL)==SQLITE_OK);
 assert(sqlite3_step(s)==SQLITE_ROW&&sqlite3_column_int(s,0)==1&&sqlite3_column_int(s,1)==2); sqlite3_finalize(s);
 double centers[3]; size_t count; assert(!sn_db_targets(&db,centers,3,&count)&&count==1&&centers[0]==1325000);
 assert(!sn_db_finish(&db,"complete","")); assert(!sn_db_start(&db,&c)); assert(!sn_db_targets(&db,centers,3,&count)&&count==0);
 sn_db_close(&db); fclose(log);
}
static int fixture(const char *directory) {
 char path[SN_PATH]; assert(!sn_mkdirs(directory)); assert(!sn_path(path,sizeof(path),"%s/sweep.csv",directory)); FILE *f=fopen(path,"w"); assert(f);
 for(unsigned t=0;t<2;++t) { fprintf(f,"2026-10-30,12:00:0%u,1000000,1640000,10000,20",t); for(unsigned i=0;i<64;++i) fprintf(f,",%d",i==32?-25:-90); fputc('\n',f); } assert(!fclose(f));
 assert(!sn_path(path,sizeof(path),"%s/tone.cs8",directory)); f=fopen(path,"wb"); assert(f);
 for(size_t i=0;i<16384;++i) { int8_t pair[]={(int8_t)llround(60*cos(2*3.141592653589793*.1*(double)i)),(int8_t)llround(60*sin(2*3.141592653589793*.1*(double)i))}; assert(fwrite(pair,2,1,f)==1); } assert(!fclose(f));
 assert(!sn_path(path,sizeof(path),"%s/tone.meta",directory)); f=fopen(path,"w"); assert(f); fprintf(f,"freq_mhz=915\nsample_rate=2000000\nsamples=16384\nstatus=complete\n"); assert(!fclose(f)); return 0;
}
int main(int argc,char **argv) {
 if(argc==3&&!strcmp(argv[1],"--fixture")) return fixture(argv[2]);
 parser_tests(); detector_tests(); feature_tests(); database_tests(); puts("native core checks passed (including 2000 malformed-input cases)"); return 0;
}
