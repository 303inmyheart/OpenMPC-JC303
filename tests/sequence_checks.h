#pragma once
#include "sequencer/HostClock.h"
#include "sequencer/AcidSequencer303.h"

void clockChecks() {
    // Independent oracle: enumerate quarter-beat note boundaries directly;
    // the implementation clocks at 96 PPQN and must land on the same samples.
    for(bool loop : {false,true}) {
        HostClock clock;
        clock.cycle(.125,.5,loop);
        std::vector<int> actual,expected;
        double previous=-1;
        int absolute=0,resetCount=0;
        constexpr double beatsPerSample=120.0/(60*44100);
        for(int block=0;absolute<88200;++block) {
            const int sizes[]{1,31,96,127,128,257};
            int count=std::min(sizes[block%6],88200-absolute);
            double ppq=absolute*beatsPerSample;
            if(loop) ppq=.125+std::fmod(ppq,.375);
            clock.begin(ppq,120,true,count,[&](bool){++resetCount;});
            for(int i=0;i<count;++i) {
                double beat=(absolute+i)*beatsPerSample;
                if(loop) beat=.125+std::fmod(beat,.375);
                const int grid=static_cast<int>(std::floor(beat*4+1e-9));
                const int old=static_cast<int>(std::floor(previous*4+1e-9));
                if((!loop && absolute+i==0) || (previous>=0 && beat>=previous && grid>old))
                    expected.push_back(absolute+i);
                clock.sample([&](uint32_t tick){if(tick%24==0) actual.push_back(absolute+i);},
                             [&](bool){++resetCount;});
                previous=beat;
            }
            absolute+=count;
        }
        require(actual.size()==expected.size(),"host loop clock lost or duplicated a step");
        for(size_t i=0;i<actual.size();++i)
            require(std::abs(actual[i]-expected[i])<=1,"host step started in the wrong sample/block");
        require(!loop || resetCount>2,"non-aligned host loop did not flush gates");
        const auto before=actual.size();
        clock.begin(0,120,false,128,[&](bool){});
        for(int i=0;i<128;++i) clock.sample([&](uint32_t){actual.push_back(-1);},[](bool){});
        require(before==actual.size(),"sequencer emitted events after host stop");
    }
    HostClock clock;
    int current=0;
    std::vector<int> starts;
    clock.begin(.26,120,true,6000,[](bool){});
    for(;current<6000;++current) clock.sample([&](uint32_t tick){if(tick%24==0) starts.push_back(current);},[](bool){});
    require(starts.size()==1 && std::abs(starts[0]-5292)<=1,"mid-song start must join the next exact step boundary");
    // A brief backwards PPQ jitter must not replay an already emitted tick.
    HostClock jitter;
    std::vector<uint32_t> ticks;
    auto reset=[](bool){};
    jitter.begin(0,120,true,128,reset);
    for(int i=0;i<128;++i) jitter.sample([&](uint32_t t){ticks.push_back(t);},reset);
    jitter.begin(128*120.0/(60*44100)-.0009,120,true,128,reset);
    for(int i=0;i<128;++i) jitter.sample([&](uint32_t t){ticks.push_back(t);},reset);
    require(std::adjacent_find(ticks.begin(),ticks.end(),[](auto a,auto b){return b<=a;})==ticks.end(),
            "host PPQ jitter replayed a tick");
    HostClock changingTempo;
    std::vector<int> tempoActual,tempoExpected;
    double position=.013,previousPosition=position;
    int frame=0;
    for(int block=0;block<200;++block) {
        const double tempos[]{60,180,92,240};
        const double bpm=tempos[(block/9)%4], increment=bpm/(60*44100);
        changingTempo.begin(position,bpm,true,128,reset);
        for(int i=0;i<128;++i,++frame) {
            double beat=position+i*increment;
            if(static_cast<int>(std::floor(beat*4+1e-9))>
               static_cast<int>(std::floor(previousPosition*4+1e-9))) tempoExpected.push_back(frame);
            changingTempo.sample([&](uint32_t tick){if(tick%24==0)tempoActual.push_back(frame);},reset);
            previousPosition=beat;
        }
        position+=128*increment;
    }
    require(tempoActual==tempoExpected,"tempo changes shifted or duplicated sequencer steps");
    std::puts("host clock: variable blocks, non-aligned loops, mid-song start and stop: passed");
}

