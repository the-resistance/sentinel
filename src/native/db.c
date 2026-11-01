#include "sentinel.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#define SN_APPLICATION_ID 1397642316
static int query(SnDb *db,const char *format,...) {
 va_list args; va_start(args,format); char *sql=sqlite3_vmprintf(format,args); va_end(args);
 if(!sql) return sn_fail("SQL allocation failed");
 int result=sqlite3_exec(db->handle,sql,NULL,NULL,NULL); sqlite3_free(sql);
 return result==SQLITE_OK?0:sn_fail("SQLite: %s",sqlite3_errmsg(db->handle));
}
static int scalar(SnDb *db,const char *sql,sqlite3_int64 *out) {
 sqlite3_stmt *s=NULL; if(sqlite3_prepare_v2(db->handle,sql,-1,&s,NULL)!=SQLITE_OK) return sn_fail("SQLite: %s",sqlite3_errmsg(db->handle));
 int result=sqlite3_step(s); if(result==SQLITE_ROW) *out=sqlite3_column_int64(s,0); sqlite3_finalize(s);
 return result==SQLITE_ROW?0:sn_fail("SQLite scalar query failed");
}
int sn_db_open(SnDb *db,const char *path,int readonly) {
 sn_log(SN_LOG_DEBUG,"database","open path=%s readonly=%d",path,readonly);
 memset(db,0,sizeof(*db));
 if(sqlite3_open_v2(path,&db->handle,readonly?SQLITE_OPEN_READONLY:SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE,NULL)!=SQLITE_OK) { sn_fail("open database: %s",db->handle?sqlite3_errmsg(db->handle):"allocation failure"); sn_db_close(db); return -1; }
 sqlite3_busy_timeout(db->handle,1000); sqlite3_int64 application=0,version=0,tables=0;
 if(scalar(db,"PRAGMA application_id",&application)||scalar(db,"PRAGMA user_version",&version)||scalar(db,"SELECT count(*) FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%'",&tables)) goto fail;
 if(application!=SN_APPLICATION_ID&&(readonly||tables||application)) { sn_fail("not a native Sentinel database; preserve old databases separately"); goto fail; }
 if(version>1) { sn_fail("database schema is newer than this runtime"); goto fail; }
 if(readonly) return 0;
 if(query(db,"PRAGMA journal_mode=WAL; PRAGMA foreign_keys=ON;"
 "CREATE TABLE IF NOT EXISTS sessions(id TEXT PRIMARY KEY,created_utc TEXT NOT NULL,state TEXT NOT NULL,detail TEXT NOT NULL,clock_basis TEXT NOT NULL,config TEXT NOT NULL);"
 "CREATE TABLE IF NOT EXISTS raw_rows(session_id TEXT NOT NULL REFERENCES sessions(id),timestamp TEXT NOT NULL,low_hz REAL NOT NULL,high_hz REAL NOT NULL,bin_hz REAL NOT NULL,raw_csv TEXT NOT NULL,UNIQUE(session_id,timestamp,low_hz,high_hz,bin_hz));"
 "CREATE TABLE IF NOT EXISTS input_errors(session_id TEXT NOT NULL REFERENCES sessions(id),line INTEGER NOT NULL,reason TEXT NOT NULL,raw_text TEXT NOT NULL);"
 "CREATE TABLE IF NOT EXISTS signal_groups(id INTEGER PRIMARY KEY,center_hz REAL NOT NULL,bandwidth_hz REAL NOT NULL,bin_hz REAL NOT NULL,first_seen TEXT NOT NULL,last_seen TEXT NOT NULL,observed_sweeps INTEGER NOT NULL DEFAULT 0,peak_db REAL NOT NULL,clock_basis TEXT NOT NULL);"
 "CREATE INDEX IF NOT EXISTS group_frequency ON signal_groups(center_hz);"
 "CREATE TABLE IF NOT EXISTS observations(session_id TEXT NOT NULL REFERENCES sessions(id),timestamp TEXT NOT NULL,group_id INTEGER NOT NULL REFERENCES signal_groups(id),low_hz REAL NOT NULL,high_hz REAL NOT NULL,peak_db REAL NOT NULL,noise_db REAL NOT NULL,contrast_db REAL NOT NULL,UNIQUE(session_id,timestamp,group_id));"
 "CREATE TABLE IF NOT EXISTS sweep_reports(session_id TEXT NOT NULL REFERENCES sessions(id),timestamp TEXT NOT NULL,coverage_hz REAL NOT NULL,events INTEGER NOT NULL,UNIQUE(session_id,timestamp));"
 "PRAGMA application_id=%d; PRAGMA user_version=1;",SN_APPLICATION_ID)) goto fail;
 return 0;
fail: sn_db_close(db); return -1;
}
int sn_db_start(SnDb *db,const SnConfig *c) {
 char timestamp[40]; if(sn_id(db->session)||sn_utc(timestamp)) return -1;
 db->requested_span=c->stop-c->start;
 char *settings=NULL; size_t size=0; FILE *f=open_memstream(&settings,&size); if(!f) return sn_fail("configuration allocation failed");
 fprintf(f,"{\"start_hz\":%.0f,\"stop_hz\":%.0f,\"requested_bin_hz\":%.9g,\"margin_db\":%.9g,\"lna\":%u,\"vga\":%u,\"sample_rate_hz\":%.9g,\"version\":\"%s\",\"power_units\":\"uncalibrated_relative_dB\",\"receiver_serial_request\":",c->start,c->stop,c->bin_hz,c->margin,c->lna,c->vga,c->sample_rate,SN_VERSION);
 sn_json(f,c->serial?c->serial:"default_receiver"); fprintf(f,"}"); int error=ferror(f); if(fclose(f)) error=1;
 if(error) { free(settings); return sn_fail("configuration write failed"); }
 error=query(db,"INSERT INTO sessions VALUES(%Q,%Q,'running','',%Q,%Q)",db->session,timestamp,c->replay?"receiver_local_unspecified":"UTC",settings); free(settings); return error;
}
static int transaction(SnDb *db) { if(db->transaction) return 0; if(query(db,"BEGIN IMMEDIATE")) return -1; db->transaction=1; return 0; }
int sn_db_raw(SnDb *db,const SnRow *r,const char *raw) {
 if(transaction(db)||query(db,"INSERT OR IGNORE INTO raw_rows VALUES(%Q,%Q,%!.17g,%!.17g,%!.17g,%Q)",db->session,r->timestamp,r->low,r->high,r->width,raw)) return -1;
 return sqlite3_changes(db->handle)>0;
}
int sn_db_bad(SnDb *db,size_t line,const char *raw,const char *reason) { return transaction(db)||query(db,"INSERT INTO input_errors VALUES(%Q,%lld,%Q,%Q)",db->session,(sqlite3_int64)line,reason,raw)?-1:0; }
int sn_db_event(SnDb *db,const char *timestamp,const SnEvent *e) {
 if(transaction(db)) return -1;
 char *sql=sqlite3_mprintf("SELECT id FROM signal_groups WHERE center_hz BETWEEN %!.17g AND %!.17g AND abs(bin_hz-%!.17g)<1 AND max(bandwidth_hz,%!.17g)/min(bandwidth_hz,%!.17g)<=1.5 AND clock_basis=(SELECT clock_basis FROM sessions WHERE id=%Q) AND id NOT IN(SELECT group_id FROM observations WHERE session_id=%Q AND timestamp=%Q)",e->center-e->bin_width,e->center+e->bin_width,e->bin_width,e->bandwidth,e->bandwidth,db->session,db->session,timestamp);
 if(!sql) return sn_fail("SQL allocation failed");
 sqlite3_stmt *s=NULL; int result=sqlite3_prepare_v2(db->handle,sql,-1,&s,NULL); sqlite3_free(sql);
 if(result!=SQLITE_OK) return sn_fail("SQLite group query: %s",sqlite3_errmsg(db->handle));
 size_t matches=0; sqlite3_int64 group=0;
 while((result=sqlite3_step(s))==SQLITE_ROW) { group=sqlite3_column_int64(s,0); ++matches; } sqlite3_finalize(s);
 if(result!=SQLITE_DONE) return sn_fail("SQLite group read failed");
 if(matches!=1) {
  if(query(db,"INSERT INTO signal_groups(center_hz,bandwidth_hz,bin_hz,first_seen,last_seen,peak_db,clock_basis) VALUES(%!.17g,%!.17g,%!.17g,%Q,%Q,%!.17g,(SELECT clock_basis FROM sessions WHERE id=%Q))",e->center,e->bandwidth,e->bin_width,timestamp,timestamp,e->peak,db->session)) return -1;
  group=sqlite3_last_insert_rowid(db->handle);
 }
 if(query(db,"INSERT INTO observations VALUES(%Q,%Q,%lld,%!.17g,%!.17g,%!.17g,%!.17g,%!.17g)",db->session,timestamp,group,e->low,e->high,e->peak,e->noise,e->contrast)||query(db,"UPDATE signal_groups SET observed_sweeps=observed_sweeps+1,first_seen=min(first_seen,%Q),last_seen=max(last_seen,%Q),peak_db=max(peak_db,%!.17g) WHERE id=%lld",timestamp,timestamp,e->peak,group)) return -1;
 sn_log(SN_LOG_DEBUG,"detection","group=%lld timestamp=%s center_hz=%.0f bandwidth_hz=%.0f contrast_db=%.3f matches=%zu",(long long)group,timestamp,e->center,e->bandwidth,e->contrast,matches);
 FILE *f=db->events?db->events:stdout;
 fprintf(f,"{\"type\":\"spectrum_observation\",\"group_id\":%lld,\"timestamp\":\"%s\",\"center_hz\":%.9f,\"bandwidth_hz\":%.9f,\"peak_db\":%.9f,\"noise_db\":%.9f,\"contrast_db\":%.9f,\"device_identity\":null,\"association\":\"frequency_bandwidth_only\"}\n",(long long)group,timestamp,e->center,e->bandwidth,e->peak,e->noise,e->contrast);
 return ferror(f)?sn_fail("event output failed"):0;
}
int sn_db_sweep(SnDb *db,const SnSweep *s,double coverage,size_t events) {
 if(transaction(db)||query(db,"INSERT INTO sweep_reports VALUES(%Q,%Q,%!.17g,%lld)",db->session,s->timestamp,coverage,(sqlite3_int64)events)) return -1;
 FILE *f=db->events?db->events:stdout;
 fprintf(f,"{\"type\":\"sweep_summary\",\"timestamp\":\"%s\",\"observed_span_hz\":%.9f,\"requested_span_hz\":%.9f,\"coverage_fraction\":%.9f,\"candidate_events\":%zu}\n",s->timestamp,coverage,db->requested_span,db->requested_span>0?coverage/db->requested_span:0,events);
 return ferror(f)?sn_fail("summary output failed"):0;
}
int sn_db_commit(SnDb *db) { if(!db->transaction) return 0; if(query(db,"COMMIT")) return -1; db->transaction=0; return 0; }
int sn_db_finish(SnDb *db,const char *state,const char *detail) { if(sn_db_commit(db)) return -1; return query(db,"UPDATE sessions SET state=%Q,detail=%Q WHERE id=%Q",state,detail,db->session); }
int sn_db_report(SnDb *db,FILE *out,int current) {
 char *sql=current?sqlite3_mprintf("SELECT g.id,g.center_hz,g.bandwidth_hz,g.bin_hz,min(o.timestamp),max(o.timestamp),count(*),max(o.peak_db),g.clock_basis FROM signal_groups g JOIN observations o ON g.id=o.group_id WHERE o.session_id=%Q GROUP BY g.id ORDER BY count(*) DESC,g.id",db->session):sqlite3_mprintf("SELECT id,center_hz,bandwidth_hz,bin_hz,first_seen,last_seen,observed_sweeps,peak_db,clock_basis FROM signal_groups ORDER BY observed_sweeps DESC,id");
 if(!sql) return sn_fail("SQL allocation failed");
 sqlite3_stmt *s=NULL;
 int result=sqlite3_prepare_v2(db->handle,sql,-1,&s,NULL); sqlite3_free(sql); if(result!=SQLITE_OK) return sn_fail("SQLite report: %s",sqlite3_errmsg(db->handle));
 fprintf(out,"group_id,center_hz,bandwidth_hz,bin_width_hz,first_seen,last_seen,observed_sweeps,peak_db_uncalibrated,clock_basis\n");
 while((result=sqlite3_step(s))==SQLITE_ROW) fprintf(out,"%lld,%.9f,%.9f,%.9f,%s,%s,%lld,%.9f,%s\n",(long long)sqlite3_column_int64(s,0),sqlite3_column_double(s,1),sqlite3_column_double(s,2),sqlite3_column_double(s,3),sqlite3_column_text(s,4),sqlite3_column_text(s,5),(long long)sqlite3_column_int64(s,6),sqlite3_column_double(s,7),sqlite3_column_text(s,8));
 sqlite3_finalize(s); return result!=SQLITE_DONE||ferror(out)?sn_fail("report read/write failed"):0;
}
int sn_db_targets(SnDb *db,double *centers,size_t maximum,size_t *count) {
 *count=0; char *sql=sqlite3_mprintf("SELECT g.center_hz FROM signal_groups g JOIN observations o ON g.id=o.group_id WHERE o.session_id=%Q GROUP BY g.id ORDER BY count(*) DESC,max(o.contrast_db) DESC,g.id LIMIT %lld",db->session,(sqlite3_int64)maximum);
 if(!sql) return sn_fail("SQL allocation failed");
 sqlite3_stmt *s=NULL;
 int result=sqlite3_prepare_v2(db->handle,sql,-1,&s,NULL); sqlite3_free(sql); if(result!=SQLITE_OK) return sn_fail("SQLite target query failed");
 while((result=sqlite3_step(s))==SQLITE_ROW) centers[(*count)++]=sqlite3_column_double(s,0);
 sqlite3_finalize(s); return result==SQLITE_DONE?0:sn_fail("SQLite target read failed");
}
void sn_db_close(SnDb *db) { if(db->handle) { if(db->transaction) sqlite3_exec(db->handle,"ROLLBACK",NULL,NULL,NULL); sqlite3_close(db->handle); } db->handle=NULL; db->transaction=0; }
