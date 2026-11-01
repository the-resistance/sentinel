#include "sentinel.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
int sn_live(SnDb *db,const SnConfig *c,SnStats *stats) {
 SnReceiver *receiver=NULL; if(sn_receiver_open(&receiver,c)) return -1;
 const double sr=20000000; size_t nfft=64;
 while(nfft<16384&&sr/(double)nfft>c->bin_hz) nfft*=2;
 size_t edge=nfft/8,kept=nfft-2*edge,samples=nfft*8;
 double width=sr/(double)nfft,usable=(double)kept*width;
 sn_log(SN_LOG_INFO,"survey","sample_rate_hz=%.0f fft=%zu actual_bin_hz=%.3f tile_span_hz=%.0f settle_pairs=32768",sr,nfft,width,usable);
 int8_t *raw=malloc(samples*2); double *power=malloc(nfft*sizeof(*power)); int error=0;
 if(!raw||!power) { error=sn_fail("live scan allocation failed"); goto done; }
 for(unsigned pass=0;!sn_cancelled&&(!c->sweeps||pass<c->sweeps);++pass) {
  SnSweep sweep={0}; if(sn_utc(sweep.timestamp)) { error=-1; break; }
  for(double tile=c->start;tile<c->stop&&!sn_cancelled;tile+=usable) {
   double center=fmin(6e9,fmax(1e6,tile+usable/2+width/2));
   double began=sn_clock();
   sn_log(SN_LOG_DEBUG,"tile","pass=%u center_hz=%.0f pairs=%zu",pass+1,center,samples);
   if(sn_receiver_read(receiver,(uint64_t)llround(center),sr,raw,samples,c->timeout)||sn_cancelled) { error=sn_cancelled?0:-1; break; }
   if(sn_psd(raw,samples,nfft,power)) { error=-1; break; }
   double *levels=malloc(kept*sizeof(*levels)); if(!levels) { error=sn_fail("tile allocation failed"); break; }
   for(size_t i=0;i<kept;++i) levels[i]=10*log10(fmax(power[edge+i]/((double)nfft*(double)nfft),1e-30));
   SnRow row={.low=center+(-(double)nfft/2+(double)edge-.5)*width,.width=width,.samples=(unsigned)samples,.power=levels,.count=kept};
   row.high=row.low+(double)kept*width; snprintf(row.timestamp,sizeof(row.timestamp),"%s",sweep.timestamp);
   size_t capacity=256+kept*32; char *csv=malloc(capacity); if(!csv) { free(levels); error=sn_fail("spectral row allocation failed"); break; }
   int length=snprintf(csv,capacity,"%.*s,%s,%.9f,%.9f,%.9f,%zu",10,row.timestamp,row.timestamp+11,row.low,row.high,width,samples);
   size_t used=length>0?(size_t)length:capacity;
   for(size_t i=0;i<kept&&used<capacity;++i) { length=snprintf(csv+used,capacity-used,",%.9f",levels[i]); if(length<0||(size_t)length>=capacity-used) { used=capacity; break; } used+=(size_t)length; }
   if(used>=capacity) error=sn_fail("spectral row exceeds buffer");
   else if(sn_db_raw(db,&row,csv)<0||sn_sweep_append(&sweep,&row,c->start,c->stop)) error=-1;
   else ++stats->rows;
   sn_log(SN_LOG_TRACE,"tile","center_hz=%.0f elapsed_ms=%.3f status=%s",center,1000*(sn_clock()-began),error?"failed":"ok");
   free(csv); free(levels); if(error) break;
  }
  if(!error&&!sn_cancelled) error=sn_flush(db,c,stats,&sweep);
  sn_sweep_free(&sweep); if(error) break;
 }
done:
 free(raw); free(power); char original[512]; memcpy(original,sn_error,sizeof(original));
 if(sn_receiver_close(receiver)&&!error) error=-1; else if(error) memcpy(sn_error,original,sizeof(sn_error)); return error;
}
