// Behavioural checks through the real VST2 wrapper and an independently configured
// Open303 reference. These detect sample shifts, int16 quantisation, instance leaks,
// broken MIDI panic, invalid state handling, and unstable filter/LFO combinations.
#include "params.h"
extern "C" {
#include "engine.h"
}
#include "dsp/open303/rosic_Open303.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include <thread>
#include <chrono>

struct Effect;
using Callback = intptr_t (*)(Effect*,int32_t,int32_t,intptr_t,void*,float);
struct Effect {
    int32_t magic;
    intptr_t (*dispatch)(Effect*,int32_t,int32_t,intptr_t,void*,float);
    void (*process)(Effect*,float**,float**,int32_t);
    void (*set)(Effect*,int32_t,float);
    float (*get)(Effect*,int32_t);
    int32_t programs,params,inputs,outputs,flags;
    intptr_t reserved1,reserved2;
    int32_t delay,quality1,quality2;
    float ratio;
    void *object,*user;
    int32_t uid,version;
    void (*replace)(Effect*,float**,float**,int32_t);
    void* doubleProcess;
    char future[56];
};
struct MidiEvent { int32_t type=1,bytes=32,frame=0,flags=0,length=0,offset=0;
    uint8_t midi[4]{}; char rest[4]{}; };
