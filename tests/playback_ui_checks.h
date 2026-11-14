#pragma once
void playbackUiChecks(){
    Plugin p;p.units("engine014",1);p.units("seqTiming",1);p.units("seqLength64",64);p.units("seqClear",1);
    // Stopped instruments may not receive audio blocks. The UI thread must
    // publish canonical cell values even before rendering resumes.
    HostUi idleUi;idleUi.echo=true;activeUi=&idleUi;
    p.units("seqSelected64",16);p.units("seqNote",36);p.units("seqRest",0);idleUi.cached[index("seqOctCell16_3")]=1;p.units("seqOctCell16_3",1);
    std::this_thread::sleep_for(std::chrono::milliseconds(170));
    require(idleUi.cached[index("seqOctCell16_3")]==0,"stopped momentary action stays armed until Play");
    require(std::abs(idleUi.cached[index("seqOctView16")].load()-1)<.001,"stopped octave view needs audio rendering to update");
    require(std::abs(idleUi.cached[index("seqPitchView16")].load()*12-1)<.001,"stopped note view needs audio rendering to update");
    activeUi=nullptr;
    p.units("seqEnabled",1);TimeInfo time;time.flags=(1<<1)|(1<<9)|(1<<10);activeTime=&time;
    auto run=[&](int frames){auto data=p.render(frames);time.ppq+=frames*120.0/(60*44100);return data;};
    run(16384);std::this_thread::sleep_for(std::chrono::milliseconds(40));
    displayUpdates=0;audioCallbacks=0;
    for(int n=0;n<12;++n)run(1024);
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    require(displayUpdates==0,"playhead redraw rebuilt the entire parameter screen");
    HostUi ui;ui.echo=true;activeUi=&ui;
    // Enter all four octave actions during playback, as the native bindings do.
    const char* controls[]{"seqOctCell1_0","seqOctCell2_1","seqOctCell3_2","seqOctCell4_3"};
    const int expected[]{72,60,48,24};
    for(int n=0;n<4;++n){
        p.units("seqSelected64",n+1);p.units("seqNote",36);p.units("seqRest",0);
        p.units(controls[n],1);run(1024);
        require(std::abs(p.fx->get(p.fx,index("seqNote"))*127-expected[n])<.001,"octave action mapped to the wrong MIDI note during Play");
        const int octaveView=index((std::string("seqOctView")+std::to_string(n+1)).c_str());
        const int pitchView=index((std::string("seqPitchView")+std::to_string(n+1)).c_str());
        require(std::abs(ui.cached[octaveView].load()*4-(n+1))<.001,"host cached the wrong octave highlight during Play");
        require(std::abs(ui.cached[pitchView].load()*12-1)<.001,"host lost the visible C note during Play");
        // A cached output value sent back by MPC must not edit or erase a note.
        p.set((std::string("seqPitchView")+std::to_string(n+1)).c_str(),0);
        p.set((std::string("seqOctView")+std::to_string(n+1)).c_str(),0);
        run(1024);
        require(p.fx->get(p.fx,index("seqRest"))==0 && std::abs(p.fx->get(p.fx,index("seqNote"))*127-expected[n])<.001,"host output echo erased a playing step");
    }
    p.units("seqSelected64",4);p.units("seqCell4_2",1);run(1024);
    require(std::abs(p.fx->get(p.fx,index("seqNote"))*127-26)<.001,"touch note in the -1 octave was not retained during Play");
    // Live III can deliver one physical tap as two ON writes 30..140 ms apart.
    // Rendering/idle feedback may already have reset the first ON to zero.
    p.units("seqCell8_7",1);run(1024);
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    p.units("seqCell8_7",1);run(1024);
    require(p.fx->get(p.fx,index("seqRest"))==0,"one physical tap was executed twice and erased its own note");
    auto playing=chunk(p);time.flags&=~(1<<1);run(1024);
    require(chunk(p)==playing,"Stop changed stored sequencer data");
    time.flags|=1<<1;run(1024);require(chunk(p)==playing,"Play reset stored touch edits");
    activeUi=nullptr;
    // Deliberately slow host callbacks must be dispatched away from rendering.
    blockUiCallbacks=true;const int before=totalUiCallbacks;
    p.units("seqSelected64",5);p.units("seqCell5_6",1);
    for(int n=0;n<16;++n){run(128);std::this_thread::sleep_for(std::chrono::milliseconds(2));}
    blockUiCallbacks=false;std::this_thread::sleep_for(std::chrono::milliseconds(50));
    require(totalUiCallbacks>before && audioCallbacks==0,"UI notifications reached the audio-render thread");
    activeTime=nullptr;
    std::puts("Play/Stop UI: touch edits retained, octave +3/+2/+1/-1 = 72/60/48/24, readonly echoes ignored, no per-step global rebuild and no audio-thread UI callbacks PASS");
}
