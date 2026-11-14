// JC303 GuitarML adapter. GPL-3.0-only; see CHOWDSP-LICENSE.md and VENDORED.md.
// Inference and model shape are the vendored JC303/RTNeural implementation.
// JSON parsing, weight updates, allocation and warmup run only on the worker.
#include "Drive.h"
#include "ModelData.h"
#include "RNNAccelerated.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <sstream>
#include <thread>

namespace {
constexpr float pi=3.14159265358979323846f;
constexpr int fadeSamples=441; // JC303's 10 ms fade out and in, at 44.1 kHz
struct Ramp {
    float current=0,target=0,step=0;int remaining=0;
    void reset(float x) {current=target=x;remaining=0;step=0;}
    void set(float x,int length) {
        if(x==target) return;
        target=x;remaining=std::max(1,length);step=(target-current)/remaining;
    }
    float next() {
        if(remaining>0) {current+=step;if(--remaining==0)current=target;}
        return current;
    }
};
// Mono 30 Hz Butterworth SVF highpass: chowdsp::SVFHighpass equations used
// by JC303's DCBlocker. The 8100 Hz correction high shelf is unity at the
// engine/model rate of 44100 Hz, so it needs no additional filter here.
struct DcBlocker {
    float s1=0,s2=0;
    const float g=std::tan(pi*30.0f/44100.0f);
    const float a1=1/(1+g*(g+1.4142135623730951f));
    const float a2=g*a1,a3=g*a2,ak=(g+1.4142135623730951f)*a1;
    float process(float x) noexcept {
        const float v3=x-s2;
        const float v0=a1*v3-ak*s1,v1=a2*v3+a1*s1,v2=a3*v3+a2*s1+s2;
        s1=2*v1-s1;s2=2*v2-s2;
        return v0;
    }
    void reset() {s1=s2=0;}
};
float gainFor(float level) {return std::pow(10.0f,(-30.0f+36.0f*level)/20.0f);}
}

struct GuitarDrive::Impl {
    enum State {Free,Building,Ready,Active};
    struct Slot {
        rnn_sse_arm::RNNAccelerated<1,40,RecurrentLayerType::LSTMLayer,0> plain;
        rnn_sse_arm::RNNAccelerated<2,40,RecurrentLayerType::LSTMLayer,0> conditioned;
        DcBlocker dc;
        std::atomic<int> state{Free};
        uint64_t token=0;
        int inputs=1;
        float warmedLevel=.15f;
        float process(float dry,float condition,float gain) noexcept {
            float x=inputs==1 ? dry*gain : dry;
            if(inputs==1) plain.process({&x,1},true);
            else conditioned.process_conditioned({&x,1},{&condition,1},true);
            return dc.process(x);
        }
    };
    std::array<Slot,3> slots;
    Settings settings;
    std::atomic<uint64_t> requested{0},failed{0};
    std::atomic<float> warmLevel{.15f};
    std::atomic<bool> quit{false};
    std::mutex waitMutex;
    std::condition_variable wake;
    std::thread worker;
    int active=-1,pending=-1;
    uint64_t activeToken=0;
    float mixModulation=0;
    Ramp condition,gain,mix,enabled,modelFade;
    float cachedMix=-1,dryGain=1,wetGain=0;

