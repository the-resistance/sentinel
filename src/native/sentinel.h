#ifndef SENTINEL_NATIVE_H
#define SENTINEL_NATIVE_H
#define _POSIX_C_SOURCE 200809L
#include <complex.h>
#include <signal.h>
#include <sqlite3.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#define SN_VERSION "0.2.0-c"
#define SN_MAX_BINS 4000000u
#define SN_PATH 4096
extern volatile sig_atomic_t sn_cancelled;
extern char sn_error[512];
FILE *sn_open_regular(const char *path);
int sn_fail(const char *format,...);
int sn_path(char *out,size_t n,const char *format,...);
int sn_mkdirs(const char *path);
int sn_id(char out[33]);
double sn_clock(void);
int sn_utc(char out[40]);
void sn_json(FILE *stream,const char *text);
int sn_atomic(FILE *stream,const char *temporary,const char *final);
typedef struct {
 const char *command,*workdir,*replay,*input,*captures_dir,*serial;
 double start,stop,bin_hz,margin,timeout,duration,sample_rate,frequency;
 unsigned sweeps,lna,vga,max_targets;
 int capture;
} SnConfig;
typedef struct { double low,width,power; } SnBin;
typedef struct { char timestamp[40]; double low,high,width; unsigned samples; double *power; size_t count; } SnRow;
typedef struct { char timestamp[40]; SnBin *bins; size_t count,capacity,rows; } SnSweep;
typedef struct { double low,high,center,bandwidth,peak,noise,contrast,bin_width; } SnEvent;
typedef struct {
 double obw,flatness,crest,moment4,dc_fraction,clipped,shape[32];
 size_t samples,frames;
 int zero_energy;
} SnFeatures;
typedef struct { size_t rows,errors,sweeps,events,captures,capture_errors; } SnStats;
typedef struct { sqlite3 *handle; char session[33]; int transaction; FILE *events; double requested_span; } SnDb;
typedef int (*SnEventFn)(const SnEvent *,void *);
int sn_parse_row(const char *line,SnRow *row);
void sn_row_free(SnRow *row);
void sn_sweep_free(SnSweep *sweep);
int sn_sweep_append(SnSweep *sweep,const SnRow *row,double low,double high);
int sn_detect(SnSweep *sweep,double margin,SnEventFn emit,void *context);
int sn_psd(const int8_t *raw,size_t samples,size_t nfft,double *power);
double sn_obw(const double *power,size_t count,double width,double fraction);
int sn_features(const int8_t *raw,size_t samples,double sr,SnFeatures *result);
int sn_db_open(SnDb *db,const char *path,int readonly);
int sn_db_start(SnDb *db,const SnConfig *config);
int sn_db_raw(SnDb *db,const SnRow *row,const char *raw);
int sn_db_bad(SnDb *db,size_t line,const char *raw,const char *reason);
int sn_db_event(SnDb *db,const char *timestamp,const SnEvent *event);
int sn_db_sweep(SnDb *db,const SnSweep *sweep,double coverage,size_t events);
int sn_db_commit(SnDb *db);
int sn_db_finish(SnDb *db,const char *state,const char *detail);
int sn_db_report(SnDb *db,FILE *output,int current);
int sn_db_targets(SnDb *db,double *centers,size_t maximum,size_t *count);
void sn_db_close(SnDb *db);
int sn_flush(SnDb *db,const SnConfig *config,SnStats *stats,SnSweep *sweep);
int sn_replay(SnDb *db,const SnConfig *config,SnStats *stats);
typedef struct SnReceiver SnReceiver;
int sn_receiver_open(SnReceiver **out,const SnConfig *config);
int sn_receiver_read(SnReceiver *receiver,uint64_t frequency,double sr,int8_t *buffer,size_t count,double timeout);
int sn_receiver_close(SnReceiver *receiver);
const char *sn_receiver_backend(void);
int sn_live(SnDb *db,const SnConfig *config,SnStats *stats);
int sn_capture(SnReceiver *receiver,const SnConfig *config,double frequency,const char *directory,SnFeatures *features,char *path,size_t path_size);
int sn_analyze_file(const char *path,double *sample_rate,double *frequency,SnFeatures *features);
int sn_write_features(FILE *stream,const char *path,double sr,double frequency,const SnFeatures *features);
int sn_analyze_directory(const char *directory,FILE *stream,SnStats *stats);
#endif