struct Events { int32_t count=1; intptr_t reserved=0; void* events[2]{}; };
extern "C" Effect* VSTPluginMain(Callback);

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
struct HostUi {
    std::array<std::atomic<float>,NPARAMS> cached{};
    std::array<std::atomic<int>,NPARAMS> reports{};
    inline static thread_local bool inWrite=false;
    std::atomic<bool> echo=false;
};
struct ActiveUi {
    std::recursive_mutex mutex;
    HostUi* ptr=nullptr;
    void operator=(HostUi* p){std::lock_guard<std::recursive_mutex> lock(mutex);ptr=p;}
    explicit operator bool(){std::lock_guard<std::recursive_mutex> lock(mutex);return ptr!=nullptr;}
} activeUi;
struct TimeInfo {
    double samplePos=0,sampleRate=44100,nanos=0,ppq=0,bpm=120,bar=0,cycleStart=0,cycleEnd=4;
    int32_t numerator=4,denominator=4,smpteOffset=0,smpteRate=0,nextClock=0,flags=0;
};
TimeInfo* activeTime=nullptr;
thread_local bool audioRender=false;
std::atomic<int> audioCallbacks=0,displayUpdates=0,totalUiCallbacks=0;
std::atomic<bool> blockUiCallbacks=false;
intptr_t host(Effect* fx,int32_t opcode,int32_t parameter,intptr_t,void*,float value) {
    if(opcode==0||opcode==42){
        ++totalUiCallbacks;if(audioRender)++audioCallbacks;
        if(opcode==42)++displayUpdates;
        if(blockUiCallbacks)std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    if(opcode==7) return reinterpret_cast<intptr_t>(activeTime);
    if (opcode==0 && parameter>=0 && parameter<NPARAMS) {
        std::lock_guard<std::recursive_mutex> lock(activeUi.mutex);
        if(auto* ui=activeUi.ptr){
            require(!HostUi::inWrite,"wrapper called the host recursively from setParameter");
            ui->cached[parameter]=value;
            ++ui->reports[parameter];
            if(ui->echo)fx->set(fx,parameter,value);
        }
    }
    return opcode == 1 ? 2400 : 0;
}
int index(const char* key) {
    for (int i=0; i<NPARAMS; ++i) if (!std::strcmp(PARAMS[i].key,key)) return i;
    throw std::runtime_error(key);
}
struct Plugin {
    Effect* fx = VSTPluginMain(host);
    // Existing DSP oracles exercise enabled submodules explicitly. Pass false
    // to check the real untouched defaults instead of that historical setup.
    Plugin(bool enablePages=true) { require(fx && fx->magic==0x56737450,"VST2 entry failed"); fx->dispatch(fx,0,0,0,nullptr,0); units("engine014",0); units("seqTiming",0); if(enablePages)for(int i=603;i<610;++i)fx->set(fx,i,1); }
    ~Plugin() { fx->dispatch(fx,1,0,0,nullptr,0); }
    void set(const char* key,float value) { fx->set(fx,index(key),value); }
    void units(const char* key,float value) {
        const auto& q=PARAMS[index(key)];
        set(key,q.nopts ? value/(q.nopts-1) : (value-q.min)/(q.max-q.min));
    }
    void note(uint8_t status,uint8_t key,uint8_t velocity,int frame=0) {
        MidiEvent event; event.frame=frame; event.midi[0]=status; event.midi[1]=key; event.midi[2]=velocity;
        Events events; events.events[0]=&event;
        fx->dispatch(fx,25,0,0,&events,0);
    }
    std::vector<float> render(int frames) {
        std::vector<float> left(frames),right(frames);
        float* out[]{left.data(),right.data()};
        audioRender=true;
        fx->replace(fx,nullptr,out,frames);
        audioRender=false;
        if(activeUi)std::this_thread::sleep_for(std::chrono::milliseconds(20));
        require(left==right,"mono voice must reach both stereo channels identically");
        for (float x:left) require(std::isfinite(x),"non-finite plugin output");
        return left;
    }
};

void referenceTest() {
    for (int filter=0;filter<3;++filter) {
        Plugin p;
        p.set("switchModState",1);
        p.set("filterType",filter/2.0f);
        p.set("volume",0.25f); // Quiet enough to expose the old int16 conversion.
        auto reference=std::make_unique<rosic::Open303>();
        reference->setSampleRate(44100);
        reference->setWaveform(1);
        reference->setTuning(440);
        reference->setCutoff(314);
        reference->setResonance(92);
        reference->setEnvMod(0);
        reference->setDecay(30*std::pow(100,static_cast<double>(PARAMS[index("decay")].def)));
        reference->setAccent(100*PARAMS[index("accent")].def);
        reference->setVolume(-45);
        reference->setAmpDecay(30+2970*static_cast<double>(PARAMS[index("normalDecay")].def));
        reference->setAccentDecay(30+2970*static_cast<double>(PARAMS[index("accentDecay")].def));
        reference->setFeedbackHighpass(350*std::pow(100.0/350,static_cast<double>(PARAMS[index("feedbackFilter")].def)));
        reference->setNormalAttack(.3*std::pow(10000,static_cast<double>(PARAMS[index("softAttack")].def)));
        reference->setSlideTime(2+998*static_cast<double>(PARAMS[index("slideTime")].def));
        reference->setTanhShaperDrive(25+55*static_cast<double>(PARAMS[index("sqrDriver")].def));
        reference->setFilterType(static_cast<rosic::FilterType>(filter));
        reference->setFilterDrive(4.5);
        reference->setPassbandCompensation(PARAMS[index("bassComp")].def);
        reference->setLfoOn(true);
        p.note(0x90,36,127,37);
        auto first=p.render(128);
        require(p.fx->get(p.fx,index("switchModState"))==1,"mods switch did not enable");
        require(p.fx->get(p.fx,index("filterType"))==filter/2.0f,"filter option did not select");
        for (int i=0;i<37;++i) require(first[i]==0,"MIDI started before its sample offset");
        reference->noteOn(36,127,0);
        double maxError=0,energy=0;
        for(int i=37;i<128;++i) {
            float expected=static_cast<float>(reference->getSample());
            maxError=std::max(maxError,std::abs(static_cast<double>(first[i]-expected)));
        }
        for (int frames : {1,31,96,127,128,257,511}) {
            const auto audio=p.render(frames);
            for (float x:audio) {
                float expected=static_cast<float>(reference->getSample());
                maxError=std::max(maxError,std::abs(static_cast<double>(x-expected)));
                energy+=x*x;
            }
        }
        std::printf("filter %d: reference max error %.3g, energy %.6g\n",filter,maxError,energy);
        require(maxError<1e-8,"plugin changed the reference voice (precision or sample alignment)");
        require(energy>0,"reference test produced silence");
    }
}

void statesAndMidi() {
    Plugin p,other;
    p.set("cutoff",.63f);
    require(other.fx->get(other.fx,index("cutoff"))==0,"parameter state leaked between instances");
    void* chunk=nullptr;
    const auto length=p.fx->dispatch(p.fx,23,0,0,&chunk,0);
    require(length>0 && chunk,"missing project state");
    std::vector<char> saved(static_cast<char*>(chunk),static_cast<char*>(chunk)+length);
    p.set("cutoff",.1f);
    p.fx->dispatch(p.fx,24,0,saved.size(),saved.data(),0);
    require(std::abs(p.fx->get(p.fx,index("cutoff"))-.63f)<1e-6,"project restore failed");
    const char invalid[]="JC303-MPC-1\nwaveform 0\ntuning nan\n";
    p.fx->dispatch(p.fx,24,0,sizeof invalid,const_cast<char*>(invalid),0);
    require(std::abs(p.fx->get(p.fx,index("cutoff"))-.63f)<1e-6,"invalid state changed the voice");
    p.note(0x90,36,100);
    p.note(0x90,36,127);
    p.render(128);
    p.note(0x90,43,110,61); // overlap uses the engine's native slide
    p.render(128);
    p.note(0xb0,120,0);
    auto silent=p.render(257);
    require(std::all_of(silent.begin(),silent.end(),[](float x){return x==0;}),"all-sound-off failed");
    p.note(0x90,48,100);
    auto playing=p.render(1024);
    require(std::any_of(playing.begin(),playing.end(),[](float x){return x!=0;}),"voice did not recover after panic");
    auto alone=other.render(128);
    require(std::all_of(alone.begin(),alone.end(),[](float x){return x==0;}),"audio leaked between instances");
    std::puts("instance isolation, chunk restore, invalid chunks, overlapping MIDI, and panic: passed");
}

void modulationStress() {
    Plugin p;
    p.set("switchModState",1);
    for(int filter=0;filter<3;++filter) {
        p.set("filterType",filter/2.0f);
        for(int waveform=0;waveform<6;++waveform) {
            p.set("lfoWaveform",waveform/5.0f);
            for(int destination=0;destination<3;++destination) {
                p.set("lfoDestination",destination/2.0f);
                p.set("lfoDepth",1);
                p.set("lfoRate",1);
                p.set("resonance",1);
                p.set("cutoff",1);
                p.note(0x90,24+waveform*12,127);
                p.render(4096);
                p.note(0xb0,123,0);
                p.render(4096);
            }
        }
    }
    std::puts("54 filter/LFO combinations with MIDI and parameter edits: finite output");
}
void hostUiSync() {
    Plugin p;
    HostUi ui;
    activeUi=&ui;
    ui.echo=true; // MPC can echo audioMasterAutomate back into the plugin.
    auto turn=[&](const char* key,float raw,float expected) {
        const int i=index(key);
        ui.cached[i]=raw; // host widgets initially see the fractional Q-Link write
        const int before=ui.reports[i];
        ui.inWrite=true;
        p.set(key,raw);
        ui.inWrite=false;
        require(p.fx->get(p.fx,i)==expected,"Q-Link did not select the expected engine state");
        p.render(128);
        require(ui.cached[i]==expected,"host cached fractional Q-Link value; touch state is stale");
        require(ui.reports[i]==before+1,"canonical stepped value must be reported once");
        p.render(128);
        require(ui.reports[i]==before+1,"host echo scheduled repeated automation feedback");
    };
    turn("switchModState",1.0f/128,1);
    turn("switchModState",1-1.0f/128,0);
    turn("filterType",1.0f/128,.5f);
    turn("filterType",.5f+1.0f/128,1);
    turn("filterType",1-1.0f/128,.5f);
    turn("filterType",.5f-1.0f/128,0);
    turn("lfoWaveform",1.0f/128,.2f);
    turn("lfoDestination",1.0f/128,.5f);
    const int filter=index("filterType");
    ui.cached[filter]=1;
    p.set("filterType",1); // an exact touch choice already has canonical host state
    p.render(128);
    require(p.fx->get(p.fx,filter)==1 && ui.cached[filter]==1,"touch selection lost its state");
    activeUi=nullptr;
    auto driveText=[&]() {
        char text[64]{};
        p.fx->dispatch(p.fx,7,index("filterDrive"),0,text,0);
        return std::string(text);
    };
    require(driveText()=="Mods off","Filter Drive must explain the disabled mods state");
    p.set("switchModState",1);
    require(driveText()=="4.5 dB","Filter Drive must show gain when a diode filter is active");
    p.set("filterType",0);
    require(driveText()=="Diode only","Filter Drive must explain the inactive TeeBee path");
    std::puts("fractional Q-Link -> canonical host/touch state, including host echoes: passed");
}
void waveformSweep() {
    // A real host can echo each accepted option while an absolute knob gesture
    // continues through fractional values. Those echoes must not become a new
    // direction sample for the ongoing gesture.
    for (int echoMode=0;echoMode<3;++echoMode) {
        Plugin p;
        HostUi ui;
        ui.echo=echoMode==1;
        activeUi=&ui;
        const int id=index("lfoWaveform");
        float previous=0;
        int changes=0;
        auto sample=[&](float raw,bool up) {
            p.set("lfoWaveform",raw);
            p.render(128);
            if(echoMode==2) { // delayed echo, after the callback has returned
                p.set("lfoWaveform",ui.cached[id]);
                p.render(128);
            }
            const float now=p.fx->get(p.fx,id);
            require(up ? now>=previous : now<=previous,
                    "LFO waveform reverses during a one-direction knob sweep");
            require(std::abs(now-previous)<=.20001f,"LFO waveform skipped an option");
            if(now!=previous) ++changes;
            previous=now;
        };
        for(int k=1;k<=128;++k) sample(k/128.0f,true);
        require(previous==1 && changes==5,"up sweep must select each of the six waveforms once");
        changes=0;
        for(int k=127;k>=0;--k) sample(k/128.0f,false);
        require(previous==0 && changes==5,"down sweep must select each of the six waveforms once");
        // Data-wheel clicks use the current value, rather than an absolute sweep.
        for(float delta : {.01f,1.0f/128}) {
            for(int direction : {1,-1}) {
                for(int k=0;k<5;++k) {
                    const float before=p.fx->get(p.fx,id);
                    p.set("lfoWaveform",before+direction*delta);
                    p.render(128);
                    if(echoMode==2) p.set("lfoWaveform",ui.cached[id]);
                    const float after=p.fx->get(p.fx,id);
                    require(std::abs(after-(before+direction*.2f))<1e-5,
                            "waveform wheel click must move exactly one option");
                }
            }
        }
        activeUi=nullptr;
    }
    std::puts("LFO knob sweeps and wheel clicks, without/immediate/delayed host echoes: passed");
}
void slideRangeMigration() {
    Plugin p;
    p.set("slideTime",1);
    char text[64]{};
    p.fx->dispatch(p.fx,7,index("slideTime"),0,text,0);
    require(std::string(text)=="1000 ms","new slide range must reach one second");
    void* chunk=nullptr;
    const auto length=p.fx->dispatch(p.fx,23,0,0,&chunk,0);
    require(length>0,"missing slide migration fixture");
    std::string legacy(static_cast<char*>(chunk));
    legacy.replace(0,legacy.find('\n'),"JC303-MPC-1");
    legacy.erase(legacy.find("seqEnabled "));
    auto start=legacy.find("slideTime ");
    auto end=legacy.find('\n',start);
    legacy.replace(start,end-start,"slideTime 0.33");
    p.fx->dispatch(p.fx,24,0,legacy.size()+1,legacy.data(),0);
    p.fx->dispatch(p.fx,7,index("slideTime"),0,text,0);
    require(std::string(text)=="120 ms","old project must retain its original slide duration");
    require(std::abs(p.fx->get(p.fx,index("slideTime"))-.33f*358/998)<1e-6,
            "old slide value was not rescaled");
    std::puts("one-second slide range and old-project duration migration: passed");
}
#include "sequence_checks.h"
#include "drive_checks.h"
#include "modern_checks.h"
#include "sequence64_checks.h"
#include "scale_checks.h"
#include "playback_ui_checks.h"
#include "matrix_checks.h"
#include "run_mode_checks.h"
#include "mod_page_checks.h"
#include "drive_fx_checks.h"
#include "factory_checks.h"
int main() {
    try { referenceTest(); statesAndMidi(); modulationStress(); hostUiSync(); waveformSweep(); slideRangeMigration(); sequenceChecks(); driveChecks(); modernChecks(); sequence64Checks(); scaleChecks(); playbackUiChecks(); matrixChecks(); runModeChecks(); modPageChecks(); driveFxChecks(); factoryChecks(); }
    catch(const std::exception& e) { std::fprintf(stderr,"FAIL: %s\n",e.what()); return 1; }
    std::puts("JC303 behavioural tests PASSED");
    return 0;
}
