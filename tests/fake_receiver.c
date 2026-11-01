/* Deterministic test backend; never linked into bin/sentinel. */
#include "sentinel.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
struct SnReceiver { int unused; };
int sn_receiver_open(SnReceiver **out,const SnConfig *config) {
 (void)config; *out=NULL;
 if(getenv("SN_TEST_OPEN_FAIL")) return sn_fail("injected receiver open failure");
 *out=calloc(1,sizeof(**out)); return *out?0:sn_fail("test allocation failed");
}
int sn_receiver_read(SnReceiver *r,uint64_t frequency,double sr,int8_t *buffer,size_t samples,double timeout) {
 (void)r; (void)frequency;
 if(getenv("SN_TEST_READ_FAIL")||(getenv("SN_TEST_CAPTURE_FAIL")&&sr<20000000)) return sn_fail("injected receive failure");
 if(getenv("SN_TEST_STALL")) {
  double end=sn_clock()+timeout;
  while(!sn_cancelled&&sn_clock()<end) { struct timespec pause={0,1000000}; nanosleep(&pause,NULL); }
  return sn_cancelled?0:sn_fail("injected receive timeout");
 }
 for(size_t i=0;i<samples;++i) {
  buffer[2*i]=(int8_t)llround(50*cos(2*3.141592653589793*.1*(double)i));
  buffer[2*i+1]=(int8_t)llround(50*sin(2*3.141592653589793*.1*(double)i));
 }
 return 0;
}
int sn_receiver_close(SnReceiver *r) {
 int fail=r&&getenv("SN_TEST_CLOSE_FAIL"); free(r); return fail?sn_fail("injected receiver close failure"):0;
}
const char *sn_receiver_backend(void) { return "TEST_ONLY_SYNTHETIC"; }
