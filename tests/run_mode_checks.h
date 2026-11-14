#pragma once
// Independent sample/event oracles for note-driven clock lifetime, including
// starting inside a block while the host transport is already stopped.
void runModeChecks(){
    using namespace seqparam;
    auto core=std::make_unique<rosic::Open303>();MpcSequence seq(*core);
    seq.set(seqLength64,2);seq.set(MpcSequence::Clear,1);
    for(int n=1;n<=2;++n){seq.set(seqSelected64,n);seq.set(MpcSequence::Note,35+n);seq.set(MpcSequence::Rest,0);}
    seq.set(MpcSequence::Enable,1);
    struct Event{int at,n,v;};std::vector<Event> events;int at=0;
    seq.routeExtended([&](int n,int v,bool,bool,bool,double){events.push_back({at,n,v});});
    auto samples=[&](int n){for(int i=0;i<n;++i,++at)seq.sample();};
    seq.set(runparam::Mode,1);seq.begin(0,120,false,8000);samples(37);
    require(!seq.active() && events.empty(),"gate mode started without a trigger");
    seq.inputNote(72,100);samples(6000);
    std::vector<Event> ons;for(auto e:events)if(e.v)ons.push_back(e);
    require(ons.size()==2 && ons[0].at==37 && ons[0].n==36 && ons[1].n==37 && std::abs(ons[1].at-5549)<=1,"gate trigger offset, pitch or MPC tempo differs from oracle");
    seq.inputNote(60,100);seq.inputNote(72,0);samples(50);require(seq.active(),"releasing one of two notes stopped the gate");
    seq.inputNote(60,0);require(!seq.active(),"last note release did not stop gate");
    events.clear();seq.inputNote(48,100);samples(1);require(!events.empty() && events[0].v && events[0].n==36,"new gate did not restart at step one");
    seq.set(seqLength64,1);samples(1);require(seq.active(),"editing length lost a held trigger");
    seq.stop();seq.set(runparam::Mode,2);events.clear();seq.begin(0,120,false,12000);
    seq.inputNote(84,100);samples(20);seq.inputNote(84,0);samples(6000);require(seq.active(),"note trigger did not latch after release");
    seq.begin(0,120,true,512);samples(512);seq.begin(.03,120,false,512);
    require(!seq.active() && seq.get(runparam::Status)==2,"host Stop did not cancel latched trigger");
    seq.inputNote(60,100);samples(1);require(seq.active(),"trigger could not start while host already stopped");
    seq.stop();seq.set(seqLength64,2);events.clear();at=0;seq.inputNote(60,100);
    for(int block=0;block<50;++block){seq.begin(0,120,false,256);samples(256);}
    ons.clear();for(auto e:events)if(e.v)ons.push_back(e);
    require(ons.size()==3 && ons[0].at==0 && std::abs(ons[1].at-5512.5)<=1 && std::abs(ons[2].at-11025)<=1,"local clock restarted or drifted between host blocks");
    seq.set(runparam::Stop,1);require(!seq.active(),"native sequencer STOP did not clear trigger");seq.set(MpcSequence::Enable,0);require(!seq.inputNote(60,100),"disabled sequencer swallowed live MIDI");
    seq.set(seqTiming,0);require(seq.get(runparam::Mode)==0,"legacy timing retained unsupported start mode");
    // Actual VST MIDI splitting and saved mode: no sound before event offset,
    // and restoring the selection must wait for a new note, never resume a latch.
    for(int mode:{1,2}){
        Plugin p;require(p.fx->dispatch(p.fx,26,index("seqTriggerStop"),0,nullptr,0)==1,"STOP is not exposed as automatable");p.units("engine014",1);p.units("seqStartMode",mode);p.units("seqLength64",1);p.units("seqNote",36);p.units("seqRest",0);p.units("seqEnabled",1);
        TimeInfo time;time.flags=(1<<9)|(1<<10);activeTime=&time;
        p.note(0x90,84,100,137);auto audio=p.render(1024);activeTime=nullptr;
        for(int i=0;i<137;++i)require(audio[i]==0,"note-driven audio preceded trigger offset");
        double energy=0;for(float x:audio)energy+=x*x;require(energy>1e-8,"note-driven VST remained silent");
        std::string saved=chunk(p);Plugin loaded;loadChunk(loaded,saved);require(chunk(loaded)==saved,"start mode did not roundtrip");
        time.flags=(1<<9)|(1<<10);activeTime=&time;auto silence=loaded.render(256);activeTime=nullptr;
        for(float x:silence)require(x==0,"restored project resumed a stale trigger");
        // MPC hardware Stop sends All Notes Off, even with transport stopped.
        time.flags=(1<<9)|(1<<10);activeTime=&time;
        p.note(0xb0,123,0);p.render(128);
        char status[64]{};p.fx->dispatch(p.fx,7,index("seqTriggerStatus"),0,status,0);
        require(std::string(status)=="Waiting for Note","hardware Stop/All Notes Off left note trigger latched");
        p.note(0x90,84,100);p.render(128);p.note(0xb0,120,0);
        auto killed=p.render(128);activeTime=nullptr;for(float x:killed)require(x==0,"hardware panic did not silence note mode");
    }
    std::puts("Host/Gate/Hold/latched Trigger: sample offsets, MPC tempo, no transposition, overlapping notes, host Stop, stopped-host retrigger, state waiting PASS");
}
