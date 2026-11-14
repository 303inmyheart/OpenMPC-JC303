// New engine signal routes and old-project migration through the actual VST.
std::string chunk(Plugin& p){void* ptr=nullptr;auto n=p.fx->dispatch(p.fx,23,0,0,&ptr,0);require(n>0,"missing state");return std::string(static_cast<char*>(ptr));}
void loadChunk(Plugin& p,const std::string& s){p.fx->dispatch(p.fx,24,0,s.size()+1,const_cast<char*>(s.c_str()),0);}
void modernChecks(){
    // The reserved popup flag was implicit at index 65 in every released build.
    require(index("driveModel__open")==65 && index("engine014")==66,"old popup/automation index moved");
    for(int filter=0;filter<3;filter++){
        Plugin original,migrated;original.units("engine014",0);original.units("switchModState",1);
        original.units("filterType",filter);original.set("cutoff",.4);original.set("resonance",.7);
        std::string old=chunk(original);old.replace(0,11,"JC303-MPC-4");old.erase(old.find("engine014 "));
        migrated.units("engine014",1);loadChunk(migrated,old);
        require(migrated.fx->get(migrated.fx,index("engine014"))==0,"old project selected modern DSP");
        original.note(0x90,36,120);migrated.note(0x90,36,120);
        require(original.render(4096)==migrated.render(4096),"legacy migration changed captured audio");
    }
    std::puts("All three old filter modes migrate to legacy with sample-identical audio");
    for(int osc=0;osc<3;osc++)for(int filter=0;filter<8;filter++){
        Plugin p;p.units("engine014",1);p.units("switchModState",1);p.units("oscModel014",osc);p.units("filterModel014",filter);
        p.set("cutoff",.5);p.set("envmod",.4);p.set("resonance",.5);
        p.units("superDetune014",.35);p.units("subGain014",.1);
        p.note(0x90,48,127);auto a=p.render(2048);double en=0;for(float x:a)en+=x*x;
        require(en>1e-8 && en<1e7,"modern filter/oscillator silent or unbounded");
        p.units("filterQuality014",1);p.units("filterMorph014",.5);p.units("filterSaturation014",8);
        p.render(512);p.note(0x80,48,0);p.render(128);
    }
    // Every new oscillator family must make an audible difference on fresh voices.
    std::array<std::vector<float>,3> sounds;
    for(int osc=0;osc<3;osc++){
        Plugin p;p.units("engine014",1);p.units("switchModState",1);p.units("oscModel014",osc);p.units("superDetune014",.5);p.note(0x90,48,127);sounds[osc]=p.render(1024);
    }
    require(sounds[0]!=sounds[1] && sounds[1]!=sounds[2],"oscillator model selector disconnected");
    Plugin p;p.units("engine014",1);p.units("switchModState",1);p.units("filterModel014",6);p.units("oscModel014",2);p.units("filterFm014",.25);p.units("shaperMode014",2);p.units("subGain014",.32);
    std::string saved=chunk(p);require(saved.rfind("JC303-MPC-7",0)==0,"missing versioned modern state");
    Plugin restored;loadChunk(restored,saved);require(chunk(restored)==saved,"modern state did not round trip");
    auto broken=saved;broken.replace(broken.find("filterFm014 "),std::string::npos,"filterFm014 nan\n");loadChunk(restored,broken);require(chunk(restored)==saved,"invalid modern state partially mutated instance");
    loadChunk(restored,saved.substr(0,saved.find("subGain014 ")));require(chunk(restored)==saved,"truncated modern state accepted");
    p.note(0x90,48,127);restored.note(0x90,48,127);require(p.render(2048)==restored.render(2048),"modern state changed captured audio");
    restored.note(0xb0,120,0);auto silence=restored.render(128);for(float x:silence)require(x==0,"modern panic left sound");
    // Existing host sequencer must route its notes into the selected modern core.
    Plugin sequenced;sequenced.units("engine014",1);sequenced.units("switchModState",1);
    sequenced.units("oscModel014",1);sequenced.set("seqClear",1);sequenced.units("seqLength",1);
    sequenced.units("seqSelected",1);sequenced.units("seqNote",43);sequenced.set("seqRest",0);sequenced.set("seqEnabled",1);
    TimeInfo time;time.flags=(1<<1)|(1<<9)|(1<<10);activeTime=&time;
    auto audio=sequenced.render(1024);activeTime=nullptr;double energy=0;for(float x:audio)energy+=x*x;
    require(energy>1e-7,"existing sequencer did not drive modern core");
    sequenced.units("engine014",0);for(float x:sequenced.render(128))require(x==0,"engine switch resumed inactive tail");
    std::puts("24 modern filter/oscillator routes, quality/morph, distinct waveforms, JC7 exact roundtrip and transactional migration PASS");
}
