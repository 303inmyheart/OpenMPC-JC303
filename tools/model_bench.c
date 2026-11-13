/* Device-only comparison helper: load a private plugin copy, no MPC process
 * attachment or settings access. Reuse the OpenMPC block timer and host ABI. */
#define main generic_bench_main
#include "../../mpc-vst-plugins/tools/bench.c"
#undef main
#include <unistd.h>
#include <sched.h>
#include <sys/resource.h>

int main(int argc,char** argv) {
    if(argc<3){fprintf(stderr,"usage: model_bench plugin.so model-index|-1 [--audio]\n");return 2;}
    cpu_set_t cpus;CPU_ZERO(&cpus);CPU_SET(4,&cpus);
    if(sched_setaffinity(0,sizeof(cpus),&cpus))perror("helper affinity");
    if(setpriority(PRIO_PROCESS,0,19))perror("helper priority");
    if(!strcmp(argv[2],"--run") && argc>3) {
        execl(argv[1],argv[1],argv[3],(char*)0);perror("analysis test exec");return 2;
    }
    void* lib=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL);
    if(!lib){fprintf(stderr,"%s\n",dlerror());return 2;}
    AEffect* (*entry)(hostcb)=dlsym(lib,"VSTPluginMain");
    fx=entry?entry(host):NULL;
    if(!fx||fx->magic!=0x56737450)return 2;
    fx->dispatcher(fx,effOpen,0,0,0,0);
    fx->dispatcher(fx,effSetSampleRate,0,0,0,SR);
    fx->dispatcher(fx,effSetBlockSize,0,BLOCK,0,0);
    fx->dispatcher(fx,effMainsChanged,0,1,0,0);
    ti.sampleRate=SR;ti.tempo=120;ti.tsNum=4;ti.tsDen=4;
    ti.flags=(1<<9)|(1<<10); // Stopped while the model warms up.
    int model=atoi(argv[2]);
    fx->dispatcher(fx,2,0,10,0,0); // Acid Steps, same parameters/pattern every run.
    fx->setParameter(fx,22,0); // sequencer off during preparation
    if(model>=0) {
        fx->setParameter(fx,60,(float)model/36);
        fx->setParameter(fx,61,.35f);fx->setParameter(fx,62,.35f);
        fx->setParameter(fx,59,1);
        // Worker preparation is asynchronous: wait without advancing DSP time.
        usleep(500000);
        for(int i=0;i<4000;++i) {
            fx->processReplacing(fx,ins,outs,BLOCK);
            float state=fx->getParameter(fx,63);
            if(state>.9f){fprintf(stderr,"model load error\n");return 2;}
            if(state>.6f)break;
            if(i==3999){fprintf(stderr,"model load timeout\n");return 2;}
            usleep(1000);
        }
    }
    // Settle the fixed 50/100 ms ramps before measuring.
    for(int i=0;i<128;++i)fx->processReplacing(fx,ins,outs,BLOCK);
    const int idle=argc>3 && !strcmp(argv[3],"--idle");
    fx->setParameter(fx,22,idle?0:1);
    ti.samplePos=0;ti.ppqPos=0;if(!idle)ti.flags|=1<<1;
    int result=0;
    if(argc>3 && !strcmp(argv[3],"--audio")) {
        for(int b=0;b<2048;++b) {
            fx->processReplacing(fx,ins,outs,BLOCK);
            for(int i=0;i<BLOCK;++i)if(!isfinite(outL[i]))result=2;
            fwrite(outL,sizeof(float),BLOCK,stdout);
            ti.samplePos+=BLOCK;ti.ppqPos=ti.samplePos/SR*ti.tempo/60;
        }
    } else {
        synth=1;
        for(int i=0;i<3;++i) {
            Stage s=run(model<0?"dry+sequence":"model+sequence",0,1,12);
            printf("model=%d repeat=%d mean_pct=%.4f p99_pct=%.4f max_pct=%.4f workers=%.4f peak=%.6f\n",model,i,s.mean/BUDGET_US*100,s.p99/BUDGET_US*100,s.max/BUDGET_US*100,s.bg,s.peak);
        }
    }
    fx->dispatcher(fx,effClose,0,0,0,0);dlclose(lib);
    return result;
}
