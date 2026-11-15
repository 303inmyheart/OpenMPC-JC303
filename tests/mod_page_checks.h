#pragma once
void modPageChecks(){
    {
        Plugin fresh(false);
        require(fresh.fx->get(fresh.fx,index("switchModState"))==0,"new instance MODS master must be off");
        for(int i=603;i<610;++i)require(fresh.fx->get(fresh.fx,i)==0,"new instance submodule must be off");
        fresh.fx->dispatch(fresh.fx,2,0,7,nullptr,0); // Existing LFO factory sound must retain its routing.
        require(fresh.fx->get(fresh.fx,index("switchModState"))==1 && fresh.fx->get(fresh.fx,index("modSlot1Enabled"))==1,"factory LFO preset lost its enabled routing");
        fresh.fx->dispatch(fresh.fx,2,0,0,nullptr,0);
        require(fresh.fx->get(fresh.fx,index("switchModState"))==0,"Init did not reset MODS master");
        for(int i=603;i<610;++i)require(fresh.fx->get(fresh.fx,i)==0,"Init did not reset submodule switches");
        fresh.units("switchModState",1);
        for(int i=603;i<610;++i)fresh.fx->set(fresh.fx,i,(i%2)?1:0);
        auto saved=chunk(fresh);Plugin restored(false);loadChunk(restored,saved);
        require(chunk(restored)==saved,"new off defaults overwrote saved per-page switch settings");
    }

    auto sound=[](const char* page,bool on,const char* knob,float value){
        Plugin p;p.units("engine014",1);p.units("switchModState",1);p.units("cutoff",.45);p.units("envmod",.55);
        p.units(page,on);p.units(knob,value);p.note(0x90,48,127);return p.render(4096);
    };
    for(auto test:{std::pair<const char*,const char*>{"circuitEnabled","normalDecay"},{"oscModsEnabled","oscModel014"},{"filterModsEnabled","filterModel014"}}){
        float amount=std::strcmp(test.second,"normalDecay")?2:.95;
        require(sound(test.first,false,test.second,0)==sound(test.first,false,test.second,amount),"bypassed page still reads its knobs");
        require(sound(test.first,true,test.second,amount)!=sound(test.first,false,test.second,amount),"page switch did not bypass its DSP block");
    }
    auto masterOff=[](float trim){Plugin p;p.units("engine014",1);p.units("switchModState",0);p.units("resTrim014",trim);p.units("filterFm014",trim);p.note(0x90,48,127);return p.render(4096);};
    require(masterOff(.85)==masterOff(1.15),"global MODS master left advanced filter shaping active");
    for(int slot=1;slot<=4;++slot){
        auto route=[&](bool on,float amount){
            Plugin p;p.units("engine014",1);p.units("switchModState",1);p.units("cutoff",.5);
            std::string key="modSlot"+std::to_string(slot)+"Enabled";p.units(key.c_str(),on);
            key="matrixDest"+std::to_string(slot);p.units(key.c_str(),2);
            if(slot>2){key="matrixSource"+std::to_string(slot);p.units(key.c_str(),6);}
            key=slot==1?"lfoDepth":"matrixAmount"+std::to_string(slot);p.units(key.c_str(),amount);
            p.note(0x90,60,127);return p.render(4096);
        };
        require(route(false,0)==route(false,.7),"bypassed matrix slot still modulates audio");
        require(route(true,.7)!=route(false,.7),"matrix slot enable disconnected");
    }
    auto depthRoute=[](bool extra){
        Plugin p;p.units("engine014",1);p.units("switchModState",1);p.units("cutoff",.5);
        p.units("modSlot1Enabled",0);p.units("matrixDest1",2);p.units("matrixSource3",6);p.units("matrixDest3",12);p.units("matrixAmount3",extra?1:0);
        p.note(0x90,60,127);return p.render(4096);
    };
    require(depthRoute(true)==depthRoute(false),"LFO Depth route reactivated a bypassed slot one");
    auto lfo=[](float phase,bool sync,float contour){
        Plugin p;p.units("engine014",1);p.units("switchModState",1);p.units("matrixDest1",2);p.units("lfoDepth",.6);p.units("lfoRate",.65);
        p.units("lfoPhase014",phase);p.units("lfoSync014",sync);p.units("lfoContour014",contour);
        p.note(0x90,48,127);return p.render(4096);
    };
    require(lfo(0,false,0)==lfo(.5,false,0),"start phase changed free-running LFO without key sync");
    require(lfo(0,true,0)!=lfo(.5,true,0),"key-synced start phase did not affect audio");
    require(lfo(0,true,-.6)!=lfo(0,true,.6),"bipolar LFO contour disconnected");
    for(const char* knob:{"accentSweep014","reverseGate014"}){
        require(sound("circuitEnabled",true,knob,0)!=sound("circuitEnabled",true,knob,.8),"new circuit effect disconnected");
        require(sound("circuitEnabled",false,knob,0)==sound("circuitEnabled",false,knob,.8),"circuit bypass left new effect active");
    }
    Plugin p;p.units("engine014",1);p.units("oscModel014",2);p.units("oscModsEnabled",0);p.units("modSlot3Enabled",0);
    p.units("lfoPhase014",.375);p.units("lfoSync014",1);p.units("lfoContour014",-.4);p.units("accentSweep014",.65);p.units("reverseGate014",.5);
    auto state=chunk(p);Plugin loaded;loadChunk(loaded,state);require(chunk(loaded)==state,"page/extension JC9 state failed exact roundtrip");
    require(loaded.fx->get(loaded.fx,index("oscModel014"))==1,"bypass erased saved oscillator choice");
    auto bad=state;bad.replace(bad.find("oscModsEnabled 0"),std::strlen("oscModsEnabled 0"),"oscModsEnabled 2");loadChunk(loaded,bad);require(chunk(loaded)==state,"invalid page state partially mutated project");
    auto jc9=state;jc9.replace(0,jc9.find('\n'),"JC303-MPC-9");jc9.erase(jc9.find("DRIVEFX\n"));loadChunk(loaded,jc9);require(loaded.fx->get(loaded.fx,index("drivePreVca"))==0 && loaded.fx->get(loaded.fx,index("driveDisperse"))==0,"JC9 import enabled new drive effects");
    auto old=state;old.replace(0,old.find('\n'),"JC303-MPC-8");old.erase(old.find("SUBMODS\n"));loadChunk(loaded,old);
    for(int i=603;i<610;++i)require(loaded.fx->get(loaded.fx,i)==1,"old JC8 project did not retain enabled page defaults");
    require(loaded.fx->get(loaded.fx,index("lfoContour014"))==.5f && loaded.fx->get(loaded.fx,index("accentSweep014"))==0,"JC8 migration did not neutralize new effects");
    std::puts("Independent Circuit/Osc/Filter/four-slot bypass, retained values, LFO depth isolation, phase/sync/contour, Accent Sweep/Reverse Gate, JC9 roundtrip and JC8 migration PASS");
}
