#pragma once
#include "sequencer/MpcSequence.h"

void sequence64Checks(){
    using namespace seqparam;
    auto core=std::make_unique<rosic::Open303>();
    MpcSequence seq(*core);
    require(seq.get(seqTiming)==1,"new Init must use the 64-step sequencer");
    seq.set(seqLength64,64);seq.set(seqPage,3);seq.set(seqSelected64,64);seq.set(MpcSequence::Note,59);
    require(seq.get(seqSelectedView16)==1 && seq.get(seqSelectedView1)==0,"selected-step frame does not match the fourth-page edit cursor");
    seq.set(MpcSequence::Rest,0);seq.set(seqGridAccent16,1);seq.set(seqGridHammer16,1);
    require(seq.get(seqPitchView16)==12 && seq.get(seqGridHammer16)==1,"step 64 or fourth-page alias missing");
    seq.set(seqCell16_0,1);require(seq.get(MpcSequence::Note)==48,"piano-roll pitch must preserve the stored octave");
    seq.set(seqCell16_0,1);require(seq.get(seqPitchView16)==0,"tapping an entered pitch must make a rest");
    seq.set(seqCell16_7,1);require(seq.get(seqPitchView16)==8,"rest-to-note piano entry failed");
    seq.set(seqOctCell16_1,1);require(seq.get(MpcSequence::Note)==67,"+2 octave row is relative to base octave");
    seq.set(seqOctCell16_1,1);require(seq.get(MpcSequence::Note)==43,"tapping an octave twice must return to base");
    seq.set(seqGridSlide16,1);require(seq.get(seqGridHammer16)==0,"slide and hammer must exclude each other");
    seq.set(seqGridMute16,1);require(seq.get(seqGridAccent16)==0,"mute and accent must exclude each other");
    seq.set(seqGridVibrato16,1);seq.set(seqGridReverse16,1);
    std::ostringstream before;seq.saveExtended(before);
    // Generate only the active length, preserve deliberately edited step 64.
    seq.set(seqLength64,7);seq.set(MpcSequence::Generate,1);
    require(seq.get(MpcSequence::Note)==43 && seq.get(seqGridVibrato16)==1 && seq.get(seqGridReverse16)==1,"generator overwrote a step outside its requested length");
    seq.set(seqCopyPage,1);seq.set(seqPage,1);seq.set(seqPastePage,1);
    require(seq.get(seqGridVibrato16)==1 && seq.get(seqPitchView16)==8,"page clipboard lost pitch/attributes");
    seq.set(seqClearPage,1);require(seq.get(seqPitchView16)==0,"clear page failed");
    seq.set(seqUndo,1);require(seq.get(seqPitchView16)==8,"undo did not restore page");
    seq.set(seqRedo,1);require(seq.get(seqPitchView16)==0,"redo did not clear page");
    seq.set(seqPitchView16,12);require(seq.get(seqPitchView16)==0,"read-only pitch view accepted a host write");
    std::ostringstream saved;seq.saveExtended(saved);MpcSequence::Extended restored;
    std::istringstream in(saved.str());require(MpcSequence::parseExtended(in,restored),"new 64-step state rejected");
    auto invalid=saved.str();invalid.replace(invalid.find("seqLength64 "),13,"seqLength64 nan");
    std::istringstream bad(invalid);require(!MpcSequence::parseExtended(bad,restored),"non-finite sequencer settings accepted");
    // Independent sample oracle: a 55% gate is 3031.875 samples at 120 BPM.
    // Shuffle 60% delays odd sixteenths by 1102.5 samples.
    jc303seq014::AcidSequencer303 engine;engine.prepare(44100,960);engine.setTrackLength(64);
    engine.gatePercent(55);engine.setSwingPercent(60);engine.clockRate(229.6875);
    struct Event{int at,note,velocity;bool reverse,hammer;double gate;};std::vector<Event> events;int at=0;
    engine.onNoteEvent=[&](const jc303seq014::Acid303Event&e){events.push_back({at,e.note,e.velocity,e.reverse,e.hammer,e.gateSamples});};
    auto s=engine.getTrackData().step[63];s.note=43;s.reverse=true;engine.replaceStep(63,s);
    engine.start();
    int tick=-1;
    for(at=0;at<360000;++at){int t=int(std::floor(at/229.6875+1e-9));while(tick<t)engine.tick96(++tick);engine.mpcSample(229.6875);}
    std::vector<Event> ons;for(auto e:events)if(e.velocity)ons.push_back(e);
    require(ons.size()==66,"64-step wrap lost or duplicated notes");
    for(size_t n=0;n<ons.size();++n){int expected=int(std::llround(n*5512.5+(n%2?1102.5:0)));require(std::abs(ons[n].at-expected)<=1,"shuffle note onset differs from independent sample grid");require(std::abs(ons[n].gate-3031.875)<.001,"fractional gate length was rounded to clock ticks");}
    require(ons[63].note==43 && ons[63].reverse && ons[64].note==36,"step 64 flags or 64-to-1 wrap missing");
    require(events.size()>1 && events[1].velocity==0 && std::abs(events[1].at-3032)<=1,"fractional note-off has wrong sample position");
    engine.stop();
    // Sub-tick gates must remain shorter than a single 96-PPQN tick.
    engine.gatePercent(1);engine.setSwingPercent(50);events.clear();engine.start();tick=-1;
    for(at=0;at<300;++at){int t=int(std::floor(at/229.6875+1e-9));while(tick<t)engine.tick96(++tick);engine.mpcSample(229.6875);}
    require(events.size()==2 && events[1].velocity==0 && std::abs(events[1].at-55)<=1,"1% gate was clamped to a full clock tick");engine.stop();
    jc303seq014::AcidSequencer303 cpu;cpu.prepare(44100,960);cpu.gatePercent(55);cpu.setCpuTiming(true);cpu.clockRate(229.6875);
    events.clear();cpu.onNoteEvent=[&](const jc303seq014::Acid303Event&e){events.push_back({at,e.note,e.velocity,e.reverse,e.hammer,e.gateSamples});};cpu.start();tick=-1;
    for(at=0;at<5000;++at){int t=int(std::floor(at/229.6875+1e-9));while(tick<t)cpu.tick96(++tick);cpu.mpcSample(229.6875);}
    require(events.size()==2 && events[0].at==44 && events[1].at==3199,"303 CPU timing does not match IRQ/latency sample oracle");cpu.stop();
    // Step scale and dotted/triplet modifiers follow MPC's beat position.
    seq.set(seqLength64,4);seq.set(MpcSequence::Clear,1);
    for(int n=1;n<=4;++n){seq.set(seqSelected64,n);seq.set(MpcSequence::Note,35+n);seq.set(MpcSequence::Rest,0);}
    struct ScaleCase{int scale,modifier;double beats;};
    for(auto setting:{ScaleCase{2,0,.25},ScaleCase{0,0,.0625},ScaleCase{3,1,.75},ScaleCase{2,2,1.0/6}}){
        seq.stop();seq.set(seqScale,setting.scale);seq.set(seqModifier,setting.modifier);seq.set(MpcSequence::Enable,1);
        std::vector<int> starts;int sample=0;
        seq.routeExtended([&](int,int v,bool,bool,bool,double){if(v)starts.push_back(sample);});
        int frames=int(setting.beats*22050*4)+5;seq.begin(0,120,true,frames);
        for(sample=0;sample<frames;++sample)seq.sample();
        require(starts.size()==5,"scaled host clock lost a step");
        for(int n=0;n<5;++n)require(std::abs(starts[n]-n*setting.beats*22050)<=1,"step scale/modifier differs from host beat oracle");
    }
    seq.stop();
    // Through the real host adapter, sweep all 64 edit positions with immediate
    // host echoes. Canonical view updates must not reverse the gesture history.
    {
        Plugin rotary;rotary.units("seqTiming",1);rotary.render(128);
        HostUi ui;ui.echo=true;activeUi=&ui;float prior=0;
        for(int direction:{1,-1})for(int j=1;j<=256;++j){
            float raw=direction>0?j/256.0f:1-j/256.0f;
            rotary.set("seqSelected64",raw);rotary.render(128);
            float value=rotary.fx->get(rotary.fx,index("seqSelected64"));
            require(direction>0?value>=prior:value<=prior,"64-step cursor flickered during monotonic sweep");prior=value;
        }
        activeUi=nullptr;
    }
    // Render the real VST: every added articulation must reach the modern DSP,
    // rather than merely changing an editor flag.
    std::vector<float> ordinary;
    for(const char* flag:{"none","seqMute","seqHammer","seqReverse","seqVibrato"}){
        Plugin voice;voice.units("engine014",1);voice.units("switchModState",1);voice.units("seqTiming",1);
        voice.units("seqLength64",2);voice.units("seqClear",1);
        for(int n=1;n<=2;++n){voice.units("seqSelected64",n);voice.units("seqNote",n==1?36:48);voice.units("seqRest",0);}
        voice.units("seqSelected64",1);voice.units("seqSlide",1);
        if(std::strcmp(flag,"none"))voice.units(flag,1);
        voice.units("seqEnabled",1);TimeInfo time;time.flags=(1<<1)|(1<<9)|(1<<10);activeTime=&time;
        auto audio=voice.render(12000);activeTime=nullptr;
        if(!std::strcmp(flag,"none"))ordinary=audio;else require(audio!=ordinary,"new articulation did not reach the actual modern audio core");
    }
    // Full project serialization, legacy v5 import, malformed extension rollback.
    Plugin plugin;plugin.units("seqTiming",1);plugin.units("seqLength64",64);plugin.units("seqSelected64",64);plugin.units("seqNote",61);
    plugin.units("seqGridVibrato16",1);std::string state=chunk(plugin);Plugin loaded;loadChunk(loaded,state);
    require(chunk(loaded)==state,"full JC6 project failed exact roundtrip");
    auto broken=state.substr(0,state.size()-5);loadChunk(loaded,broken);require(chunk(loaded)==state,"truncated 64-step state partially changed the plugin");
    auto old=state;old.replace(0,11,"JC303-MPC-5");old.erase(old.find("SEQ64\n"));loadChunk(loaded,old);
    require(loaded.fx->get(loaded.fx,index("seqTiming"))==0,"old v5 project did not retain legacy timing");
    std::puts("64-step editor: pages, grid input, exclusive flags, active-only generation, clipboard, undo/redo, fractional swing/gates, wrap, JC6 roundtrip and JC5 migration PASS");
}
