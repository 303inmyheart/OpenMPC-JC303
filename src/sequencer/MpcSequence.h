#pragma once
#include "LegacyMpcSequence.h"
#include "v014/AcidSequencer303.h"
#include "SeqParams.h"
#include "ScaleGuide.h"
#include "../RunModeParams.h"

// Append-only host adapter. Legacy chunks retain the original event engine;
// the 64-step editor uses the upstream v0.14 event engine under the host lock.
class MpcSequence : public LegacyMpcSequence {
    using Step=jc303seq014::StepData303;
    using Sequencer=jc303seq014::AcidSequencer303;
    Sequencer seq;
    ScaleGuide guide;
    HostClock clock;
    std::array<float,seqparam::seqFollow-seqparam::First+1> cfg{};
    std::array<float,stored> base{};
    std::array<bool,NPARAMS> dirty{};
    std::array<float,NPARAMS> shown{};
    std::array<Step,64> clipboard{};
    std::array<Step,16> pageClipboard{};
    bool haveCopy=false,havePageCopy=false;
    int current=0,status=0;
    int startMode=0,lastTriggerStatus=-1,framesLeft=0;
    std::array<bool,128> triggerHeld{};
    bool latched=false,hostWasPlaying=false;
    double localBeat=0,triggerBpm=120;
    bool armed()const{return latched || std::any_of(triggerHeld.begin(),triggerHeld.end(),[](bool x){return x;});}
    int triggerStatus()const{return get(Enable)<.5f?0:!startMode?1:!armed()?2:startMode==1?3:4;}
    void restartForEdit(){clock.invalidate();seq.stop();vibrating=false;localBeat=0;publish();}
    void startLocalClock(){clock.cycle(0,0,false);clock.begin(localBeat*factor,triggerBpm*factor,true,framesLeft,[this](bool p){reset(p);});}
    double factor=1,samplesPerTick=229.6875,loopA=0,loopB=0;
    bool looping=false,vibrating=false;
    double vibPhase=0,vibValue=0;
    std::function<void()> noteStarted,allOff;
    std::function<void(int,int,bool,bool,bool,double)> routed;
    float& c(int p){return cfg[p-seqparam::First];}
    float c(int p)const{return cfg[p-seqparam::First];}
    bool modern()const{return c(seqparam::seqTiming)>.5f;}
    int selected()const{return int(c(seqparam::seqSelected64))-1;}
    int page()const{return int(c(seqparam::seqPage));}
    static float def(int p){const auto&q=PARAMS[p];return q.nopts?q.def*(q.nopts-1):q.min+(q.max-q.min)*q.def;}
    Step step(int n)const{
        if(modern())return seq.getTrackData().step[n];
        if(n<16){auto s=LegacyMpcSequence::stepAt(n);Step r;r.note=s.note;r.rest=s.rest;r.accent=s.accent;r.slide=s.slide;r.tie=s.tie;return r;}
        return seq.getTrackData().step[n];
    }
    void changedViews(){
        // Notify only actual changes. Repainting every alias for a single cell
        // edit needlessly blocks the host UI and its audio workers.
        auto refresh=[this](int p){float x=get(p);if(x!=shown[p]){shown[p]=x;dirty[p]=true;}};
        for(int p=Note;p<=Tie;++p)refresh(p);
        for(int p=seqparam::seqMute;p<=seqparam::seqVibrato;++p)refresh(p);
        for(int p=seqparam::GateFirst;p<seqparam::NoteActionFirst;++p)refresh(p);
        for(int p=seqparam::SelectedViewFirst;p<seqparam::SelectedViewFirst+16;++p)refresh(p);
        for(int p=seqparam::ScaleViewFirst;p<seqparam::ScaleViewFirst+12;++p)refresh(p);
        dirty[seqparam::seqKeyInfo]=true;
        refresh(seqparam::seqCanUndo);refresh(seqparam::seqCanRedo);
        dirty[seqparam::seqInfo]=true;
    }
    void select(int n,bool notify=true){
        c(seqparam::seqSelected64)=std::clamp(n,0,63)+1;
        c(seqparam::seqPage)=selected()/16;
        if(modern())seq.setRecStep(selected());else LegacyMpcSequence::set(Selected,selected()%16+1);
        dirty[Selected]=true;
        if(notify)dirty[seqparam::seqSelected64]=true;
        dirty[seqparam::seqPage]=true;changedViews();
    }
    void configure(){
        seq.setTrackLength(c(seqparam::seqLength64));seq.setTranspose(base[Transpose-first]);
        seq.setPitchLength(c(seqparam::seqPitchLen));seq.setSwingPercent(c(seqparam::seqSwing));
        seq.gatePercent(c(seqparam::seqGatePct));seq.setReversePlayback(c(seqparam::seqReversePlay)>.5f);
        seq.setCpuTiming(c(seqparam::seqCpuTiming)>.5f);seq.setRecState(base[Record-first]>.5f);
        seq.setRecStep(selected());
        seq.liveOverrides(c(seqparam::seqLiveGate),c(seqparam::seqLiveSlide),c(seqparam::seqLiveAccent),c(seqparam::seqLiveVibrato));
        factor=std::pow(2.0,2-int(c(seqparam::seqScale)));
        if(c(seqparam::seqModifier)==1)factor/=1.5;
        if(c(seqparam::seqModifier)==2)factor*=1.5;
        clock.cycle(loopA*factor,loopB*factor,looping&&!startMode);
    }
    void ensureModern(){
        if(modern())return;
        auto old=LegacyMpcSequence::snapshot();LegacyMpcSequence::stop();base=old.settings;
        for(int n=0;n<16;++n){Step s;s.note=old.steps[n].note;s.rest=old.steps[n].rest;s.accent=old.steps[n].accent;s.slide=old.steps[n].slide;s.tie=old.steps[n].tie;seq.replaceStep(n,s);}
        c(seqparam::seqTiming)=1;c(seqparam::seqLength64)=base[Length-first];
        configure();clock.invalidate();dirty[seqparam::seqTiming]=true;changedViews();
    }
    void publish(){
        int next=active()?seq.getCurrentStep()+1:0;
        int state=base[Enable-first]<.5f?0:clock.playing()?2:1;
        if(current!=next){
            if(current && (current-1)/16==page())dirty[seqparam::PlayViewFirst+(current-1)%16]=true;
            current=next;dirty[Current]=true;
            if(current && c(seqparam::seqFollow)>.5f && (current-1)/16!=page()){
                c(seqparam::seqPage)=(current-1)/16;dirty[seqparam::seqPage]=true;changedViews();
            }
            if(current && (current-1)/16==page())dirty[seqparam::PlayViewFirst+(current-1)%16]=true;
        }
        if(status!=state){status=state;dirty[Status]=true;}
    }
    void reset(bool play){seq.stop();vibrating=false;if(play)seq.start();}
    void editFlag(int n,int kind,bool on){
        auto s=step(n);
        switch(kind){
        case 0:s.rest=!on;s.tie=false;break;
        case 1:s.slide=on;if(on)s.hammer=false;break;
        case 2:s.accent=on;if(on)s.mute=false;break;
        case 3:s.vibrato=on;break;
        case 4:s.tie=on;if(on)s.rest=true;break;
        case 5:s.mute=on;if(on)s.accent=false;break;
        case 6:s.hammer=on;if(on)s.slide=false;break;
        case 7:s.reverse=on;break;
        }
        seq.replaceStep(n,s);
    }
public:
    struct Extended {
        std::array<float,seqparam::seqFollow-seqparam::First+1> settings{};
        std::array<Step,64> steps{};
        ScaleGuide guide;
    };
    explicit MpcSequence(rosic::Open303& voice):LegacyMpcSequence(voice){
        for(int p=seqparam::First;p<=seqparam::seqFollow;++p)c(p)=def(p);
        for(int p=first;p<first+stored;++p)base[p-first]=LegacyMpcSequence::get(p);
        seq.prepare(44100,960);
        for(int i=16;i<64;++i){Step s;s.rest=true;seq.replaceStep(i,s);}
        configure();
        seq.onNoteEvent=[this](const jc303seq014::Acid303Event&e){
            const bool on=e.type==jc303seq014::Acid303EventType::NoteOn;
            if(on){if(noteStarted)noteStarted();vibrating=e.vibrato;}
            if(routed)routed(e.note,on?e.velocity:0,e.mute,e.hammer,e.reverse,e.gateSamples);
        };
        seq.onAllNotesOff=[this]{vibrating=false;if(allOff)allOff();};
    }
    void routeVoice(std::function<void(int,int)> note,std::function<void()> off){
        LegacyMpcSequence::routeVoice(note,off);allOff=off;
        routed=[note](int n,int v,bool,bool,bool,double){note(n,v);};
    }
    void routeExtended(std::function<void(int,int,bool,bool,bool,double)> cb){routed=std::move(cb);}
    void onVoiceNote(std::function<void()> cb){noteStarted=cb;LegacyMpcSequence::onVoiceNote(cb);}
    bool active()const{return modern()?base[Enable-first]>.5f&&clock.playing():LegacyMpcSequence::active();}
    void cycle(double a,double b,bool on){loopA=a;loopB=b;looping=on;clock.cycle(a*factor,b*factor,on&&!startMode);LegacyMpcSequence::cycle(a,b,on);}
    // Trigger notes control the local clock; their pitch never transposes the pattern.
    bool inputNote(int note,int velocity){
        if(!startMode || get(Enable)<.5f)return false;
        const bool wasArmed=armed();
        triggerHeld[note]=velocity>0;
        if(velocity>0 && startMode==2)latched=true;
        if(!wasArmed && armed())restartForEdit();
        else if(wasArmed && !armed())restartForEdit();
        return true;
    }
    void begin(double ppq,double bpm,bool play,int frames){
        if(std::isfinite(bpm)&&bpm>0)triggerBpm=bpm;
        framesLeft=std::max(0,frames);
        if(startMode && hostWasPlaying && !play)stop();
        hostWasPlaying=play;
        if(!modern()){LegacyMpcSequence::begin(ppq,bpm,play,frames);return;}
        if(base[Enable-first]<.5f){clock.invalidate();seq.stop();publish();return;}
        if(std::isfinite(bpm)&&bpm>0){samplesPerTick=44100*60/(bpm*factor*96);seq.clockRate(samplesPerTick);}
        if(startMode){if(armed())startLocalClock();else{clock.invalidate();seq.stop();}}
        else clock.begin(ppq*factor,bpm*factor,play,frames,[this](bool p){reset(p);});
        publish();
    }
    void sample(){
        if(!modern()){LegacyMpcSequence::sample();return;}
        if(startMode && armed() && get(Enable)>.5f && !clock.playing())startLocalClock();
        if(active()){
            clock.sample([this](uint32_t t){seq.tick96(t);},[this](bool p){reset(p);});
            if(startMode)localBeat+=triggerBpm/(60*44100);
        }
        if(framesLeft>0)--framesLeft;
        seq.mpcSample(samplesPerTick);publish();
    }
    double pitchMod(){
        if(vibrating&&active()){
            vibPhase+=c(seqparam::seqVibRate)/44100.0;if(vibPhase>=1)vibPhase-=1;
            const double target=vibPhase<.5?0:c(seqparam::seqVibDepth);
            vibValue+=(target-vibValue)*.003;
        }else{vibValue=0;vibPhase=0;}
        return vibValue;
    }
    void stop(){triggerHeld.fill(false);latched=false;localBeat=0;LegacyMpcSequence::stop();clock.invalidate();seq.stop();vibrating=false;publish();}
    bool recordNote(uint8_t n,uint8_t velocity,bool legato){
        if(!modern()){bool done=LegacyMpcSequence::recordNote(n,velocity,legato);if(done){c(seqparam::seqSelected64)=LegacyMpcSequence::get(Selected);changedViews();}return done;}
        if(base[Record-first]<.5f)return false;
        seq.pushUndo();seq.recNote(n,velocity>=100,legato);select(seq.getRecStep());return true;
    }
    void recordRest(){
        if(!modern()){LegacyMpcSequence::recordRest();c(seqparam::seqSelected64)=LegacyMpcSequence::get(Selected);changedViews();return;}
        if(base[Record-first]>.5f){seq.pushUndo();seq.recRest();select(seq.getRecStep());}
    }
    void set(int p,float x){
        using namespace seqparam;
        if(p==runparam::Mode){
            if(startMode==int(x))return;
            stop();if(x>0)ensureModern();startMode=int(x);configure();dirty[runparam::Mode]=true;return;
        }
        if(p==runparam::Stop){if(x>.5f)stop();return;}
        if(p==runparam::Status)return;
        if(p<First){
            if(p>=TileFirst&&p<TileFirst+16){if(x>.5f)select(page()*16+p-TileFirst);return;}
            if(p<first||p>=first+stored)return;
            if(!modern()){LegacyMpcSequence::set(p,x);if(p==Selected)c(seqparam::seqSelected64)=page()*16+x;changedViews();return;}
            if(p==Selected){if(x!=get(p))select(page()*16+int(x)-1);return;}
            if(p==Length){set(seqLength64,x);return;}
            if(p>=Note&&p<=Tie){
                if(x==get(p))return;
                seq.pushUndo();auto s=step(selected());
                if(p==Note)s.note=x;
                if(p==Rest){s.rest=x>.5f;if(!s.rest)s.tie=false;}
                seq.replaceStep(selected(),s);
                if(p==Accent)editFlag(selected(),2,x>.5f);
                if(p==Slide)editFlag(selected(),1,x>.5f);
                if(p==Tie)editFlag(selected(),4,x>.5f);
                changedViews();return;
            }
            if(p==Generate||p==Clear){
                if(x<.5f)return;
                seq.pushUndo();
                if(p==Generate)seq.acidRandomize(base[Fill-first],base[GenAccent-first],base[GenSlide-first],base[GenTie-first],base[Tones-first],base[Low-first],base[Range-first]);
                else for(int n=0;n<int(c(seqLength64));++n){Step s;s.rest=true;seq.replaceStep(n,s);}
                changedViews();return;
            }
            if(x==base[p-first])return;
            base[p-first]=x;
            if(p==Enable)stop();else if(p==Transpose)restartForEdit();configure();publish();return;
        }
        if(p>=End)return;
        if(p==seqKeyRoot||p==seqKeyScale){
            if(x==get(p))return;
            if(p==seqKeyRoot)guide.root=int(x);else guide.kind=int(x);
            changedViews();return;
        }
        if(p>=SelectedViewFirst)return;
        if(p>=PitchViewFirst&&p<NoteActionFirst)return; // host cannot write views
        if(p==seqInfo||p==seqCanUndo||p==seqCanRedo)return;
        if(p>=NoteActionFirst){
            if(x<.5f)return;
            ensureModern();seq.pushUndo();
            if(p<OctActionFirst){
                int id=p-NoteActionFirst,n=page()*16+id/12,pitch=id%12;auto s=step(n);
                if(!s.rest&&s.note%12==pitch)s.rest=true;
                else{s.note=std::clamp(int(s.note)/12*12+pitch,0,127);s.rest=false;s.tie=false;}
                seq.replaceStep(n,s);select(n);
            }else{
                int id=p-OctActionFirst,n=page()*16+id/4,row=id%4;static constexpr int offsets[]{3,2,1,-1};auto s=step(n);
                int octave=int(c(seqOctBase))+offsets[row];if(int(s.note)/12==octave)octave=c(seqOctBase);
                s.note=std::clamp(octave*12+int(s.note)%12,0,127);seq.replaceStep(n,s);select(n);
            }
            return;
        }
        if(p>=GateFirst){
            if(x==get(p))return;
            ensureModern();seq.pushUndo();int id=p-GateFirst;
            editFlag(page()*16+id%16,id/16,x>.5f);changedViews();return;
        }
        if(p>=seqUndo&&p<=seqTransUp){
            if(x<.5f)return;
            ensureModern();
            if(p==seqUndo)seq.undo();else if(p==seqRedo)seq.redo();
            else if(p==seqCopy){for(int n=0;n<64;++n)clipboard[n]=step(n);haveCopy=true;}
            else if(p==seqCopyPage){for(int n=0;n<16;++n)pageClipboard[n]=step(page()*16+n);havePageCopy=true;}
            else if(p==seqPaste){if(haveCopy){seq.pushUndo();for(int n=0;n<64;++n)seq.replaceStep(n,clipboard[n]);}}
            else if(p==seqPastePage){if(havePageCopy){seq.pushUndo();for(int n=0;n<16;++n)seq.replaceStep(page()*16+n,pageClipboard[n]);}}
            else{
                seq.pushUndo();
                if(p==seqClearPage)for(int n=0;n<16;++n){Step s;s.rest=true;seq.replaceStep(page()*16+n,s);}
                if(p==seqShiftLeft||p==seqShiftRight)seq.shiftSteps(p==seqShiftLeft?1:-1);
                if(p==seqTransDown||p==seqTransUp)seq.transposeNotes(p==seqTransDown?-1:1);
            }
            changedViews();return;
        }
        if(p>=seqMute&&p<=seqVibrato){
            if(x==get(p))return;
            ensureModern();seq.pushUndo();static constexpr int kinds[]{5,6,7,3};editFlag(selected(),kinds[p-seqMute],x>.5f);changedViews();return;
        }
        if(p==seqTiming){
            if(x==get(p))return;
            if(x>.5f)ensureModern();else{
                Snapshot snap;for(int i=0;i<stored;++i)snap.settings[i]=get(first+i);
                snap.settings[Length-first]=std::min(16,int(c(seqLength64)));snap.settings[Selected-first]=selected()%16+1;
                for(int n=0;n<16;++n){auto s=step(n);snap.steps[n]={s.note,s.rest,s.accent,s.slide,s.tie};}
                stop();startMode=0;dirty[runparam::Mode]=true;c(seqTiming)=0;LegacyMpcSequence::load(snap);changedViews();
            }return;
        }
        if(p==seqPage){if(x==c(p))return;c(p)=x;changedViews();return;}
        if(p==seqSelected64){if(x!=get(p))select(int(x)-1,false);return;}
        if(x==get(p))return;
        ensureModern();float old=c(p);c(p)=x;
        if(p==seqShift||p==seqPitchRotate){seq.pushUndo();int delta=int(x-old);if(p==seqShift)seq.shiftSteps(delta);else seq.rotatePitches(delta);changedViews();}
        if(p==seqScale||p==seqModifier||p==seqLength64){restartForEdit();dirty[Length]=true;}
        configure();if(p==seqOctBase)changedViews();
    }
    float get(int p)const{
        using namespace seqparam;
        if(p==runparam::Mode)return startMode;
        if(p==runparam::Status)return triggerStatus();
        if(p==seqTiming)return modern();
        if(p==seqLength64&&!modern())return LegacyMpcSequence::get(Length);
        if(p>=First&&p<=seqFollow){
            if(p>=seqUndo&&p<=seqTransUp)return 0;
            if(p>=seqMute&&p<=seqVibrato){auto s=step(selected());return p==seqMute?s.mute:p==seqHammer?s.hammer:p==seqReverse?s.reverse:s.vibrato;}
            return c(p);
        }
        if(p==seqCanUndo)return seq.canUndo();
        if(p==seqCanRedo)return seq.canRedo();
        if(p==seqKeyInfo)return guide.root*ScaleGuide::Count+guide.kind;
        if(p==seqKeyRoot)return guide.root;
        if(p==seqKeyScale)return guide.kind;
        if(p>=ScaleViewFirst&&p<ScaleViewFirst+12)return guide.includes(p-ScaleViewFirst);
        if(p>=SelectedViewFirst&&p<SelectedViewFirst+16)return selected()==page()*16+p-SelectedViewFirst;
        if(p>=NoteActionFirst)return 0;
        if(p>=GateFirst&&p<NoteActionFirst){
            int col=(p-GateFirst)%16;auto s=step(page()*16+col);
            if(p>=PlayViewFirst)return current==page()*16+col+1&&modern();
            if(p>=OctViewFirst){int d=int(s.note)/12-int(c(seqOctBase));return d==3?1:d==2?2:d==1?3:d==-1?4:0;}
            if(p>=PitchViewFirst)return s.rest?0:s.note%12+1;
            switch((p-GateFirst)/16){case 0:return !s.rest;case 1:return s.slide;case 2:return s.accent;case 3:return s.vibrato;case 4:return s.tie;case 5:return s.mute;case 6:return s.hammer;case 7:return s.reverse;}
        }
        if(!modern())return LegacyMpcSequence::get(p);
        if(p==Length)return std::min(16,int(c(seqLength64)));
        if(p==Selected)return selected()%16+1;
        if(p>=Note&&p<=Tie){auto s=step(selected());switch(p){case Note:return s.note;case Rest:return s.rest;case Accent:return s.accent;case Slide:return s.slide;case Tie:return s.tie;}}
        if(p==Generate||p==Clear)return 0;
        if(p==Current)return current;
        if(p==Status)return status;
        return p>=first&&p<first+stored?base[p-first]:0;
    }
    int text(int p,char*out,int size)const{
        if(p>=TileFirst&&p<TileFirst+16){auto s=step(page()*16+p-TileFirst);return std::snprintf(out,size,"%02d %s",page()*16+p-TileFirst+1,s.rest?"REST":"NOTE");}
        if(p==seqparam::seqInfo)return display(p,out,size);
        return std::snprintf(out,size,"%.9g",get(p));
    }
    int display(int p,char*out,int size)const{
        if(p==seqparam::seqKeyInfo){
            static const char* roots[]{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
            return guide.kind?std::snprintf(out,size,"%s %s",roots[guide.root],ScaleGuide::Names[guide.kind]):std::snprintf(out,size,"Key / Scale Off");
        }
        if(p==seqparam::seqInfo)return std::snprintf(out,size,"STEPS %02d-%02d  |  LENGTH %02d  |  %s",page()*16+1,page()*16+16,int(get(seqparam::seqLength64)),modern()?"NEW 64":"LEGACY");
        if(p==Note){int n=get(p);static const char* names[]{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};return std::snprintf(out,size,"%d / %s%d",n,names[n%12],n/12-1);}
        if(p==Current&&!modern())return LegacyMpcSequence::display(p,out,size);
        if(p==Current)return current?std::snprintf(out,size,"Step %02d",current):std::snprintf(out,size,"Stopped");
        if(p>=seqparam::First){
            if(PARAMS[p].int_display)return std::snprintf(out,size,"%.0f",get(p));
            return std::snprintf(out,size,"%.2f %s",get(p),PARAMS[p].unit);
        }
        return LegacyMpcSequence::display(p,out,size);
    }
    bool tileOn(int p)const{return p>=TileFirst&&p<TileFirst+16&&p-TileFirst==selected()%16;}
    int changed(char*out,int size){
        const int ts=triggerStatus();if(ts!=lastTriggerStatus){lastTriggerStatus=ts;dirty[runparam::Status]=true;}
        for(int p=first;p<NPARAMS;++p)if(dirty[p]){dirty[p]=false;std::snprintf(out,size,"%s",PARAMS[p].key);return 1;}
        return LegacyMpcSequence::changed(out,size);
    }
    void save(std::ostream&out)const{
        for(int p=first;p<first+stored;++p)out<<PARAMS[p].key<<' '<<get(p)<<'\n';
        out<<"pattern\n";for(int n=0;n<16;++n){auto s=step(n);out<<n<<' '<<int(s.note)<<' '<<s.rest<<' '<<s.accent<<' '<<s.slide<<' '<<s.tie<<'\n';}
    }
    void saveExtended(std::ostream&out)const{
        out<<"SEQ64\n";for(int p=seqparam::First;p<=seqparam::seqFollow;++p)out<<PARAMS[p].key<<' '<<get(p)<<'\n';
        for(int n=0;n<64;++n){auto s=step(n);out<<n<<' '<<int(s.note)<<' '<<s.rest<<' '<<s.accent<<' '<<s.slide<<' '<<s.tie<<' '<<s.mute<<' '<<s.hammer<<' '<<s.reverse<<' '<<s.vibrato<<'\n';}
    }
    void saveGuide(std::ostream&out)const{out<<"KEYSCALE "<<guide.root<<' '<<guide.kind<<'\n';}
    static bool parseGuide(std::istream&in,Extended&snap){
        std::string tag;int root,kind;
        if(!(in>>tag>>root>>kind)||tag!="KEYSCALE"||root<0||root>11||kind<0||kind>=ScaleGuide::Count)return false;
        snap.guide.root=root;snap.guide.kind=kind;return true;
    }
    static bool parseExtended(std::istream&in,Extended&snap){
        std::string key;if(!(in>>key)||key!="SEQ64")return false;
        for(int p=seqparam::First;p<=seqparam::seqFollow;++p){float x;const auto&q=PARAMS[p];if(!(in>>key>>x)||key!=q.key||!std::isfinite(x)||x<(q.nopts?0:q.min)||x>(q.nopts?q.nopts-1:q.max)||((q.nopts||q.int_display)&&x!=std::round(x)))return false;snap.settings[p-seqparam::First]=x;}
        for(int n=0;n<64;++n){int index,note,flags[8];if(!(in>>index>>note)||index!=n||note<0||note>127)return false;for(int&f:flags)if(!(in>>f)||f<0||f>1)return false;
            auto&s=snap.steps[n];s.note=note;s.rest=flags[0];s.accent=flags[1];s.slide=flags[2];s.tie=flags[3];s.mute=flags[4];s.hammer=flags[5];s.reverse=flags[6];s.vibrato=flags[7];
            if((s.accent&&s.mute)||(s.slide&&s.hammer))return false;
        }return true;
    }
    void load(const Snapshot&snap){stop();startMode=0;dirty[runparam::Mode]=true;guide=ScaleGuide{};seq.clearHistory();LegacyMpcSequence::load(snap);base=snap.settings;for(int p=seqparam::First;p<=seqparam::seqFollow;++p)c(p)=def(p);c(seqparam::seqTiming)=0;c(seqparam::seqLength64)=snap.settings[Length-first];c(seqparam::seqSelected64)=snap.settings[Selected-first];for(int n=0;n<64;++n){Step s;s.rest=true;seq.replaceStep(n,s);}changedViews();}
    void loadExtended(const Extended&snap){stop();guide=snap.guide;cfg=snap.settings;for(int n=0;n<64;++n)seq.replaceStep(n,snap.steps[n]);configure();changedViews();}
    void legacyReset(){Snapshot s;for(int p=first;p<first+stored;++p)s.settings[p-first]=std::round(def(p));load(s);}
};