void patternEventChecks() {
    AcidSequencer303 seq;
    seq.prepare(44100,960);
    seq.setTrackLength(4);
    seq.clearTrack();
    seq.setStepData(0,36);seq.setRest(0,false);seq.setAccent(0,true);seq.setSlide(0,true);
    seq.setStepData(1,43);seq.setRest(1,false);
    seq.setRest(2,true);seq.setTie(2,true);
    struct Event {int tick,note,velocity;};
    std::vector<Event> actual;
    int current=0;
    seq.onNoteEvent=[&](const Acid303Event& e){actual.push_back({current,e.note,e.velocity});};
    seq.start();
    for(;current<192;++current) seq.tick96(current);
    // Aciduino gate model: 12 ticks, slide adds 20; one tied rest adds 24.
    const std::vector<Event> expected{{0,36,127},{24,43,70},{32,36,0},{60,43,0},
                                     {96,36,127},{120,43,70},{128,36,0},{156,43,0}};
    require(actual.size()==expected.size(),"pattern note count differs from the gate model");
    for(size_t i=0;i<actual.size();++i)
        require(actual[i].tick==expected[i].tick && actual[i].note==expected[i].note &&
                actual[i].velocity==expected[i].velocity,"accent/slide/tie gate event differs from the model");
    // A tie from the final note into the first rest must survive pattern wrap.
    seq.stop();seq.clearTrack();seq.setRest(0,true);seq.setTie(0,true);
    seq.setRest(3,false);seq.setStepData(3,48);
    actual.clear();seq.start();
    for(current=0;current<192;++current) seq.tick96(current);
    require(actual.size()>=3 && actual[0].tick==72 && actual[1].tick==108,
            "tie gate did not bridge the last/first pattern step");
    std::puts("pattern event model: accent, slide, rest and ties across wrap: passed");
}

std::unique_ptr<rosic::Open303> defaultSequenceReference() {
    auto v=std::make_unique<rosic::Open303>();
    v->setSampleRate(44100);v->setWaveform(1);v->setTuning(440);v->setCutoff(314);
    v->setResonance(100*static_cast<double>(PARAMS[index("resonance")].def));
    v->setEnvMod(0);v->setDecay(200*std::pow(10,static_cast<double>(PARAMS[index("decay")].def)));
    v->setAccent(100*static_cast<double>(PARAMS[index("accent")].def));v->setVolume(-15);
    v->setAmpDecay(1230);v->setAccentDecay(200);v->setFeedbackHighpass(150);
    v->setNormalAttack(3);v->setSlideTime(60);v->setTanhShaperDrive(36.9);
    v->setFilterType(rosic::FILTER_TEEBEE);v->setFilterDrive(0);v->setPassbandCompensation(.1);
    v->setLfoOn(false);
    return v;
}