    Impl() {
        condition.reset(.15f);gain.reset(gainFor(.15f));mix.reset(.35f);
        enabled.reset(0);modelFade.reset(1);
        worker=std::thread([this]{run();});
    }
    ~Impl() {
        quit.store(true,std::memory_order_release);wake.notify_one();worker.join();
    }
    void request() {
        const uint64_t old=requested.load(std::memory_order_relaxed);
        const auto token=(((old>>8)+1)<<8)|static_cast<uint64_t>(settings.model);
        requested.store(token,std::memory_order_release);
        wake.notify_one();
    }
    void run() {
        uint64_t handled=0;
        while(!quit.load(std::memory_order_acquire)) {
            const auto token=requested.load(std::memory_order_acquire);
            if(token==0 || token==handled) {
                std::unique_lock<std::mutex> lock(waitMutex);
                wake.wait_for(lock,std::chrono::milliseconds(10));
                continue;
            }
            Slot* slot=nullptr;
            for(auto& candidate:slots) {
                int expected=Free;
                if(candidate.state.compare_exchange_strong(expected,Building,std::memory_order_acq_rel)) {slot=&candidate;break;}
            }
            if(!slot) {
                std::unique_lock<std::mutex> lock(waitMutex);
                wake.wait_for(lock,std::chrono::milliseconds(5));continue;
            }
            try {
                const int model=static_cast<int>(token&255);
                const auto json=nlohmann::json::parse(jc303drive::models[model].json);
                slot->inputs=jc303drive::models[model].inputs;
                slot->warmedLevel=warmLevel.load(std::memory_order_acquire);
                if(slot->inputs==1) {slot->plain.initialise(json);slot->plain.prepare(1.0f);}
                else {slot->conditioned.initialise(json);slot->conditioned.prepare(1.0f);}
                slot->dc.reset();
                const float inputGain=gainFor(slot->warmedLevel);
                bool cancelled=false;
                for(int i=0;i<5120;++i) {
                    if((i&127)==0 && (quit.load() || requested.load(std::memory_order_acquire)!=token)) {cancelled=true;break;}
                    slot->process(0,slot->warmedLevel,inputGain);
                }
                if(cancelled || requested.load(std::memory_order_acquire)!=token) {
                    slot->state.store(Free,std::memory_order_release);continue;
                }
                slot->token=token;
                slot->state.store(Ready,std::memory_order_release);
                handled=token;
            } catch(...) {
                slot->state.store(Free,std::memory_order_release);
                failed.store(token,std::memory_order_release);handled=token;
            }
        }
    }
    void activate(int index) noexcept {
        const int old=active;
        active=index;activeToken=slots[index].token;
        slots[index].state.store(Active,std::memory_order_release);
        condition.reset(slots[index].warmedLevel);condition.set(settings.level,2205);
        gain.reset(gainFor(slots[index].warmedLevel));gain.set(gainFor(settings.level),4410);
        if(old>=0) slots[old].state.store(Free,std::memory_order_release);
        pending=-1;
    }
    void begin() noexcept {
        const auto target=requested.load(std::memory_order_acquire);
        if(pending>=0 && slots[pending].token!=target) {
            slots[pending].state.store(Free,std::memory_order_release);pending=-1;
            modelFade.set(1,fadeSamples);
        }
        for(int i=0;i<3;++i) {
            if(slots[i].state.load(std::memory_order_acquire)!=Ready) continue;
            if(slots[i].token!=target) {slots[i].state.store(Free,std::memory_order_release);continue;}
            if(active<0 || (settings.enabled<.5f && enabled.current==0)) {
                activate(i);modelFade.reset(1);
            } else if(pending!=i) {
                pending=i;modelFade.set(0,fadeSamples);
            }
        }
    }
    float process(float dry) noexcept {
        if(active<0) return dry; // while preparing the first model, keep the dry voice audible
        if(settings.enabled<.5f && enabled.current==0) return dry;
        const float amount=std::clamp(mix.next()+mixModulation,0.0f,1.0f), on=enabled.next(), fade=modelFade.next();
        float wet=slots[active].process(dry,condition.next(),gain.next());
        if(!std::isfinite(wet)) {wet=0;slots[active].dc.reset();}
        // Model fade passes through dry, so the synth remains audible while swapping.
        const float effectiveMix=amount*on*fade;
        float result;
        if(effectiveMix<=0) result=dry;
        else if(effectiveMix>=1) result=wet;
        else {
            if(effectiveMix!=cachedMix) {
                cachedMix=effectiveMix;
                dryGain=std::cos(pi*.5f*effectiveMix);wetGain=std::sin(pi*.5f*effectiveMix);
            }
            result=dryGain*dry+wetGain*wet;
        }
        if(pending>=0 && modelFade.current==0) {
            activate(pending);modelFade.set(1,fadeSamples);
        }
        return result;
    }
    int status() const {
        if(settings.enabled<.5f) return 0;
        const auto target=requested.load(std::memory_order_acquire);
        if(target && failed.load(std::memory_order_acquire)==target) return 3;
        return active>=0 && activeToken==target && pending<0 && modelFade.current==1 ? 2 : 1;
    }
};

