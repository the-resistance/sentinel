/* Exercise the real C adapter against a controlled libhackrf API shim. */
#include "sentinel.h"
#include <libhackrf/hackrf.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
enum { NORMAL, STOP_FAIL, START_FAIL, BAD_TRANSFER, STALL, OPEN_FAIL };
static int mode;
static hackrf_sample_block_cb_fn saved_callback;
static void *saved_context;
static void transfer(size_t pairs) {
 uint8_t bytes[65536]; memset(bytes,7,sizeof(bytes));
 assert(pairs<=32768);
 hackrf_transfer block={.buffer=bytes,.buffer_length=(int)(pairs*2),.valid_length=mode==BAD_TRANSFER?-1:(int)(pairs*2),.rx_ctx=saved_context};
 saved_callback(&block);
}
int hackrf_init(void) { return HACKRF_SUCCESS; }
int hackrf_exit(void) { return HACKRF_SUCCESS; }
int hackrf_open(hackrf_device **device) { if(mode==OPEN_FAIL) return HACKRF_ERROR_NOT_FOUND; *device=malloc(1); return *device?HACKRF_SUCCESS:HACKRF_ERROR_NO_MEM; }
int hackrf_open_by_serial(const char *serial,hackrf_device **device) { (void)serial; return hackrf_open(device); }
int hackrf_close(hackrf_device *device) { free(device); saved_callback=NULL; saved_context=NULL; return HACKRF_SUCCESS; }
int hackrf_set_freq(hackrf_device *device,uint64_t frequency) { (void)device;(void)frequency; return HACKRF_SUCCESS; }
int hackrf_set_sample_rate(hackrf_device *device,double rate) { (void)device;(void)rate; return HACKRF_SUCCESS; }
int hackrf_set_amp_enable(hackrf_device *device,uint8_t value) { (void)device; assert(value==0); return HACKRF_SUCCESS; }
int hackrf_set_antenna_enable(hackrf_device *device,uint8_t value) { (void)device; assert(value==0); return HACKRF_SUCCESS; }
int hackrf_set_lna_gain(hackrf_device *device,uint32_t gain) { (void)device;(void)gain; return HACKRF_SUCCESS; }
int hackrf_set_vga_gain(hackrf_device *device,uint32_t gain) { (void)device;(void)gain; return HACKRF_SUCCESS; }
int hackrf_start_rx(hackrf_device *device,hackrf_sample_block_cb_fn callback,void *context) {
 (void)device; saved_callback=callback; saved_context=context;
 if(mode!=STALL) { transfer(32768); transfer(64); }
 return mode==START_FAIL?HACKRF_ERROR_OTHER:HACKRF_SUCCESS;
}
int hackrf_stop_rx(hackrf_device *device) { (void)device; return mode==STOP_FAIL?HACKRF_ERROR_OTHER:HACKRF_SUCCESS; }
int hackrf_is_streaming(hackrf_device *device) { (void)device; return HACKRF_TRUE; }
const char *hackrf_error_name(enum hackrf_error error) { (void)error; return "injected libhackrf error"; }
int main(void) {
 SnConfig config={.lna=16,.vga=20}; SnReceiver *receiver=NULL;
 mode=OPEN_FAIL; assert(sn_receiver_open(&receiver,&config)<0&&receiver==NULL);
 for(mode=NORMAL;mode<=STALL;++mode) {
  assert(!sn_receiver_open(&receiver,&config));
  int8_t *raw=malloc(128); assert(raw); memset(raw,0,128);
  int result=sn_receiver_read(receiver,915000000,2000000,raw,64,.01);
  if(mode==NORMAL) { assert(result==0); for(size_t i=0;i<128;++i) assert(raw[i]==7); }
  else assert(result<0);
  free(raw);
  /* A late callback must not touch a caller buffer after return, even on error. */
  transfer(64);
  if(mode==STOP_FAIL||mode==START_FAIL) {
   raw=malloc(128); assert(raw); assert(sn_receiver_read(receiver,915000000,2000000,raw,64,.01)<0); free(raw);
  }
  int closed=sn_receiver_close(receiver); assert(mode==STOP_FAIL?closed<0:closed==0);
 }
 puts("real receive-adapter lifecycle checks passed"); return 0;
}
