#include "sentinel.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#define PI 3.14159265358979323846
static int bin_order(const void *a,const void *b) {
 const SnBin *x=a,*y=b; if(x->low!=y->low) return x->low<y->low?-1:1;
 return (x->width>y->width)-(x->width<y->width);
}
static int number_order(const void *a,const void *b) { double x=*(const double *)a,y=*(const double *)b; return (x>y)-(x<y); }
int sn_detect(SnSweep *sweep,double margin,SnEventFn emit,void *context) {
 if(!sweep->count) return 0;
 qsort(sweep->bins,sweep->count,sizeof(*sweep->bins),bin_order);
 size_t count=0;
 for(size_t i=0;i<sweep->count;++i) {
  SnBin b=sweep->bins[i];
  if(count&&fabs(sweep->bins[count-1].low-b.low)<.001&&fabs(sweep->bins[count-1].width-b.width)<.001) sweep->bins[count-1].power=fmax(sweep->bins[count-1].power,b.power);
  else sweep->bins[count++]=b;
 }
 sweep->count=count;
 for(size_t start=0;start<count;) {
  size_t end=start+1;
  while(end<count&&fabs(sweep->bins[end-1].low+sweep->bins[end-1].width-sweep->bins[end].low)<=1&&fabs(sweep->bins[end-1].width-sweep->bins[end].width)<=1) ++end;
  int active=0; SnEvent e={0};
  for(size_t i=start;i<=end;++i) {
   int hit=0; double noise=0;
   if(i<end) {
    double refs[32]; size_t used=0;
    for(int offset=-18;offset<=18;++offset) {
     if(offset>=-2&&offset<=2) continue;
     long j=(long)i+offset; if(j>=(long)start&&j<(long)end) refs[used++]=sweep->bins[j].power;
    }
    if(used>=16) {
     qsort(refs,used,sizeof(*refs),number_order); noise=used%2?refs[used/2]:(refs[used/2-1]+refs[used/2])/2;
     hit=sweep->bins[i].power-noise>=margin;
    }
   }
   if(hit) {
    SnBin *b=&sweep->bins[i];
    if(!active) { e=(SnEvent){.low=b->low,.peak=b->power,.noise=noise,.bin_width=b->width}; active=1; }
    e.high=b->low+b->width;
    if(b->power>e.peak) { e.peak=b->power; e.noise=noise; }
   } else if(active) {
    e.center=(e.low+e.high)/2; e.bandwidth=e.high-e.low; e.contrast=e.peak-e.noise;
    if(emit(&e,context)) return -1;
    active=0;
   }
  }
  start=end;
 }
 return 0;
}
static void fft(double complex *x,size_t n) {
 for(size_t i=1,j=0;i<n;++i) {
  size_t bit=n>>1; for(;j&bit;bit>>=1) j^=bit; j^=bit;
  if(i<j) { double complex v=x[i]; x[i]=x[j]; x[j]=v; }
 }
 for(size_t length=2;length<=n;length<<=1) {
  double complex step=cexp(-2*PI*I/(double)length);
  for(size_t i=0;i<n;i+=length) {
   double complex w=1;
   for(size_t j=0;j<length/2;++j) { double complex u=x[i+j],v=w*x[i+j+length/2]; x[i+j]=u+v; x[i+j+length/2]=u-v; w*=step; }
  }
 }
}
int sn_psd(const int8_t *raw,size_t samples,size_t nfft,double *power) {
 if(nfft<64||nfft>16384||(nfft&(nfft-1))||samples<nfft) return sn_fail("invalid FFT/sample dimensions");
 double complex *frame=malloc(nfft*sizeof(*frame)); if(!frame) return sn_fail("FFT allocation failed");
 memset(power,0,nfft*sizeof(*power)); size_t frames=0;
 for(size_t start=0;start+nfft<=samples;start+=nfft/2) {
  for(size_t j=0;j<nfft;++j) { size_t k=start+j; double window=.5-.5*cos(2*PI*(double)j/(double)(nfft-1)); frame[j]=((double)raw[2*k]+I*(double)raw[2*k+1])/128*window; }
  fft(frame,nfft);
  for(size_t j=0;j<nfft;++j) { double v=cabs(frame[(j+nfft/2)%nfft]); power[j]+=v*v; }
  ++frames;
 }
 free(frame); for(size_t j=0;j<nfft;++j) power[j]/=(double)frames; return 0;
}
double sn_obw(const double *power,size_t count,double width,double fraction) {
 if(!count||!isfinite(width)||width<=0||!isfinite(fraction)||fraction<=0||fraction>1) return NAN;
 double total=0; for(size_t i=0;i<count;++i) { if(!isfinite(power[i])||power[i]<0) return NAN; total+=power[i]; }
 if(!isfinite(total)) return NAN;
 if(!total) return 0;
 double sum=0,tail=(1-fraction)/2; size_t low=count,high=count-1;
 for(size_t i=0;i<count;++i) { sum+=power[i]; if(low==count&&sum>tail*total) low=i; if(sum>=(1-tail)*total) { high=i; break; } }
 return (double)(high-low+1)*width;
}
int sn_features(const int8_t *raw,size_t samples,double sr,SnFeatures *f) {
 if(samples<4096||samples>262144||!isfinite(sr)||sr<=0) return sn_fail("analysis needs 4096..262144 samples and a positive sample rate");
 memset(f,0,sizeof(*f)); f->samples=samples;
 double a_sum=0,b_sum=0,sum=0,sum4=0,peak=0; size_t clipped=0;
 for(size_t i=0;i<samples;++i) { double a=(double)raw[2*i]/128,b=(double)raw[2*i+1]/128,p=a*a+b*b; a_sum+=a; b_sum+=b; sum+=p; sum4+=p*p; peak=fmax(peak,p); clipped+=(raw[2*i]==-128||raw[2*i]==127)+(raw[2*i+1]==-128||raw[2*i+1]==127); }
 f->clipped=(double)clipped/(2*(double)samples); if(!sum) { f->zero_energy=1; return 0; }
 double average=sum/(double)samples;
 f->dc_fraction=(a_sum*a_sum+b_sum*b_sum)/((double)samples*(double)samples)/average;
 f->moment4=sum4/(double)samples/(average*average); f->crest=10*log10(peak/average);
 double power[4096]={0}; if(sn_psd(raw,samples,4096,power)) return -1;
 f->frames=1+(samples-4096)/2048; double total=0,log_sum=0;
 for(size_t i=0;i<4096;++i) { total+=power[i]; log_sum+=log(fmax(power[i],1e-30)); }
 f->flatness=10/log(10)*(log_sum/4096-log(total/4096)); f->obw=sn_obw(power,4096,sr/4096,.99);
 for(size_t i=0;i<4096;++i) f->shape[i/128]+=power[i]/total;
 return 0;
}
