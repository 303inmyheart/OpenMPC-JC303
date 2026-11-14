/* Supplemental JC303 release-configuration measurements; NOT the stock verdict.
 * Uses the pinned OpenMPC timer/host unchanged. No plugin/project installation.
 * Run only on the real aarch64 MPC; affinity and priority affect this helper only.
 */
#define main openmpc_generic_main
#include "../../mpc-vst-plugins-release-tools/tools/bench.c"
#undef main
#include <sched.h>
#include <sys/resource.h>
#include <unistd.h>
static void param(int index,float value){fx->setParameter(fx,index,value);}
static void block(void){fx->processReplacing(fx,ins,outs,BLOCK);ti.samplePos+=BLOCK;ti.ppqPos+=BLOCK*120.0/(60*SR);}
int main(int argc,char**argv){
 if(argc!=2)return 2;
 setvbuf(stdout,NULL,_IOLBF,0);
 cpu_set_t cpus;CPU_ZERO(&cpus);CPU_SET(4,&cpus);
 if(sched_setaffinity(0,sizeof(cpus),&cpus)||setpriority(PRIO_PROCESS,0,19))return 2;
 struct Config {const char*name;int model,pre,filter;float disperse;};
 const struct Config cases[]={
  {"modern+sequence dry",-1,0,0,0},
  {"Blackstar post",16,0,0,0},
  {"Blackstar pre",16,1,0,0},
  {"Blackstar post+disperse",16,0,0,.5f},
  {"Blackstar pre+disperse",16,1,0,.5f},
  {"Spicy HQ dry",-1,0,1,0},
  {"Spicy HQ+Blackstar pre+disperse",16,1,1,.5f}
 };
 for(unsigned scenario=0;scenario<sizeof(cases)/sizeof(*cases);++scenario){
  const struct Config c=cases[scenario];void*lib=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL);if(!lib)return 2;
  AEffect*(*entry)(hostcb)=dlsym(lib,"VSTPluginMain");fx=entry?entry(host):NULL;if(!fx||fx->numParams<618)return 2;
  memset(&ti,0,sizeof(ti));ti.sampleRate=SR;ti.tempo=120;ti.tsNum=4;ti.tsDen=4;ti.flags=(1<<9)|(1<<10);
  fx->dispatcher(fx,effOpen,0,0,0,0);fx->dispatcher(fx,effSetSampleRate,0,0,0,SR);fx->dispatcher(fx,effSetBlockSize,0,BLOCK,0,0);fx->dispatcher(fx,effMainsChanged,0,1,0,0);
  fx->dispatcher(fx,2,0,10,0,0); /* frozen Acid Steps pattern: 16 steps */
  param(22,0);param(66,1);param(8,1);param(86,1);param(68,c.filter/7.0f);param(71,c.filter?1:0);
  param(61,.35f);param(62,.35f);param(615,c.pre);param(616,c.disperse);
  if(c.model>=0){
   param(60,c.model/36.0f);param(59,1);
   double deadline=now_us(CLOCK_MONOTONIC)+30e6;
   while(fx->getParameter(fx,63)<.6f){
    if(now_us(CLOCK_MONOTONIC)>deadline){fprintf(stderr,"model timeout\n");return 2;}
    block();usleep(1000);
   }
  }
  param(22,1);ti.samplePos=0;ti.ppqPos=0;ti.flags|=1<<1;
  for(int i=0;i<128;++i)block(); /* settle enable ramps before timing */
  synth=1;
  for(int repeat=0;repeat<3;++repeat){
   Stage s=run(c.name,0,1,12);
   if(!isfinite(s.peak)||s.peak<=1e-8){fprintf(stderr,"silent or invalid scenario\n");return 2;}
   printf("{\"scenario\":\"%s\",\"repeat\":%d,\"mean_pct\":%.4f,\"p99_pct\":%.4f,\"max_pct\":%.4f,\"threads_pct\":%.4f,\"peak\":%.6f,\"filter\":%d,\"pre_vca\":%d,\"disperse\":%.2f,\"model\":%d}\n",c.name,repeat,s.mean/BUDGET_US*100,s.p99/BUDGET_US*100,s.max/BUDGET_US*100,s.bg,s.peak,c.filter,c.pre,c.disperse,c.model);
  }
  fx->dispatcher(fx,effClose,0,0,0,0);dlclose(lib);fx=NULL;
 }
 return 0;
}
