#include "sentinel.h"
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
static char *field(char **cursor) {
 if(!*cursor) return NULL;
 char *start=*cursor,*comma=strchr(start,',');
 if(comma) { *comma=0; *cursor=comma+1; } else *cursor=NULL;
 while(isspace((unsigned char)*start)) ++start;
 char *end=start+strlen(start); while(end>start&&isspace((unsigned char)end[-1])) *--end=0;
 return start;
}
static int number(const char *text,double *out) {
 if(!text||!*text) return -1;
 char *end; errno=0; *out=strtod(text,&end); return errno||*end||!isfinite(*out)?-1:0;
}
static int stamp(char out[40],const char *date,const char *time) {
 int y,m,d,h,minute,second,offset=0;
 if(!date||strlen(date)!=10||sscanf(date,"%4d-%2d-%2d%n",&y,&m,&d,&offset)!=3||offset!=10) return -1;
 static const int days[]={0,31,28,31,30,31,30,31,31,30,31,30,31};
 if(y<1970||y>9999||m<1||m>12) return -1;
 int leap=y%4==0&&(y%100!=0||y%400==0);
 if(d<1||d>days[m]+(m==2&&leap)) return -1;
 if(!time||sscanf(time,"%2d:%2d:%2d%n",&h,&minute,&second,&offset)!=3||offset!=8||h<0||h>23||minute<0||minute>59||second<0||second>59) return -1;
 const char *p=time+offset; unsigned fraction=0,digits=0;
 if(*p=='.') { ++p; while(isdigit((unsigned char)*p)&&digits<6) { fraction=10*fraction+(unsigned)(*p++-'0'); ++digits; } if(!digits) return -1; }
 if(*p) return -1;
 while(digits++<6) fraction*=10;
 snprintf(out,40,"%04d-%02d-%02dT%02d:%02d:%02d.%06u",y,m,d,h,minute,second,fraction); return 0;
}
int sn_parse_row(const char *line,SnRow *row) {
 memset(row,0,sizeof(*row)); char *copy=strdup(line); if(!copy) return sn_fail("CSV allocation failed");
 char *cursor=copy,*date=field(&cursor),*time=field(&cursor); double samples=0;
 int bad=stamp(row->timestamp,date,time)||number(field(&cursor),&row->low)||number(field(&cursor),&row->high)||number(field(&cursor),&row->width)||number(field(&cursor),&samples);
 if(bad||row->low<0||row->high<=row->low||row->high>7e9||row->width<=0||samples<1||samples>4294967295.0||floor(samples)!=samples) { free(copy); return sn_fail("invalid timestamp or sweep geometry"); }
 row->samples=(unsigned)samples; size_t capacity=0;
 while(cursor) {
  if(row->count==capacity) {
   capacity=capacity?capacity*2:64; if(capacity>SN_MAX_BINS) { bad=1; break; }
   double *next=realloc(row->power,capacity*sizeof(*next)); if(!next) { bad=1; break; } row->power=next;
  }
  if(number(field(&cursor),&row->power[row->count])) { bad=1; break; } ++row->count;
 }
 free(copy);
 if(bad||!row->count||fabs((double)row->count*row->width-(row->high-row->low))>fmax(.1,(double)row->count*.02)) { sn_row_free(row); return sn_fail("invalid power bins or inconsistent row span"); }
 return 0;
}
void sn_row_free(SnRow *row) { free(row->power); row->power=NULL; row->count=0; }
void sn_sweep_free(SnSweep *sweep) { free(sweep->bins); memset(sweep,0,sizeof(*sweep)); }
int sn_sweep_append(SnSweep *sweep,const SnRow *row,double low,double high) {
 if(sweep->count+row->count>SN_MAX_BINS) return sn_fail("sweep exceeds 4 million input bins");
 if(sweep->count+row->count>sweep->capacity) {
  size_t cap=sweep->capacity?sweep->capacity:256; while(cap<sweep->count+row->count) cap*=2;
  SnBin *next=realloc(sweep->bins,cap*sizeof(*next)); if(!next) return sn_fail("sweep allocation failed"); sweep->bins=next; sweep->capacity=cap;
 }
 for(size_t i=0;i<row->count;++i) { double begin=row->low+(double)i*row->width,center=begin+row->width/2; if(center>=low&&center<high) sweep->bins[sweep->count++]=(SnBin){begin,row->width,row->power[i]}; }
 ++sweep->rows; return 0;
}
typedef struct { SnDb *db; SnStats *stats; const char *timestamp; } EmitContext;
static int emit(const SnEvent *event,void *opaque) {
 EmitContext *ctx=opaque; if(sn_db_event(ctx->db,ctx->timestamp,event)) return -1; ++ctx->stats->events; return 0;
}
int sn_flush(SnDb *db,const SnConfig *config,SnStats *stats,SnSweep *sweep) {
 if(!sweep->count) { sweep->rows=0; return 0; }
 double began=sn_clock();
 size_t before=stats->events; EmitContext context={db,stats,sweep->timestamp};
 if(sn_detect(sweep,config->margin,emit,&context)) return -1;
 double coverage=0,right=config->start;
 for(size_t i=0;i<sweep->count;++i) { double low=fmax(config->start,sweep->bins[i].low),high=fmin(config->stop,sweep->bins[i].low+sweep->bins[i].width); coverage+=fmax(0,high-fmax(low,right)); right=fmax(right,high); }
 if(sn_db_sweep(db,sweep,coverage,stats->events-before)||sn_db_commit(db)) return -1;
 sn_log(SN_LOG_INFO,"sweep","timestamp=%s bins=%zu rows=%zu events=%zu coverage_hz=%.0f coverage_fraction=%.6f processing_ms=%.3f",sweep->timestamp,sweep->count,sweep->rows,stats->events-before,coverage,db->requested_span>0?coverage/db->requested_span:0,1000*(sn_clock()-began));
 ++stats->sweeps; sweep->count=0; sweep->rows=0; return 0;
}
int sn_replay(SnDb *db,const SnConfig *config,SnStats *stats) {
 sn_log(SN_LOG_DEBUG,"replay","open path=%s",config->replay);
 FILE *input=sn_open_regular(config->replay); if(!input) return sn_fail("open replay %s: %s",config->replay,strerror(errno));
 const size_t limit=1024*1024; char *line=malloc(limit); if(!line) { fclose(input); return sn_fail("replay allocation failed"); }
 SnSweep sweep={0}; size_t line_number=0; int error=0;
 while(!sn_cancelled&&fgets(line,(int)limit,input)) {
  ++line_number;
  if(!strchr(line,'\n')&&!feof(input)) { int ch; while((ch=fgetc(input))!=EOF&&ch!='\n'&&!sn_cancelled) {} ++stats->errors; if(sn_db_bad(db,line_number,line,"line exceeds 1 MiB")) { error=-1; break; } continue; }
  char *trim=line; while(isspace((unsigned char)*trim)) ++trim; if(!*trim) continue;
  SnRow row;
  if(sn_parse_row(line,&row)) { char reason[512]; memcpy(reason,sn_error,sizeof(reason)); sn_log(SN_LOG_WARN,"replay","rejected line=%zu reason=%s",line_number,reason); ++stats->errors; if(sn_db_bad(db,line_number,line,reason)) { error=-1; break; } continue; }
  if(strcmp(row.timestamp,sweep.timestamp)) {
   if(*sweep.timestamp&&strcmp(row.timestamp,sweep.timestamp)<0) { sn_row_free(&row); error=sn_fail("timestamps moved backwards"); break; }
   if(sn_flush(db,config,stats,&sweep)) { sn_row_free(&row); error=-1; break; }
   snprintf(sweep.timestamp,sizeof(sweep.timestamp),"%s",row.timestamp);
  }
  sn_log(SN_LOG_TRACE,"replay","line=%zu timestamp=%s low_hz=%.0f high_hz=%.0f bins=%zu",line_number,row.timestamp,row.low,row.high,row.count);
  int inserted=sn_db_raw(db,&row,line);
  if(inserted<0||(inserted&&sn_sweep_append(&sweep,&row,config->start,config->stop))) error=-1;
  if(inserted>0) ++stats->rows;
  sn_row_free(&row); if(error) break;
 }
 if(!error&&ferror(input)) error=sn_fail("replay read failed");
 if(!error&&!sn_cancelled) error=sn_flush(db,config,stats,&sweep);
 if(!error&&!sn_cancelled&&!stats->sweeps) error=sn_fail("no valid sweep data in requested range");
 free(line); sn_sweep_free(&sweep); if(fclose(input)&&!error) error=sn_fail("replay close failed"); return error;
}