GuitarDrive::GuitarDrive():impl(std::make_unique<Impl>()){}
GuitarDrive::~GuitarDrive()=default;
void GuitarDrive::set(int p,float x) {
    if(!std::isfinite(x)) return;
    auto& v=*impl;
    if(p==Model) {
        int model=std::clamp(static_cast<int>(std::round(x)),0,jc303drive::modelCount-1);
        if(model!=v.settings.model) {v.settings.model=model;v.request();}
    } else if(p==Enable) {
        v.settings.enabled=x>=.5f ? 1 : 0;
        v.enabled.set(v.settings.enabled,fadeSamples);
        if(v.settings.enabled && v.requested.load(std::memory_order_acquire)==0) v.request();
    } else if(p==Level) {
        v.settings.level=std::clamp(x,0.0f,1.0f);
        v.warmLevel.store(v.settings.level,std::memory_order_release);
        v.condition.set(v.settings.level,2205);v.gain.set(gainFor(v.settings.level),4410);
    } else if(p==Mix) {
        v.settings.mix=std::clamp(x,0.0f,1.0f);v.mix.set(v.settings.mix,2205);
    }
}
float GuitarDrive::get(int p) const {
    const auto& v=*impl;
    if(p==Enable)return v.settings.enabled;
    if(p==Model)return static_cast<float>(v.settings.model);
    if(p==Level)return v.settings.level;
    if(p==Mix)return v.settings.mix;
    if(p==Status)return static_cast<float>(v.status());
    return 0;
}
int GuitarDrive::display(int p,char* out,int size) const {
    const auto& v=*impl;
    if(p==Model)return std::snprintf(out,size,"%s",jc303drive::models[v.settings.model].name);
    if(p==Level) return jc303drive::models[v.settings.model].inputs==2 ?
        std::snprintf(out,size,"%.0f %%",100*v.settings.level):
        std::snprintf(out,size,"%.1f dB",-18+36*v.settings.level);
    if(p==Mix)return std::snprintf(out,size,"%.0f %%",100*v.settings.mix);
    return 0;
}
void GuitarDrive::beginRender(){impl->begin();}
void GuitarDrive::setMixModulation(float amount)noexcept{impl->mixModulation=std::isfinite(amount)?std::clamp(amount,-1.0f,1.0f):0;}
float GuitarDrive::process(float x) noexcept{return impl->process(x);}
void GuitarDrive::save(std::ostream& out) const {
    const auto& s=impl->settings;
    // Store model identity by filename as well as the fixed parameter order.
    out<<"drive "<<s.enabled<<' '<<jc303drive::models[s.model].file<<' '<<s.level<<' '<<s.mix<<'\n';
}
bool GuitarDrive::parse(std::istream& in,Settings& s) {
    std::string tag,file;
    if(!(in>>tag>>s.enabled>>file>>s.level>>s.mix) || tag!="drive") return false;
    if((s.enabled!=0 && s.enabled!=1) || !std::isfinite(s.level) || !std::isfinite(s.mix) ||
       s.level<0 || s.level>1 || s.mix<0 || s.mix>1) return false;
    s.model=-1;
    for(int i=0;i<jc303drive::modelCount;++i) if(file==jc303drive::models[i].file) s.model=i;
    return s.model>=0;
}
void GuitarDrive::load(const Settings& s) {
    set(Enable,0);set(Level,s.level);set(Mix,s.mix);set(Model,static_cast<float>(s.model));set(Enable,s.enabled);
}
