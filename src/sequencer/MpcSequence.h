#pragma once
#include "AcidSequencer303.h"
#include "HostClock.h"
#include <array>
#include <string>
#include <sstream>

// All methods run under the wrapper's per-instance engine lock.
class MpcSequence {
public:
    static constexpr int first=22, stored=19;
    enum Param { Enable=22,Length,Transpose,Selected,Note,Rest,Accent,Slide,Tie,
                 Generate,Clear,Record,Fill,GenAccent,GenSlide,GenTie,Tones,Low,Range,
                 Current,Status,TileFirst };
    struct Snapshot {
        std::array<float,stored> settings{};
        std::array<StepData303,16> steps{};
    };
private:
    rosic::Open303& voice;
    AcidSequencer303 seq;
    HostClock clock;
    std::array<float,stored> settings{};
    std::array<unsigned,128> held{};
    std::array<bool,NPARAMS> dirty{};
    int lastCurrent=0,lastStatus=0;
    std::function<void()> voiceNote;
    float& value(int p) { return settings[p-first]; }
    float value(int p) const { return settings[p-first]; }
    int selected() const { return std::clamp(static_cast<int>(value(Selected))-1,0,15); }
    static float defaultValue(int p) {
        const auto& q=PARAMS[p];
        return std::round(q.nopts ? q.def*(q.nopts-1) : q.min+(q.max-q.min)*q.def);
    }
    void flush() { held.fill(0); voice.allNotesOff(); }
    void editorChanged() {
        for(int p=Note;p<=Tie;++p) dirty[p]=true;
    }
    void reset(bool play) {
        seq.stop(); flush();
        if(play) seq.start();
    }
    void publish() {
        int current=clock.playing() ? seq.getCurrentStep()+1 : 0;
        int status=value(Enable)<.5f ? 0 : clock.playing() ? 2 : 1;
        if(current!=lastCurrent) { dirty[Current]=true; lastCurrent=current; }
        if(status!=lastStatus) { dirty[Status]=true; lastStatus=status; }
    }
    static bool valid(int p,float x) {
        const auto& q=PARAMS[p];
        const float lo=q.nopts ? 0 : q.min, hi=q.nopts ? q.nopts-1 : q.max;
        return std::isfinite(x) && x>=lo && x<=hi && x==std::round(x);
    }
public:
    explicit MpcSequence(rosic::Open303& core):voice(core) {
        for(int i=0;i<stored;++i) settings[i]=defaultValue(first+i);
        seq.prepare(44100,960);
        seq.onNoteEvent=[this](const Acid303Event& e) {
            const int n=e.note;
            if(e.type==Acid303EventType::NoteOn) {
                if(held[n]++==0) { if(voiceNote) voiceNote(); voice.noteOn(n,e.velocity,0); }
            } else if(held[n] && --held[n]==0) voice.noteOn(n,0,0);
        };
        seq.onAllNotesOff=[this]{flush();};
    }
    bool active() const { return value(Enable)>=.5f && clock.playing(); }
    void onVoiceNote(std::function<void()> callback) { voiceNote=std::move(callback); }
    void cycle(double a,double b,bool enabled) { clock.cycle(a,b,enabled); }
    void begin(double beat,double bpm,bool playing,int frames) {
        if(value(Enable)<.5f) { clock.invalidate(); seq.stop(); publish(); return; }
        clock.begin(beat,bpm,playing,frames,[this](bool p){reset(p);});
        publish();
    }
    void sample() {
        if(!active()) return;
        clock.sample([this](uint32_t tick){seq.tick96(tick);},[this](bool p){reset(p);});
        publish();
    }
    void stop() { clock.invalidate(); seq.stop(); flush(); publish(); }
    bool recordNote(uint8_t note,uint8_t velocity,bool legato) {
        if(value(Record)<.5f) return false;
        seq.recNote(note,velocity>=100,legato);
        value(Selected)=seq.getRecStep()+1;
        dirty[Selected]=true;
        editorChanged();
        return true;
    }
    void recordRest() {
        if(value(Record)<.5f) return;
        seq.recRest(); value(Selected)=seq.getRecStep()+1; dirty[Selected]=true; editorChanged();
    }
    void set(int p,float x) {
        if(p>=TileFirst && p<TileFirst+16) {
            if(x>.5f && selected()!=p-TileFirst) {
                set(Selected,static_cast<float>(p-TileFirst+1)); dirty[Selected]=true;
            }
            return;
        }
        if(p<first || p>=first+stored) return;
        if(p==Generate) {
            if(x>.5f) {
                seq.acidRandomize(value(Fill),value(GenAccent),value(GenSlide),value(GenTie),
                                  value(Tones),value(Low),value(Range));
                editorChanged();
            }
            return;
        }
        if(p==Clear) { if(x>.5f) {seq.clearTrack();editorChanged();} return; }
        if(x==get(p)) return; // host acknowledgements must not restart/edit/notify again
        if(p==Note) seq.setStepData(selected(),static_cast<uint8_t>(x));
        if(p==Rest) seq.setRest(selected(),x>.5f);
        if(p==Accent) seq.setAccent(selected(),x>.5f);
        if(p==Slide) seq.setSlide(selected(),x>.5f);
        if(p==Tie) seq.setTie(selected(),x>.5f);
        value(p)=x;
        switch(p) {
        case Enable: stop(); break;
        case Length: seq.stop(); seq.setTrackLength(static_cast<uint8_t>(x)); clock.invalidate(); break;
        case Transpose: seq.stop(); seq.setTranspose(static_cast<int8_t>(x)); clock.invalidate(); break;
        case Selected: seq.setRecStep(selected()); editorChanged(); break;
        case Record: seq.setRecState(x>.5f); seq.setRecStep(selected()); break;
        default: break;
        }
        publish();
    }
    float get(int p) const {
        const auto& step=seq.getTrackData().step[selected()];
        switch(p) {
        case Note:return step.note;
        case Rest:return step.rest;
        case Accent:return step.accent;
        case Slide:return step.slide;
        case Tie:return step.tie;
        case Generate:case Clear:return 0;
        case Current:return lastCurrent;
        case Status:return lastStatus;
        default:return p>=first && p<first+stored ? value(p) : 0;
        }
    }
    int text(int p,char* out,int size) const {
        if(p>=TileFirst && p<TileFirst+16) {
            const int i=p-TileFirst;
            const auto& step=seq.getTrackData().step[i];
            static const char* notes[]{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
            return step.rest ? std::snprintf(out,size,"%02d REST",i+1) :
                std::snprintf(out,size,"%02d %s%d",i+1,notes[step.note%12],step.note/12-1);
        }
        return std::snprintf(out,size,"%.0f",get(p));
    }
    int display(int p,char* out,int size) const {
        if(p==Note) {
            const int n=static_cast<int>(get(p));
            static const char* notes[]{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
            return std::snprintf(out,size,"%d / %s%d",n,notes[n%12],n/12-1);
        }
        if(p==Current) return lastCurrent ? std::snprintf(out,size,"Step %02d",lastCurrent) : std::snprintf(out,size,"Stopped");
        return 0;
    }
    bool tileOn(int p) const { return p-TileFirst==selected(); }
    int changed(char* out,int size) {
        for(int i=first;i<NPARAMS;++i) if(dirty[i]) {
            dirty[i]=false;
            std::snprintf(out,size,"%s",PARAMS[i].key); return 1;
        }
        return 0;
    }
    void save(std::ostream& out) const {
        for(int i=first;i<first+stored;++i) out<<PARAMS[i].key<<' '<<get(i)<<'\n';
        out<<"pattern\n";
        for(int i=0;i<16;++i) {
            const auto& s=seq.getTrackData().step[i];
            out<<i<<' '<<static_cast<int>(s.note)<<' '<<s.rest<<' '<<s.accent<<' '<<s.slide<<' '<<s.tie<<'\n';
        }
    }
    static bool parse(std::istream& in,Snapshot& snap) {
        for(int i=first;i<first+stored;++i) {
            std::string key;float x;
            if(!(in>>key>>x) || key!=PARAMS[i].key || !valid(i,x)) return false;
            snap.settings[i-first]=x;
        }
        std::string tag;
        if(!(in>>tag) || tag!="pattern") return false;
        for(int i=0;i<16;++i) {
            int index,n,r,a,s,t;
            if(!(in>>index>>n>>r>>a>>s>>t) || index!=i || n<0 || n>127 ||
               r<0||r>1||a<0||a>1||s<0||s>1||t<0||t>1) return false;
            snap.steps[i]={static_cast<uint8_t>(n),r!=0,a!=0,s!=0,t!=0};
        }
        return true;
    }
    void load(const Snapshot& snap) {
        stop(); settings=snap.settings;
        seq.setTrackLength(static_cast<uint8_t>(value(Length)));
        seq.setTranspose(static_cast<int8_t>(value(Transpose)));
        seq.setRecState(value(Record)>.5f); seq.setRecStep(selected());
        for(int i=0;i<16;++i) {
            const auto& s=snap.steps[i];
            seq.setStepData(i,s.note);seq.setRest(i,s.rest);seq.setAccent(i,s.accent);
            seq.setSlide(i,s.slide);seq.setTie(i,s.tie);
        }
        dirty[Selected]=true; editorChanged(); publish();
    }
    void legacyReset() {
        Snapshot snap;
        for(int i=0;i<stored;++i) snap.settings[i]=defaultValue(first+i);
        load(snap);
    }
};
