#pragma once
#include "drive/Disperser.h"
void driveFxChecks(){
    // Independent direct-form allpass oracle: no vendored filter calls.
    struct Section{float b0,b1;double x1=0,x2=0,y1=0,y2=0;
        float process(float x){double y=b0*x+b1*x1+x2-b1*y1-b0*y2;x2=x1;x1=x;y2=y1;y1=y;return float(y);}};
    for(float amount:{.01f,.35f,1.f}){
        DriveDisperser actual;actual.setAmount(amount);std::array<Section,8> ref;
        for(int i=0;i<8;++i){double f=150*std::pow(20.,amount)*std::pow(2.,-1+2.*i/7),w=2*3.14159265358979323846*f/44100,alpha=std::sin(w)/2;ref[i]={float((1-alpha)/(1+alpha)),float(-2*std::cos(w)/(1+alpha))};}
        double maxError=0,energy=0;
        for(int i=0;i<16384;++i){float x=i==0?1.f:0.f,expected=x;for(auto& stage:ref)expected=stage.process(expected);float y=actual.process(x);maxError=std::max(maxError,double(std::abs(y-expected)));energy+=y*y;}
        require(maxError<.0003,"Disperse differs from independent allpass oracle");require(std::abs(energy-1)<.003,"Disperse changed impulse magnitude/energy");
        actual.setAmount(0);for(int i=0;i<256;++i){float x=.1f*std::sin(i*.13f);require(actual.process(x)==x,"Disperse zero is not bit-exact bypass");}
        actual.setAmount(amount);DriveDisperser fresh;fresh.setAmount(amount);for(int i=0;i<1024;++i){float x=i==0?1.f:0.f;require(actual.process(x)==fresh.process(x),"Disperse reactivation reused stale filter history");}
    }
    // Exercise ready wet networks, not the dry fallback during model loading.
    auto sounds=[](int pre,int model,float disperse){
        Plugin p;p.units("engine014",1);p.units("switchModState",1);p.units("cutoff",.5);p.units("normalDecay",.05);
        p.units("driveEnabled",1);p.units("driveMix",.7);p.units("driveModel",model);p.units("drivePreVca",pre);p.units("driveDisperse",disperse);
        auto until=std::chrono::steady_clock::now()+std::chrono::seconds(30);
        while(p.fx->get(p.fx,index("driveStatus"))<.6f){require(std::chrono::steady_clock::now()<until,"pre-VCA model preparation timed out");p.render(128);std::this_thread::sleep_for(std::chrono::milliseconds(1));}
        p.render(4096);p.note(0x90,48,127);auto audio=p.render(8192);
        p.note(0xb0,120,0);for(float x:p.render(128))require(x==0,"pre-VCA panic did not silence audio");return audio;
    };
    for(int model:{0,4,36}){
        auto post=sounds(0,model,0),pre=sounds(1,model,0),spread=sounds(1,model,.5);
        require(post!=pre && pre!=spread,"Pre-VCA or Disperse signal routing disconnected");
        double en=0;for(float x:pre)en+=x*x;require(en>1e-9 && en<1e8,"pre-VCA signal silent or unbounded");
    }
    // Independent insert-location oracle: in Pre mode volume is AFTER the
    // nonlinear network and therefore scales output exactly. In Post mode the
    // network sees the changed level and must fail that proportionality test.
    auto volumeError=[](int pre){
        Plugin a,b;for(auto*p:{&a,&b}){p->units("engine014",1);p->units("switchModState",1);p->units("driveLevel",1);p->units("driveMix",1);p->units("driveEnabled",1);p->units("drivePreVca",pre);p->units("cutoff",.7);}
        a.units("volume",.7);b.units("volume",.95);
        auto until=std::chrono::steady_clock::now()+std::chrono::seconds(30);
        while(a.fx->get(a.fx,index("driveStatus"))<.6f || b.fx->get(b.fx,index("driveStatus"))<.6f){require(std::chrono::steady_clock::now()<until,"routing oracle model timeout");a.render(128);b.render(128);std::this_thread::sleep_for(std::chrono::milliseconds(1));}
        a.render(4096);b.render(4096);a.note(0x90,48,127);b.note(0x90,48,127);
        auto x=a.render(4096),y=b.render(4096);double error=0,energy=0;
        double ratio=std::pow(10.,3.*(b.fx->get(b.fx,index("volume"))-a.fx->get(a.fx,index("volume"))));
        for(size_t i=0;i<x.size();++i){error+=std::pow(y[i]-ratio*x[i],2);energy+=y[i]*y[i];}
        require(energy>1e-10,"insert-location oracle silent");return std::sqrt(error/energy);
    };
    require(volumeError(1)<.00001 && volumeError(0)>.0001,"Drive is not located before/after VCA as selected");
    // A requested insert must leave the dry legacy/current core unchanged while off.
    for(int modern:{0,1}){
        Plugin a,b;for(auto*p:{&a,&b})p->units("engine014",modern);
        b.units("drivePreVca",1);b.units("driveDisperse",.8);
        a.note(0x90,48,100);b.note(0x90,48,100);require(a.render(2048)==b.render(2048),"disabled new drive changed dry voice");
    }
    Plugin state;state.units("engine014",1);state.units("drivePreVca",1);state.units("driveDisperse",.37);
    auto saved=chunk(state);Plugin loaded;loadChunk(loaded,saved);require(chunk(loaded)==saved,"JC10 drive settings failed exact roundtrip");
    auto bad=saved;bad.replace(bad.find("drivePreVca 1"),std::strlen("drivePreVca 1"),"drivePreVca 2");loadChunk(loaded,bad);require(chunk(loaded)==saved,"malformed drive routing partially mutated state");
    auto old=saved;old.replace(0,old.find('\n'),"JC303-MPC-9");old.erase(old.find("DRIVEFX\n"));loadChunk(loaded,old);
    require(loaded.fx->get(loaded.fx,index("drivePreVca"))==0 && loaded.fx->get(loaded.fx,index("driveDisperse"))==0,"JC9 migration enabled new effects");
    std::puts("Disperse independent allpass/energy/bypass oracle; Pre/Post-VCA actual wet paths, 3 models, panic, dry preservation and JC10/JC9 migration PASS");
}
