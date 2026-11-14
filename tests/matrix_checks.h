#pragma once
void matrixChecks(){
    require(index("seqScaleView11")==587 && index("matrixDest1")==588 && index("seqStartMode")==600,"append-only API moved existing automation");
    auto sound=[](int engine,int mods,int source,int dest,float amount){
        Plugin p;p.units("engine014",engine);p.units("switchModState",mods);p.units("filterModel014",3);p.units("cutoff",.5);
        p.units("matrixSource3",source);p.units("matrixDest3",dest);p.units("matrixAmount3",amount);
        p.note(0x90,60,120);return p.render(4096);
    };
    auto zero=sound(1,1,6,0,0);
    require(zero==sound(1,1,0,0,1),"Off matrix source changed audio");
    require(sound(1,0,6,0,0)==sound(1,0,6,0,1),"matrix ignored MODS bypass");
    require(sound(0,1,6,0,0)==sound(0,1,6,0,1),"matrix changed legacy engine audio");
    auto positive=sound(1,1,6,0,.6),negative=sound(1,1,6,0,-.6);
    require(positive!=zero && negative!=zero && positive!=negative,"bipolar matrix cutoff route disconnected");
    // All selectable source/target combinations must remain finite, including
    // inactive combinations (e.g. LFO rate without an active LFO destination).
    for(int source=0;source<8;++source)for(int dest=0;dest<14;++dest)sound(1,1,source,dest,source%2?-.8f:.8f);
    Plugin p;p.units("engine014",1);p.units("switchModState",1);p.units("lfoDepth",.5);
    p.units("matrixDest1",13);p.units("matrixDest2",2);p.units("matrixAmount2",-.37);
    p.units("matrixSource3",7);p.units("matrixDest3",3);p.units("matrixAmount3",.63);
    p.units("matrixSource4",4);p.units("matrixDest4",4);p.units("matrixAmount4",-.21);
    auto state=chunk(p);Plugin restored;loadChunk(restored,state);require(chunk(restored)==state,"matrix state failed exact roundtrip");
    // A reused instance's cached legacy destination must not override a saved
    // expanded route when configure() restores the legacy parameter first.
    restored.units("lfoDestination",1);loadChunk(restored,state);require(chunk(restored)==state,"restored expanded LFO destination lost to legacy alias");
    for(auto pair:{std::pair<const char*,const char*>{"matrixAmount2 -0.370000005","matrixAmount2 nan"},{"RUNMODE 0","RUNMODE 3"},{"matrixDest1 13","matrixDest1 14"}}){
        auto bad=state;auto pos=bad.find(pair.first);require(pos!=std::string::npos,"matrix malformed fixture missing");bad.replace(pos,std::strlen(pair.first),pair.second);loadChunk(restored,bad);require(chunk(restored)==state,"malformed matrix partially changed project");
    }
    auto old=state;old.replace(0,old.find('\n'),"JC303-MPC-7");old.erase(old.find("MATRIX\n"));loadChunk(restored,old);
    require(restored.fx->get(restored.fx,index("matrixAmount2"))==.5f && restored.fx->get(restored.fx,index("matrixSource3"))==0,"JC7 migration did not reset new routes");
    require(restored.fx->get(restored.fx,index("matrixDest1"))==restored.fx->get(restored.fx,index("lfoDestination"))*2/13,"JC7 migration lost existing LFO destination");
    restored.units("matrixDest1",2);require(restored.fx->get(restored.fx,index("lfoDestination"))==1,"new LFO destination failed old alias feedback");
    restored.units("matrixDest1",13);restored.units("lfoDestination",2);require(std::abs(restored.fx->get(restored.fx,index("matrixDest1"))-2.f/13)<1e-6,"old automation failed expanded LFO destination feedback");
    // Matrix Overdrive changes only the transient wet mix. It must leave the
    // saved base setting untouched and actually reach the neural drive audio.
    Plugin dryRoute,modRoute;
    for(auto* driven:{&dryRoute,&modRoute}){
        driven->units("engine014",1);driven->units("switchModState",1);driven->units("driveEnabled",1);driven->units("driveMix",.3);driven->units("cutoff",.5);
        driven->units("matrixSource3",6);driven->units("matrixDest3",5);
    }
    modRoute.units("matrixAmount3",.6);
    auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(30);
    while(dryRoute.fx->get(dryRoute.fx,index("driveStatus"))<.6f || modRoute.fx->get(modRoute.fx,index("driveStatus"))<.6f){
        require(std::chrono::steady_clock::now()<deadline,"matrix drive preparation timed out");
        dryRoute.render(128);modRoute.render(128);std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    dryRoute.render(4096);modRoute.render(4096);
    dryRoute.note(0x90,48,127);modRoute.note(0x90,48,127);
    auto ordinary=dryRoute.render(4096),modulated=modRoute.render(4096);
    require(ordinary!=modulated,"matrix Overdrive target did not affect drive audio");
    require(std::abs(modRoute.fx->get(modRoute.fx,index("driveMix"))-.3)<1e-6,"Overdrive matrix overwrote base dry/wet");
    std::puts("Matrix: 8 sources x 14 targets finite, bypass/legacy preservation, bipolar routes, Overdrive wet mix, JC8 exact state and JC7 migration PASS");
}