void sequencePluginChecks() {
    Plugin p,other;
    p.set("seqClear",1);
    p.units("seqLength",4);
    p.units("seqSelected",1);p.units("seqNote",36);p.set("seqRest",0);
    p.set("seqAccent",1);p.set("seqSlide",1);
    p.units("seqSelected",2);p.units("seqNote",43);p.set("seqRest",0);
    p.units("seqSelected",3);p.set("seqRest",1);p.set("seqTie",1);
    p.set("seqEnabled",1);
    TimeInfo time; time.flags=(1<<1)|(1<<9)|(1<<10);
    activeTime=&time;
    auto reference=defaultSequenceReference();
    int absolute=0;
    double maxError=0;
    for(int block=0;absolute<44100;++block) {
        const int sizes[]{31,128,257,96};
        int count=std::min(sizes[block%4],44100-absolute);
        time.ppq=absolute*120.0/(60*44100);
        const auto audio=p.render(count);
        for(int j=0;j<count;++j) {
            const int s=absolute+j;
            for(int pattern=0;pattern<2;++pattern) {
                const int ticks[]{96*pattern,96*pattern+24,96*pattern+32,96*pattern+60};
                const int notes[]{36,43,36,43}, velocities[]{127,70,0,0};
                for(int e=0;e<4;++e)
                    if(s==static_cast<int>(std::ceil(ticks[e]*44100.0*60/(120*96)-1e-7)))
                        reference->noteOn(notes[e],velocities[e],0);
            }
            const float expected=static_cast<float>(reference->getSample());
            maxError=std::max(maxError,std::abs(static_cast<double>(expected-audio[j])));
        }
        absolute+=count;
    }
    std::printf("sequencer -> VST2 audio reference maximum error %.3g\n",maxError);
    require(maxError<1e-7,"sequencer audio/MIDI ordering or timing differs from reference");
    time.flags=(1<<9)|(1<<10); // Stop must flush the current gate.
    p.render(44100);
    auto tail=p.render(128);
    require(std::all_of(tail.begin(),tail.end(),[](float x){return std::abs(x)<1e-5f;}),"host stop left a hung sequence note");
    activeTime=nullptr;
    void* chunk=nullptr;
    const auto size=p.fx->dispatch(p.fx,23,0,0,&chunk,0);
    std::vector<char> saved(static_cast<char*>(chunk),static_cast<char*>(chunk)+size);
    // End-to-end cycle metadata: a loop end in the middle of an audio block
    // must flush the gate and restart at the loop's musical position.
    Plugin looped;
    looped.fx->dispatch(looped.fx,24,0,size,saved.data(),0);
    auto loopReference=defaultSequenceReference();
    time.flags=(1<<1)|(1<<2)|(1<<9)|(1<<10)|(1<<12);
    time.cycleStart=.125;time.cycleEnd=.5;
    activeTime=&time;
    double previousBeat=.125;
    double loopError=0;
    absolute=0;
    for(int block=0;absolute<33075;++block) {
        const int sizes[]{127,31,257,128};
        int count=std::min(sizes[block%4],33075-absolute);
        time.ppq=.125+std::fmod(absolute*120.0/(60*44100),.375);
        auto audio=looped.render(count);
        for(int i=0;i<count;++i) {
            double beat=.125+std::fmod((absolute+i)*120.0/(60*44100),.375);
            if(beat<previousBeat) loopReference->allNotesOff();
            else if(static_cast<int>(std::floor(beat*4+1e-9))>
                    static_cast<int>(std::floor(previousBeat*4+1e-9))) loopReference->noteOn(43,70,0);
            float expected=static_cast<float>(loopReference->getSample());
            loopError=std::max(loopError,std::abs(static_cast<double>(expected-audio[i])));
            previousBeat=beat;
        }
        absolute+=count;
    }
    activeTime=nullptr;
    std::printf("non-aligned loop -> VST2 audio reference maximum error %.3g\n",loopError);
    require(loopError<1e-7,"loop metadata was not honoured in sequencer audio");
    other.fx->dispatch(other.fx,24,0,size,saved.data(),0);
    for(int step=1;step<=16;++step) {
        p.units("seqSelected",step);other.units("seqSelected",step);
        for(const char* key:{"seqNote","seqRest","seqAccent","seqSlide","seqTie"})
            require(p.fx->get(p.fx,index(key))==other.fx->get(other.fx,index(key)),"project reload lost pattern data");
    }
    HostUi ui;ui.echo=true;activeUi=&ui;
    other.units("seqSelected",5);other.render(128);
    for(int i=0;i<8;++i) other.render(128); // allow the wrapper's staggered tile polling to settle
    int reports=0;for(int n:ui.reports) reports+=n;
    other.render(128);other.render(128);
    int after=0;for(int n:ui.reports) after+=n;
    require(after==reports,"sequencer editor host echoes caused a notification loop");
    activeUi=nullptr;
    Plugin rec;
    rec.units("seqSelected",1);rec.set("seqRecord",1);
    rec.note(0x90,48,120);rec.render(1);rec.note(0x80,48,0);rec.render(1);
    rec.units("seqSelected",1);
    require(std::abs(rec.fx->get(rec.fx,index("seqNote"))-48.0f/127)<1e-6f &&
            rec.fx->get(rec.fx,index("seqAccent"))==1,"MIDI step record failed");
    rec.units("seqSelected",2);
    rec.note(0xb0,64,127);rec.render(1);
    rec.note(0xb0,64,127);rec.render(1);
    require(std::abs(rec.fx->get(rec.fx,index("seqSelected"))-2.0f/15)<1e-6,
            "duplicate sustain-down advanced the record cursor twice");
    rec.set("seqRecord",0);
    rec.units("seqFill",100);rec.units("seqGenAccent",100);rec.units("seqLow",60);rec.units("seqRange",0);
    rec.set("seqGenerate",1);rec.render(128);
    for(int step=1;step<=16;++step) {
        rec.units("seqSelected",step);
        require(rec.fx->get(rec.fx,index("seqRest"))==0 && rec.fx->get(rec.fx,index("seqAccent"))==1,
                "deterministic generator fill/accent limits failed");
    }
    std::puts("sequencer VST2: project pattern round trip, MIDI record and generator: passed");
}

void sequenceRotaryChecks() {
    Plugin p; HostUi ui;ui.echo=true;activeUi=&ui;
    const int id=index("seqSelected");
    float previous=0;
    for(int direction:{1,-1}) {
        int changes=0;
        for(int j=1;j<=128;++j) {
            const float raw=direction>0 ? j/128.0f : 1-j/128.0f;
            p.set("seqSelected",raw);p.render(128);
            const float now=p.fx->get(p.fx,id);
            require(direction>0 ? now>=previous : now<=previous,
                    "step edit cursor reverses during a one-direction knob turn");
            if(now!=previous) ++changes;
            previous=now;
        }
        require(changes==15,"step edit cursor skipped or repeated a step");
    }
    activeUi=nullptr;
    std::puts("16-step edit cursor rotary sweep with host echoes: passed");
}
void sequenceChecks() { clockChecks(); patternEventChecks(); sequencePluginChecks(); sequenceRotaryChecks(); }
