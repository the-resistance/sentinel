#include "sentinel.h"
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef SN_WITH_HACKRF
#include <libhackrf/hackrf.h>
struct SnReceiver { hackrf_device *device; int8_t *buffer; size_t wanted,discard; _Atomic size_t received; _Atomic int callback_error; int streaming; };
static int callback(hackrf_transfer *transfer) {
 SnReceiver *r=transfer->rx_ctx;
 if(transfer->valid_length<0||transfer->valid_length>transfer->buffer_length||transfer->valid_length%2) { atomic_store(&r->callback_error,1); return 0; }
 size_t position=atomic_load_explicit(&r->received,memory_order_relaxed),count=(size_t)transfer->valid_length/2;
 size_t skip=count<r->discard?count:r->discard; r->discard-=skip; count-=skip;
 if(count>r->wanted-position) count=r->wanted-position;
 if(count) memcpy(r->buffer+2*position,transfer->buffer+2*skip,count*2);
 atomic_store_explicit(&r->received,position+count,memory_order_release); return 0;
}
static int check(int status,const char *operation) { return status==HACKRF_SUCCESS?0:sn_fail("%s: %s (%d)",operation,hackrf_error_name((enum hackrf_error)status),status); }
int sn_receiver_open(SnReceiver **out,const SnConfig *c) {
 *out=NULL; SnReceiver *r=calloc(1,sizeof(*r)); if(!r) return sn_fail("receiver allocation failed");
 if(check(hackrf_init(),"hackrf_init")) { free(r); return -1; }
 if(check(c->serial?hackrf_open_by_serial(c->serial,&r->device):hackrf_open(&r->device),"open HackRF")) { hackrf_exit(); free(r); return -1; }
 if(check(hackrf_set_amp_enable(r->device,0),"disable amplifier")||check(hackrf_set_antenna_enable(r->device,0),"disable antenna power")||check(hackrf_set_lna_gain(r->device,c->lna),"set LNA")||check(hackrf_set_vga_gain(r->device,c->vga),"set VGA")) { hackrf_close(r->device); hackrf_exit(); free(r); return -1; }
 *out=r; return 0;
}
int sn_receiver_read(SnReceiver *r,uint64_t frequency,double sr,int8_t *buffer,size_t count,double timeout) {
 if(!r||!buffer||!count) return sn_fail("invalid receive request");
 if(check(hackrf_set_sample_rate(r->device,sr),"set sample rate")||check(hackrf_set_freq(r->device,frequency),"tune frequency")) return -1;
 r->buffer=buffer; r->wanted=count; r->discard=32768; atomic_store(&r->received,0); atomic_store(&r->callback_error,0);
 if(check(hackrf_start_rx(r->device,callback,r),"start receive")) return -1;
 r->streaming=1; double deadline=sn_clock()+timeout; int error=0;
 while(!sn_cancelled&&atomic_load_explicit(&r->received,memory_order_acquire)<count) {
  if(atomic_load(&r->callback_error)) { error=sn_fail("invalid receiver transfer"); break; }
  if(hackrf_is_streaming(r->device)!=HACKRF_TRUE) { error=sn_fail("receiver stopped before requested samples arrived"); break; }
  if(sn_clock()>=deadline) { error=sn_fail("receiver timed out after %.3g seconds",timeout); break; }
  struct timespec pause={0,1000000}; nanosleep(&pause,NULL);
 }
 int stopped=hackrf_stop_rx(r->device); r->streaming=0; if(!error&&check(stopped,"stop receive")) error=-1; return error;
}
int sn_receiver_close(SnReceiver *r) {
 if(!r) return 0;
 int error=0; if(r->streaming&&check(hackrf_stop_rx(r->device),"stop receive")) error=-1;
 if(check(hackrf_close(r->device),"close receiver")) error=-1;
 if(check(hackrf_exit(),"hackrf_exit")) error=-1;
 free(r); return error;
}
const char *sn_receiver_backend(void) { return "libhackrf"; }
#else
struct SnReceiver { int unused; };
int sn_receiver_open(SnReceiver **out,const SnConfig *c) { (void)c; *out=NULL; return sn_fail("offline build: rebuild with BACKEND=hackrf and libhackrf-dev"); }
int sn_receiver_read(SnReceiver *r,uint64_t fc,double sr,int8_t *buffer,size_t count,double timeout) { (void)r;(void)fc;(void)sr;(void)buffer;(void)count;(void)timeout; return sn_fail("live receiver unavailable"); }
int sn_receiver_close(SnReceiver *r) { (void)r; return 0; }
const char *sn_receiver_backend(void) { return "offline_only"; }
#endif
