#include "sentinel.h"
#include <dirent.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
static int suffix(char *out,size_t size,const char *path,const char *extension) {
 const char *slash=strrchr(path,'/'),*dot=strrchr(path,'.'); size_t n=dot&&(!slash||dot>slash)?(size_t)(dot-path):strlen(path);
 if(n>=size||strlen(extension)>=size-n) return sn_fail("capture path too long");
 memcpy(out,path,n); strcpy(out+n,extension); return 0;
}
static int numeric(const char *text,double *value) {
 char *end; errno=0; *value=strtod(text,&end); if(errno||end==text||!isfinite(*value)) return -1;
 while(*end==' '||*end=='\t'||*end=='\r'||*end=='\n') ++end;
 return *end?-1:0;
}
int sn_analyze_file(const char *path,double *sample_rate,double *frequency,SnFeatures *features) {
 char meta[SN_PATH]; if(suffix(meta,sizeof(meta),path,".meta")) return -1;
 FILE *sidecar=fopen(meta,"r"); double declared=-1;
 if(sidecar) {
  char line[1024];
  while(fgets(line,sizeof(line),sidecar)) {
   if(!strchr(line,'\n')&&!feof(sidecar)) { fclose(sidecar); return sn_fail("metadata line too long"); }
   char *equal=strchr(line,'='); if(!equal) continue; *equal++=0; double value;
   if(!strcmp(line,"sample_rate")||!strcmp(line,"freq_mhz")||!strcmp(line,"samples")) {
    if(numeric(equal,&value)) { fclose(sidecar); return sn_fail("invalid numeric capture metadata"); }
    if(!strcmp(line,"sample_rate")) *sample_rate=value;
    else if(!strcmp(line,"freq_mhz")) *frequency=value*1e6;
    else { if(value<0||floor(value)!=value) { fclose(sidecar); return sn_fail("invalid declared sample count"); } declared=value; }
   } else if(!strcmp(line,"status")) { equal[strcspn(equal,"\r\n")]=0; if(strcmp(equal,"complete")) { fclose(sidecar); return sn_fail("capture is not complete"); } }
  }
  int bad=ferror(sidecar); if(fclose(sidecar)) bad=1; if(bad) return sn_fail("metadata read failed");
 } else if(errno!=ENOENT) return sn_fail("open metadata: %s",strerror(errno));
 if(!isfinite(*sample_rate)||*sample_rate<=0||*sample_rate>20e6||!isfinite(*frequency)||*frequency<1e6||*frequency>6e9) return sn_fail("valid sample_rate and freq_mhz metadata or explicit flags required");
 FILE *input=fopen(path,"rb"); if(!input) return sn_fail("open IQ %s: %s",path,strerror(errno)); struct stat st;
 if(fstat(fileno(input),&st)||!S_ISREG(st.st_mode)||st.st_size<8192||st.st_size%2||(declared>=0&&declared!=(double)(st.st_size/2))) { fclose(input); return sn_fail("short, odd-length or metadata-mismatched CS8 capture"); }
 size_t samples=(size_t)(st.st_size/2); if(samples>262144) samples=262144;
 int8_t *raw=malloc(samples*2); if(!raw) { fclose(input); return sn_fail("IQ allocation failed"); }
 int error=0; if(fread(raw,2,samples,input)!=samples) error=sn_fail("IQ read failed"); if(fclose(input)&&!error) error=sn_fail("IQ close failed");
 if(!error) error=sn_features(raw,samples,*sample_rate,features);
 free(raw); return error;
}
int sn_capture(SnReceiver *receiver,const SnConfig *c,double frequency,const char *directory,SnFeatures *features,char *path,size_t path_size) {
 if(!isfinite(c->sample_rate)||!isfinite(c->duration)||c->sample_rate<=0||c->duration<=0||c->sample_rate*c->duration>20000000||!isfinite(frequency)||frequency<1e6||frequency>6e9) return sn_fail("invalid capture parameters");
 size_t samples=(size_t)llround(c->sample_rate*c->duration); if(samples<4096) return sn_fail("capture too short");
 if(sn_mkdirs(directory)) return -1;
 struct statvfs disk;
 if(statvfs(directory,&disk)||(long double)disk.f_bavail*(long double)disk.f_frsize<(long double)samples*2+16000000) return sn_fail("insufficient disk space or disk information unavailable");
 char id[33],temporary[SN_PATH],meta[SN_PATH],meta_temp[SN_PATH],timestamp[40];
 if(sn_id(id)||sn_utc(timestamp)||sn_path(path,path_size,"%s/iq_%.0fHz_%s.cs8",directory,frequency,id)||sn_path(temporary,sizeof(temporary),"%s.partial",path)||suffix(meta,sizeof(meta),path,".meta")||sn_path(meta_temp,sizeof(meta_temp),"%s.partial",meta)) return -1;
 int8_t *raw=malloc(samples*2); if(!raw) return sn_fail("capture allocation failed");
 int error=sn_receiver_read(receiver,(uint64_t)llround(frequency),c->sample_rate,raw,samples,c->timeout+c->duration);
 if(sn_cancelled) { free(raw); return sn_fail("capture interrupted"); }
 if(!error) {
  FILE *f=fopen(temporary,"wb"); if(!f) error=sn_fail("create capture: %s",strerror(errno));
  else { if(fwrite(raw,2,samples,f)!=samples) error=sn_fail("capture write failed"); if(sn_atomic(f,temporary,path)) error=-1; }
 }
 if(!error) {
  FILE *f=fopen(meta_temp,"w"); if(!f) error=sn_fail("create capture metadata: %s",strerror(errno));
  else { fprintf(f,"freq_mhz=%.9f\ncenter_hz=%.0f\nsample_rate=%.0f\nsamples=%zu\ndur_sec=%.9f\nvga=%u\nlna=%u\nreceiver_serial_request=%s\ncapture_start_utc=%s\nsample_format=cs8\nrf_amp=0\nantenna_power=0\nstatus=complete\n",frequency/1e6,frequency,c->sample_rate,samples,(double)samples/c->sample_rate,c->vga,c->lna,c->serial?c->serial:"default_receiver",timestamp); if(sn_atomic(f,meta_temp,meta)) error=-1; }
 }
 if(!error) error=sn_features(raw,samples>262144?262144:samples,c->sample_rate,features);
 free(raw); if(error) { unlink(temporary); unlink(meta_temp); unlink(path); unlink(meta); } return error;
}
int sn_write_features(FILE *stream,const char *path,double sr,double frequency,const SnFeatures *f) {
 fprintf(stream,"{\"type\":\"waveform_descriptor_v1_c\",\"file\":"); sn_json(stream,path);
 fprintf(stream,",\"status\":\"%s\",\"device_identity\":null,\"class\":\"unknown\",\"sample_rate_hz\":%.9f,\"center_hz\":%.9f,\"samples_analyzed\":%zu,\"fft_size\":4096,\"frames\":%zu,\"occupied_bw_99_hz\":%.9f,\"spectral_flatness_db\":%.9f,\"envelope_crest_db\":%.9f,\"normalized_fourth_moment\":%.9f,\"dc_power_fraction\":%.9f,\"dc_removed\":false,\"clipped_component_fraction\":%.9f,\"spectral_power_shape_32\":[",f->zero_energy?"zero_energy":"ok",sr,frequency,f->samples,f->frames,f->obw,f->flatness,f->crest,f->moment4,f->dc_fraction,f->clipped);
 for(size_t i=0;i<32;++i) fprintf(stream,"%s%.12g",i?",":"",f->shape[i]);
 fprintf(stream,"]}\n");
 return ferror(stream)?sn_fail("analysis output failed"):0;
}
int sn_analyze_directory(const char *directory,FILE *stream,SnStats *stats) {
 DIR *dir=opendir(directory); if(!dir) return sn_fail("open capture directory: %s",strerror(errno)); struct dirent *entry; int error=0; errno=0;
 while(!sn_cancelled&&(entry=readdir(dir))) {
  size_t length=strlen(entry->d_name); if(length<4||strcmp(entry->d_name+length-4,".cs8")) continue;
  char path[SN_PATH]; SnFeatures f; double sr=0,fc=0;
  if(sn_path(path,sizeof(path),"%s/%s",directory,entry->d_name)) { error=-1; break; }
  if(sn_analyze_file(path,&sr,&fc,&f)) { ++stats->capture_errors; fprintf(stream,"{\"type\":\"capture_error\",\"file\":"); sn_json(stream,path); fprintf(stream,",\"error\":"); sn_json(stream,sn_error); fprintf(stream,"}\n"); }
  else { ++stats->captures; if(sn_write_features(stream,path,sr,fc,&f)) { error=-1; break; } }
  errno=0;
 }
 if(!error&&errno&&!sn_cancelled) error=sn_fail("capture directory read failed");
 if(closedir(dir)&&!error) error=sn_fail("capture directory close failed");
 return error;
}
