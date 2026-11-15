/* Exact old/new VST comparison for setter coalescing. No device settings/files. */
#define main unused_generic_main
#include "../../mpc-vst-plugins-release-tools/tools/bench.c"
#undef main
#include "params.h"
#include <sched.h>
#include <sys/resource.h>
static unsigned random_state=1234567;
static float random_value(void){random_state=random_state*1664525u+1013904223u;return (random_state>>8)/16777216.f;}
static void units(AEffect*f,const char*key,float x){
 for(int i=0;i<NPARAMS;++i)if(!strcmp(PARAMS[i].key,key)){const param_t*p=&PARAMS[i];f->setParameter(f,i,p->nopts?x/(p->nopts-1):(x-p->min)/(p->max-p->min));return;}
 fprintf(stderr,"unknown %s\n",key);exit(2);
}
static AEffect*open_fx(const char*path,void**handle){
 *handle=dlopen(path,RTLD_NOW|RTLD_LOCAL);if(!*handle){fprintf(stderr,"%s\n",dlerror());exit(2);}AEffect*(*entry)(hostcb)=dlsym(*handle,"VSTPluginMain");AEffect*f=entry?entry(host):NULL;if(!f)exit(2);
 f->dispatcher(f,effOpen,0,0,0,0);f->dispatcher(f,effSetSampleRate,0,0,0,SR);f->dispatcher(f,effSetBlockSize,0,BLOCK,0,0);f->dispatcher(f,effMainsChanged,0,1,0,0);return f;
}
int main(int argc,char**argv){
 if(argc!=3 && argc!=4)return 2;
 if(argc==4){cpu_set_t cpus;CPU_ZERO(&cpus);CPU_SET(4,&cpus);if(strcmp(argv[3],"--pin") || sched_setaffinity(0,sizeof(cpus),&cpus) || setpriority(PRIO_PROCESS,0,19))return 2;}
 long samples=0;
 for(int test=0;test<22;++test){
  void*h[2];AEffect*f[2]={open_fx(argv[1],&h[0]),open_fx(argv[2],&h[1])};
  for(int j=0;j<2;++j){f[j]->dispatcher(f[j],2,0,10,0,0);units(f[j],"engine014",test>=2);units(f[j],"switchModState",1);units(f[j],"squareModel014",test==3);units(f[j],"waveform",1);units(f[j],"seqTiming",1);units(f[j],"seqLength64",64);units(f[j],"seqEnabled",test>=4);if(test>=6){units(f[j],"seqEnabled",0);units(f[j],"filterModel014",(test-6)%8);units(f[j],"filterBias014",0.4);units(f[j],"resonance",0.75);units(f[j],"filterQuality014",test>=14);units(f[j],"filterFm014",test>=14?0.3:0);units(f[j],"filterSaturation014",test>=14?4:0);}}
  memset(&ti,0,sizeof(ti));ti.sampleRate=SR;ti.tempo=120;ti.tsNum=4;ti.tsDen=4;ti.flags=2|1<<9|1<<10;
  double energy=0;
  const int blocks=test<6?512:256;
  for(int b=0;b<blocks;++b){
   float drive[5];for(int n=0;n<5;++n)drive[n]=random_value();
   for(int j=0;j<2;++j){
    for(int n=0;n<(test<6?5:0);++n){units(f[j],"sqrDriver",drive[n]);units(f[j],"switchModState",n%2);units(f[j],"circuitEnabled",n%2);units(f[j],"oscModsEnabled",n%2);units(f[j],"filterModsEnabled",n%2);}
    units(f[j],"switchModState",1);units(f[j],"circuitEnabled",1);units(f[j],"oscModsEnabled",1);units(f[j],"filterModsEnabled",1);
    units(f[j],"seqSelected64",1+b%64);units(f[j],"seqNote",36+b%25);units(f[j],"seqGridAccent1",b%2);units(f[j],"seqGridSlide2",b%2);units(f[j],"seqKeyRoot",b%12);units(f[j],"seqKeyScale",b%20);
    if(test>=14 && b%32==0)units(f[j],"filterModel014",(b/32+test)%8);
    if(test>=6){units(f[j],"cutoff",0.1+((b*137)%800)*0.001);units(f[j],"filterBias014",(b%10)*0.08);}
    if(test>=6 && b%128==64){fx=f[j];send_notes(0,1,36);}
    if((test<4 && b%64==0) || (test>=6 && b%128==0)){fx=f[j];send_notes(0,1,36);send_notes(1,1,36);}
   }
   float a[BLOCK],ar[BLOCK],bb[BLOCK],br[BLOCK];float*oa[]={a,ar},*ob[]={bb,br};f[0]->processReplacing(f[0],ins,oa,BLOCK);f[1]->processReplacing(f[1],ins,ob,BLOCK);
   for(int i=0;i<BLOCK;++i){if(memcmp(&a[i],&bb[i],sizeof(float)) || a[i]!=ar[i] || bb[i]!=br[i] || !isfinite(a[i])){fprintf(stderr,"audio mismatch test%d block%d sample%d old%.9g new%.9g\n",test,b,i,a[i],bb[i]);return 1;}energy+=a[i]*a[i];++samples;}
   ti.samplePos+=BLOCK;ti.ppqPos+=BLOCK*120./(60*SR);
  }
  if(energy<1e-8){fprintf(stderr,"silent comparison\n");return 2;}
  for(int j=0;j<2;++j){f[j]->dispatcher(f[j],effClose,0,0,0,0);dlclose(h[j]);}
  printf("test%d: %d samples bit-identical under drive/bypass/step/scale/filter edits\n",test,blocks*BLOCK);fflush(stdout);
 }
 printf("%ld samples compared; exact old/new audio PASS\n",samples);return 0;
}
