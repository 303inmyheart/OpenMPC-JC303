// Actual VST programs, saved-state recall and unchanged historical automation.
void factoryChecks() {
    static const char* names[]={"Open303 Clean","Dice Acid","Floppy Drop","Formant Talk",
        "Hoover","Hornet Morph","Mind Bender","Miss Scream","Muff Stomp","Rail Crusher",
        "Ripple Lead","Spicy Devil","Squelch","Stiff Spring","Sub Tracker","Super JP",
        "Vowel Sync","Wasp Sting"};
    require(PARAMS[index("preset")].nopts==12,"old preset automation range changed");
    for(int i=0;i<18;++i) {
        Plugin p(false);
        HostUi ui;ui.echo=true;activeUi=&ui;
        p.units("tuning",.625f);p.units("seqLength",7);p.units("seqNote",43);
        p.units("driveEnabled",1);p.units("driveLevel",.99f);
        p.fx->dispatch(p.fx,2,0,12+i,nullptr,0);
        char name[64]{};p.fx->dispatch(p.fx,5,0,0,name,0);
        require(std::string(name)==names[i],"wrong original factory name");
        require(p.fx->get(p.fx,index("engine014"))==1,"factory selected legacy DSP");
        require(p.fx->get(p.fx,index("tuning"))==.625f,"factory reset tuning");
        require(std::abs(p.fx->get(p.fx,index("seqLength"))-.4f)<1e-6,"factory reset sequence length");
        require(std::abs(p.fx->get(p.fx,index("seqNote"))-43.f/127)<1e-6,"factory reset sequence note");
        require(p.fx->get(p.fx,index("switchModState"))==(i?1.f:0.f),"factory MODS master missing");
        for(int j=603;j<610;++j)require(p.fx->get(p.fx,j)==(i?1.f:0.f),"factory MODS area missing");
        p.note(0x90,48,110);double energy=0;
        for(float x:p.render(4096)){require(std::isfinite(x)&&std::abs(x)<100,"factory audio unbounded");energy+=x*x;}
        require(energy>1e-12,"original factory sound silent");
        p.set("cutoff",.4321f);
        const auto saved=chunk(p);Plugin restored(false);loadChunk(restored,saved);
        require(chunk(restored)==saved,"original factory state failed complete recall");
        restored.fx->dispatch(restored.fx,2,0,12+i,nullptr,0);
        require(chunk(restored)==saved,"host program echo destroyed edited factory sound");
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
        require(p.fx->dispatch(p.fx,3,0,0,nullptr,0)==12+i,"host parameter echo replaced factory sound");
        activeUi=nullptr;
        p.fx->dispatch(p.fx,2,0,0,nullptr,0);
        require(p.fx->dispatch(p.fx,3,0,0,nullptr,0)==0,"Init unreachable after factory sound");
    }
    Plugin p(false);p.fx->dispatch(p.fx,2,0,24,nullptr,0); // Squelch
    char text[64]{};p.fx->dispatch(p.fx,7,index("slideTime"),0,text,0);
    require(std::string(text)=="59 ms","factory Slide lost upstream 2..360ms mapping");
    require(std::abs(p.fx->get(p.fx,index("resTrim014"))-.98f)<1e-5,"factory trim lost normalized-to-gain mapping");
    p.fx->dispatch(p.fx,2,0,20,nullptr,0); // Muff Stomp
    require(p.fx->get(p.fx,index("driveModel"))==1,"Muff Stomp lost model 36");
    require(p.fx->get(p.fx,index("drivePreVca"))==1 && p.fx->get(p.fx,index("driveDisperse"))==.5f,"factory pre-VCA/Disperse missing");
    std::puts("18 original factory programs: names, sound, independent mapping fixtures, sequence/tuning preservation, edited recall/host echoes and legacy preset automation PASS");
}
