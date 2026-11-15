// JC303 voice adapter. Audio stays in float; the vendored voice computes in double.
// The wrapper serialises all calls per instance. Sequencer timing uses MPC's
// transport; GuitarML remains a separate upcoming milestone.
#include "params.h"
extern "C" {
#include "engine.h"
}
#include "dsp/open303/rosic_Open303.h"
#include "sequencer/MpcSequence.h"
#include "drive/Drive.h"
#include "ModernVoice.h"
#include "MatrixParams.h"
#include "ModSettings.h"
#include "DriveFxParams.h"
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <sstream>
#include <string>

namespace {
constexpr int voiceParams = 22;
enum Index { Wave, Tune, Cutoff, Resonance, EnvMod, Decay, Accent, Volume,
    Mod, NormalDecay, AccentDecay, Feedback, Attack, Slide, Shaper, Filter,
    FilterDrive, Bass, LfoWave, LfoRate, LfoDepth, LfoDest };
static_assert(ENGINE_FLOAT32 == 1, "JC303 requires the float engine ABI");

double linear(double x, double a, double b) { return a + x * (b - a); }
double exponential(double x, double a, double b) { return a * std::pow(b / a, x); }

struct Voice {
    rosic::Open303 core;
    ModernVoice modern;
    std::array<float,ModernVoice::Count> extra{};
    std::array<float,matrixparam::Count> matrix{};
    std::array<float,pageparam::Count> pages{1,1,1,1,1,1,1};
    std::array<float,extmod::Count> extensions{};
    std::array<float,drivefx::Count> driveFx{};
    int lastRoutingStatus=-1,route=0,routeTarget=0;
    float routeGain=1;
    int lastMatrixStatus=-1;
    MpcSequence sequence{core};
    GuitarDrive drive;
    static constexpr int presetParam=64, presetCount=12;
    int preset=0,lastDriveStatus=0;
    std::array<bool,NPARAMS> dirty{};
    std::array<float, voiceParams> values{};
    std::array<bool, 128> held{};
    double lastPitchMod=0;
    double legacyShaperDesired=36.9,legacyShaperApplied=36.9;
    bool silenced = false;
    bool sustainArmed = true;

    Voice() {
        core.setSampleRate(44100);
        resetExtra(true);
        resetMatrix();
        sequence.routeVoice([this](int n,int v){noteOn(n,v);},[this]{core.allNotesOff();modern.allNotesOff();});
        sequence.routeExtended([this](int n,int v,bool m,bool h,bool r,double gate){if(isModern())modern.noteOn(n,v,m,h,r,gate);else core.noteOn(n,v,0);});
        for (int i = 0; i < voiceParams; ++i) values[i] = PARAMS[i].def;
        sequence.onVoiceNote([this]{silenced=false;});
        applyAll();
    }
    bool isModern() const {return extra[0]>.5f;}
    void noteOn(int n,int velocity){if(isModern())modern.noteOn(n,velocity);else core.noteOn(n,velocity,0);}
    void resetExtra(bool enabled){
        for(int j=0;j<ModernVoice::Count;++j){const auto& p=PARAMS[ModernVoice::First+j];extra[j]=p.nopts?p.def*(p.nopts-1):p.min+(p.max-p.min)*p.def;}
        extra[0]=enabled?1:0;
    }
    void resetMatrix(){
        for(int j=0;j<matrixparam::Count;++j){const auto&p=PARAMS[matrixparam::First+j];matrix[j]=p.nopts?p.def*(p.nopts-1):p.min+(p.max-p.min)*p.def;}
    }
    int routingStatus()const{return drive.get(GuitarDrive::Enable)<.5f?3:driveFx[0]<.5f?0:isModern()?1:2;}
    float advanceRoute(){
        constexpr float step=1.0f/441;
        if(route!=routeTarget){routeGain=std::max(0.f,routeGain-step);if(routeGain==0)route=routeTarget;}
        else if(routeGain<1)routeGain=std::min(1.f,routeGain+step);
        return routeGain;
    }
    int matrixStatus()const{return !isModern()?0:values[Mod]<.5f?1:2;}
    void apply(int i) {
        const double x = values[i];
        const bool mod = values[Mod] >= 0.5f && pages[0]>.5f;
        switch (i) {
        case Wave: core.setWaveform(x); break;
        case Tune: core.setTuning(linear(x, 400, 480)); break;
        case Cutoff: core.setCutoff(exponential(x, 314, 2394)); break;
        case Resonance: core.setResonance(100 * x); break;
        case EnvMod: core.setEnvMod(100 * x); break;
        case Decay: core.setDecay(exponential(x, mod ? 30 : 200, mod ? 3000 : 2000)); break;
        case Accent: core.setAccent(100 * x); break;
        case Volume: core.setVolume(linear(x, -60, 0)); break;
        case NormalDecay: core.setAmpDecay(mod ? linear(x, 30, 3000) : 1230); break;
        case AccentDecay: core.setAccentDecay(mod ? linear(x, 30, 3000) : 200); break;
        case Feedback: core.setFeedbackHighpass(mod ? exponential(x, 350, 100) : 150); break;
        case Attack: core.setNormalAttack(mod ? exponential(x, 0.3, 3000) : 3); break;
        case Slide: core.setSlideTime(mod ? linear(x, 2, 1000) : 60); break;
        case Shaper: legacyShaperDesired=mod ? linear(x,25,80) : 36.9; break;
        case Filter: core.setFilterType(mod ? static_cast<rosic::FilterType>(static_cast<int>(x)) : rosic::FILTER_TEEBEE); break;
        case FilterDrive: core.setFilterDrive(mod ? 9 * x : 0); break;
        case Bass: core.setPassbandCompensation(mod ? x : 0.1); break;
        case LfoWave: core.setLfoWaveform(static_cast<int>(x)); break;
        case LfoRate: core.setLfoRate(exponential(x, 0.1, 20)); break;
        case LfoDepth: core.setLfoDepth(x); break;
        case LfoDest: core.setLfoDestination(x); break;
        case Mod:
            core.setLfoOn(values[Mod]>=.5f && pages[3]>.5f);
            apply(Decay);
            for (int j = NormalDecay; j <= Bass; ++j) apply(j);
            break;
        }
    }
    void applyAll() {
        for (int i = 0; i < voiceParams; ++i) if (i != Mod) apply(i);
        core.setLfoOn(values[Mod] >= 0.5f && pages[3]>.5f);
        modern.configure(values.data(),extra.data(),pages.data(),extensions.data());
        modern.configureMatrix(matrix.data(),pages.data());
        drive.setDisperse(driveFx[1]);
    }
    void stop(bool immediate = false) {
        core.allNotesOff();modern.allNotesOff();
        core.setPitchBend(0);modern.pitchBend(0);lastPitchMod=0;
        held.fill(false);
        silenced = immediate;
    }
    void loadPreset(int index) {
        if(index<0 || index>=presetCount || index==preset) return;
        stop();sequence.legacyReset();drive.load({});resetExtra(false);resetMatrix();pages.fill(1);extensions.fill(0);driveFx.fill(0);
        for(int i=0;i<voiceParams;++i) values[i]=PARAMS[i].def;
        switch(index) {
        case 1: values[Wave]=0;values[Cutoff]=.35f;values[EnvMod]=.6f;values[Decay]=.45f;break;
        case 2: values[Wave]=1;values[Cutoff]=.25f;values[Resonance]=.65f;values[EnvMod]=.65f;break;
        case 3: values[Wave]=1;values[Cutoff]=0;values[Resonance]=.12f;values[EnvMod]=0;values[Volume]=.8f;break;
        case 4: values[Wave]=0;values[Decay]=0;values[EnvMod]=.8f;values[Resonance]=.7f;break;
        case 5: values[Wave]=0;values[Cutoff]=.65f;values[Resonance]=.97f;values[EnvMod]=.65f;break;
        case 6: values[Mod]=1;values[Filter]=1;values[FilterDrive]=.8f;values[Cutoff]=.4f;values[Resonance]=.8f;break;
        case 7: values[Mod]=1;values[LfoDepth]=.8f;values[LfoRate]=.45f;values[Cutoff]=.4f;break;
        case 8: values[Wave]=0;values[Cutoff]=.4f;values[EnvMod]=.5f;drive.set(GuitarDrive::Enable,1);break;
        case 9: values[Wave]=1;values[Cutoff]=.3f;values[Resonance]=.65f;
            drive.set(GuitarDrive::Model,4);drive.set(GuitarDrive::Level,.45f);
            drive.set(GuitarDrive::Mix,.5f);drive.set(GuitarDrive::Enable,1);break;
        case 10:case 11:
            values[Wave]=0;values[Cutoff]=.25f;values[EnvMod]=.65f;values[Resonance]=.85f;
            sequence.set(MpcSequence::Length,16);
            for(int i=0;i<16;++i) {
                sequence.set(MpcSequence::Selected,static_cast<float>(i+1));
                const int notes[]{36,36,43,36,48,36,46,43,36,39,43,46,48,43,39,36};
                sequence.set(MpcSequence::Note,static_cast<float>(notes[i]));
                sequence.set(MpcSequence::Rest,(i%4==3 && index==10) ? 1 : 0);
                sequence.set(MpcSequence::Accent,i%4==0 ? 1 : 0);
                sequence.set(MpcSequence::Slide,index==11 && i%2==0 ? 1 : 0);
            }
            sequence.set(MpcSequence::Selected,1);sequence.set(MpcSequence::Enable,1);break;
        default:break;
        }
        applyAll();preset=index;
        for(int j=pageparam::First;j<drivefx::End;++j)dirty[j]=true;
        for(int j=matrixparam::First;j<matrixparam::First+matrixparam::Count;++j)dirty[j]=true;
        for(int j=ModernVoice::First;j<ModernVoice::First+ModernVoice::Count;++j)dirty[j]=true;
        for(int i=0;i<=presetParam;++i)
            if(!PARAMS[i].string_display && !PARAMS[i].momentary && i!=GuitarDrive::Status) dirty[i]=true;
    }
    int changed(char* key,int size) {
        const int rs=routingStatus();if(rs!=lastRoutingStatus){lastRoutingStatus=rs;dirty[drivefx::Status]=true;}
        const int ms=matrixStatus();if(ms!=lastMatrixStatus){lastMatrixStatus=ms;dirty[matrixparam::matrixStatus]=true;}
        const int status=static_cast<int>(drive.get(GuitarDrive::Status));
        if(status!=lastDriveStatus) {lastDriveStatus=status;dirty[GuitarDrive::Status]=true;}
        for(int i=0;i<NPARAMS;++i) if(dirty[i]) {
            dirty[i]=false;std::snprintf(key,size,"%s",PARAMS[i].key);return 1;
        }
        return sequence.changed(key,size);
    }
    void restore(const char* text) {
        // Parse into a separate array so a malformed/truncated chunk changes nothing.
        std::istringstream input(text);
        std::string version;
        input >> version;
        if (version != "JC303-MPC-1" && version != "JC303-MPC-2" && version != "JC303-MPC-3" && version != "JC303-MPC-4" && version != "JC303-MPC-5" && version != "JC303-MPC-6" && version != "JC303-MPC-7" && version != "JC303-MPC-8" && version != "JC303-MPC-9" && version != "JC303-MPC-10") return;
        const int stateVersion=version=="JC303-MPC-10"?10:version.back()-'0';
        auto restored = values;
        for (int i = 0; i < voiceParams; ++i) {
            std::string key;
            float value;
            if (!(input >> key >> value) || key != PARAMS[i].key || !valid(i, value)) return;
            restored[i] = value;
        }
        MpcSequence::Snapshot pattern;
        const bool hasSequence=stateVersion>=3;
        if(hasSequence && !MpcSequence::parse(input,pattern)) return;
        GuitarDrive::Settings driveSettings;
        int restoredPreset=0;
        if(stateVersion>=4) {
            std::string tag;
            if(!GuitarDrive::parse(input,driveSettings) || !(input>>tag>>restoredPreset) ||
               tag!="preset" || restoredPreset<0 || restoredPreset>=presetCount) return;
        }
        std::array<float,ModernVoice::Count> restoredExtra{};
        for(int j=0;j<ModernVoice::Count;++j){const auto& p=PARAMS[ModernVoice::First+j];restoredExtra[j]=p.nopts?p.def*(p.nopts-1):p.min+(p.max-p.min)*p.def;}
        restoredExtra[0]=0; // Old chunks select the unchanged legacy engine.
        if(stateVersion>=5) {
            std::string key;for(int j=0;j<ModernVoice::Count;++j)
                if(!(input>>key>>restoredExtra[j]) || key!=PARAMS[ModernVoice::First+j].key || !valid(ModernVoice::First+j,restoredExtra[j]))return;
        }
        MpcSequence::Extended extended;
        if(stateVersion>=6 && !MpcSequence::parseExtended(input,extended))return;
        if(stateVersion>=7 && !MpcSequence::parseGuide(input,extended))return;
        std::array<float,matrixparam::Count> restoredMatrix{};
        restoredMatrix[0]=restored[LfoDest];
        int startMode=0;
        if(stateVersion>=8){
            std::string tag,key;if(!(input>>tag)||tag!="MATRIX")return;
            for(int j=0;j<matrixparam::Count;++j)if(!(input>>key>>restoredMatrix[j]) || key!=PARAMS[matrixparam::First+j].key || !valid(matrixparam::First+j,restoredMatrix[j]))return;
            if(!(input>>tag>>startMode)||tag!="RUNMODE"||startMode<0||startMode>2)return;
            if(startMode>0 && extended.settings[0]<.5f)return;
        }
        std::array<float,pageparam::Count> restoredPages{1,1,1,1,1,1,1};
        std::array<float,extmod::Count> restoredExtensions{};
        if(stateVersion>=9){
            std::string tag,key;if(!(input>>tag)||tag!="SUBMODS")return;
            for(int j=0;j<pageparam::Count;++j)if(!(input>>key>>restoredPages[j])||key!=PARAMS[pageparam::First+j].key||!valid(pageparam::First+j,restoredPages[j]))return;
            if(!(input>>tag)||tag!="EXTMODS")return;
            for(int j=0;j<extmod::Count;++j)if(!(input>>key>>restoredExtensions[j])||key!=PARAMS[extmod::First+j].key||!valid(extmod::First+j,restoredExtensions[j]))return;
        }
        std::array<float,drivefx::Count> restoredDriveFx{};
        if(stateVersion>=10){
            std::string tag,key;if(!(input>>tag)||tag!="DRIVEFX")return;
            for(int j=0;j<drivefx::Count;++j)if(!(input>>key>>restoredDriveFx[j])||key!=PARAMS[drivefx::First+j].key||!valid(drivefx::First+j,restoredDriveFx[j]))return;
        }
        std::string trailing;
        if (input >> trailing) return;
        if (version == "JC303-MPC-1") restored[Slide] *= 358.0f/998.0f;
        stop();
        if(hasSequence) sequence.load(pattern); else sequence.legacyReset();
        if(stateVersion>=6)sequence.loadExtended(extended);
        drive.load(driveSettings);preset=restoredPreset;
        values = restored;extra=restoredExtra;matrix=restoredMatrix;pages=restoredPages;extensions=restoredExtensions;driveFx=restoredDriveFx;route=routeTarget=isModern()&&driveFx[0]>.5f;routeGain=1;
        sequence.set(runparam::Mode,startMode);
        applyAll();
    }
    static bool valid(int i, float x) {
        const auto& p = PARAMS[i];
        return std::isfinite(x) && x >= (p.nopts ? 0 : p.min) && x <= (p.nopts ? p.nopts - 1 : p.max) &&
               (!p.nopts || x == std::round(x));
    }
};

// Immutable, allocation-free lookup. The wrapper supplies string keys for
// every numeric read/write; a linear scan multiplied the cost of large edits.
constexpr unsigned keySlots=2048;
unsigned keyHash(const char* key){unsigned h=2166136261u;for(;*key;++key){h^=static_cast<unsigned char>(*key);h*=16777619u;}return h;}
const std::array<int,keySlots>& parameterIndex(){
    static const auto table=[](){
        std::array<int,keySlots> result{};result.fill(-1);
        for(int i=0;i<NPARAMS;++i){unsigned slot=keyHash(PARAMS[i].key)&(keySlots-1);while(result[slot]>=0)slot=(slot+1)&(keySlots-1);result[slot]=i;}
        return result;
    }();
    return table;
}
int indexOf(const char* key) {
    const auto& table=parameterIndex();unsigned slot=keyHash(key)&(keySlots-1);
    for(unsigned probe=0;probe<keySlots;++probe){int i=table[slot];if(i<0)return -1;if(!std::strcmp(key,PARAMS[i].key))return i;slot=(slot+1)&(keySlots-1);}
    return -1;
}
void* create(const char*) {
    (void)parameterIndex(); // construct lookup before any real-time call
    try { return new Voice; } catch (...) { return nullptr; }
}
void destroy(void* instance) { delete static_cast<Voice*>(instance); }
void midi(void* instance, const uint8_t* msg, int length) {
    if (!instance || !msg || length < 3) return;
    auto& v = *static_cast<Voice*>(instance);
    const int status = msg[0] & 0xf0, note = msg[1] & 127, velocity = msg[2] & 127;
    if (status == 0x90 && velocity) {
        const bool legato=std::any_of(v.held.begin(),v.held.end(),[](bool x){return x;});
        v.sequence.recordNote(note,velocity,legato);
        if(v.sequence.inputNote(note,velocity) || v.sequence.active()) { v.held[note]=true; return; }
        // Repeated note-ons replace their old stack entry, avoiding stuck duplicates.
        if (v.held[note]) v.noteOn(note, 0);
        v.held[note] = true;
        v.silenced = false;
        v.noteOn(note, velocity); // third argument is detune, NOT a slide flag
    } else if (status == 0x80 || (status == 0x90 && !velocity)) {
        v.held[note] = false;
        if(v.sequence.inputNote(note,0) || v.sequence.active()) return;
        v.noteOn(note, 0);
    } else if (status == 0xb0 && (note == 120 || note == 123)) {
        v.sequence.stop();
        v.stop(note == 120);
    } else if(status==0xb0 && note==64) {
        if(velocity>=64 && v.sustainArmed) { v.sequence.recordRest(); v.sustainArmed=false; }
        if(velocity<64) v.sustainArmed=true;
    }
}
void set(void* instance, const char* key, const char* text) {
    auto& v = *static_cast<Voice*>(instance);
    if (!std::strcmp(key, "state")) { v.restore(text); return; }
    if (!std::strcmp(key, "transport")) { if (text[0] == '0') {v.sequence.stop();v.stop();} return; }
    const int i = indexOf(key);
    if (i < 0) return;
    char* end = nullptr;
    float x = std::strtof(text, &end);
    if (end == text || *end || !std::isfinite(x)) return;
    x = std::max(PARAMS[i].nopts ? 0.0f : PARAMS[i].min,
                 std::min(PARAMS[i].nopts ? static_cast<float>(PARAMS[i].nopts - 1) : PARAMS[i].max, x));
    if (PARAMS[i].nopts) x = std::round(x);
    if (PARAMS[i].int_display) x=std::round(x);
    if(i>=drivefx::First && i<drivefx::First+drivefx::Count){v.driveFx[i-drivefx::First]=x;if(i==drivefx::Disperse)v.drive.setDisperse(x);return;}
    if(i==drivefx::Status)return;
    if(i>=pageparam::First && i<pageparam::End){if(v.pages[i-pageparam::First]!=x){v.pages[i-pageparam::First]=x;v.applyAll();}return;}
    if(i>=extmod::First && i<extmod::End){v.extensions[i-extmod::First]=x;v.modern.configure(v.values.data(),v.extra.data(),v.pages.data(),v.extensions.data());return;}
    if(i>=matrixparam::First && i<matrixparam::First+matrixparam::Count){
        if(v.matrix[i-matrixparam::First]==x)return;
        v.matrix[i-matrixparam::First]=x;
        if(i==matrixparam::matrixDest1 && x<=2){v.values[LfoDest]=x;v.apply(LfoDest);v.dirty[LfoDest]=true;}
        v.modern.configureMatrix(v.matrix.data(),v.pages.data());return;
    }
    if(i>=matrixparam::matrixStatus && i<matrixparam::End && i!=runparam::Mode)return;
    if(i==runparam::Mode){if(x!=v.sequence.get(i)){v.sequence.set(i,x);v.stop(true);}return;}
    if(i>=ModernVoice::First && i<ModernVoice::First+ModernVoice::Count){
        if(i==ModernVoice::Engine && x!=v.extra[0]){v.sequence.stop();v.stop(true);}
        v.extra[i-ModernVoice::First]=x;v.modern.configure(v.values.data(),v.extra.data(),v.pages.data(),v.extensions.data());return;
    }
    if(i==Voice::presetParam) {v.loadPreset(static_cast<int>(x));return;}
    if(i>=GuitarDrive::First && i<=GuitarDrive::Status) {v.drive.set(i,x);return;}
    if(i>=voiceParams) { v.sequence.set(i,x); return; }
    if(i==LfoDest){
        if(x==v.values[i] && x==v.matrix[0])return;
        v.values[i]=x;v.matrix[0]=x;v.apply(i);v.dirty[matrixparam::matrixDest1]=true;
        v.modern.configure(v.values.data(),v.extra.data(),v.pages.data(),v.extensions.data());v.modern.configureMatrix(v.matrix.data(),v.pages.data());return;
    }
    if (x == v.values[i]) return;
    v.values[i] = x;
    v.apply(i);
    v.modern.configure(v.values.data(),v.extra.data(),v.pages.data(),v.extensions.data());
}
int get(void* instance, const char* key, char* out, int size) {
    if (!out || size < 1) return 0;
    auto& v = *static_cast<Voice*>(instance);
    if (!std::strcmp(key, "state")) {
        std::ostringstream state;
        state.precision(9);
        state << "JC303-MPC-10\n";
        for (int i = 0; i < voiceParams; ++i) state << PARAMS[i].key << ' ' << v.values[i] << '\n';
        v.sequence.save(state);
        v.drive.save(state);
        state<<"preset "<<v.preset<<'\n';
        for(int j=0;j<ModernVoice::Count;++j)state<<PARAMS[ModernVoice::First+j].key<<' '<<v.extra[j]<<'\n';
        v.sequence.saveExtended(state);
        v.sequence.saveGuide(state);
        state<<"MATRIX\n";
        for(int j=0;j<matrixparam::Count;++j)state<<PARAMS[matrixparam::First+j].key<<' '<<v.matrix[j]<<'\n';
        state<<"RUNMODE "<<v.sequence.get(runparam::Mode)<<'\n';
        state<<"SUBMODS\n";
        for(int j=0;j<pageparam::Count;++j)state<<PARAMS[pageparam::First+j].key<<' '<<v.pages[j]<<'\n';
        state<<"EXTMODS\n";
        for(int j=0;j<extmod::Count;++j)state<<PARAMS[extmod::First+j].key<<' '<<v.extensions[j]<<'\n';
        state<<"DRIVEFX\n";
        for(int j=0;j<drivefx::Count;++j)state<<PARAMS[drivefx::First+j].key<<' '<<v.driveFx[j]<<'\n';
        const auto text = state.str();
        if (text.size() >= static_cast<size_t>(size)) return 0;
        std::memcpy(out, text.c_str(), text.size() + 1);
        return static_cast<int>(text.size());
    }
    const int i = indexOf(key);
    if(i>=drivefx::First && i<drivefx::First+drivefx::Count)return std::snprintf(out,size,"%.9g",v.driveFx[i-drivefx::First]);
    if(i==drivefx::Status)return std::snprintf(out,size,"%d",v.routingStatus());
    if(i>=pageparam::First && i<pageparam::End)return std::snprintf(out,size,"%.9g",v.pages[i-pageparam::First]);
    if(i>=extmod::First && i<extmod::End)return std::snprintf(out,size,"%.9g",v.extensions[i-extmod::First]);
    if(i>=matrixparam::First && i<matrixparam::First+matrixparam::Count)return std::snprintf(out,size,"%.9g",v.matrix[i-matrixparam::First]);
    if(i==matrixparam::matrixStatus)return std::snprintf(out,size,"%d",v.matrixStatus());
    if(i==matrixparam::matrixSource1)return std::snprintf(out,size,"LFO (fixed)");
    if(i==matrixparam::matrixSource2)return std::snprintf(out,size,"Env (fixed)");
    if(i>=ModernVoice::First && i<ModernVoice::First+ModernVoice::Count)return std::snprintf(out,size,"%.9g",v.extra[i-ModernVoice::First]);
    if(i==Voice::presetParam) return std::snprintf(out,size,"%d",v.preset);
    if(i>=GuitarDrive::First && i<=GuitarDrive::Status) return std::snprintf(out,size,"%.9g",v.drive.get(i));
    if (i >= voiceParams) return v.sequence.text(i,out,size);
    if (i >= 0) return std::snprintf(out, size, "%.9g", v.values[i]);
    const std::string name(key);
    if(name.size()>3 && name.substr(name.size()-3)=="_on") {
        const int j=indexOf(name.substr(0,name.size()-3).c_str());
        if(j>=MpcSequence::TileFirst) return std::snprintf(out,size,"%d",v.sequence.tileOn(j));
    }
    if (name.size() > 8 && name.substr(name.size() - 8) == "_display") {
        const int j = indexOf(name.substr(0, name.size() - 8).c_str());
        if (j < 0) return 0;
        if(j==drivefx::Disperse)return v.driveFx[1]<=0?std::snprintf(out,size,"Off"):std::snprintf(out,size,"%.0f Hz",150.0*std::pow(20.0,v.driveFx[1]));
        if(j>=extmod::First && j<extmod::End){
            float x=v.extensions[j-extmod::First];
            if(j==extmod::First)return std::snprintf(out,size,"%.1f deg",x*360);
            if(j==extmod::First+3)return std::snprintf(out,size,"%.1f ms",15.0*std::pow(2000.0/15.0,x));
            return std::snprintf(out,size,"%+.1f %%",100*x);
        }
        if(j>=matrixparam::First && j<matrixparam::First+matrixparam::Count)return std::snprintf(out,size,"%+.1f %%",100*v.matrix[j-matrixparam::First]);
        if(j>=ModernVoice::First && j<ModernVoice::First+ModernVoice::Count){
            float x=v.extra[j-ModernVoice::First];
            if(j==ModernVoice::Saturation)return std::snprintf(out,size,"+%.1f dB",x);
            if(j==ModernVoice::Trim)return std::snprintf(out,size,"%.2f x",x);
            return std::snprintf(out,size,"%.1f %%",100*x);
        }
        if(j>=GuitarDrive::First && j<=GuitarDrive::Status) return v.drive.display(j,out,size);
        if(j>=voiceParams) return v.sequence.display(j,out,size);
        const double x = v.values[j];
        switch (j) {
        case Wave: return std::snprintf(out, size, "%.0f %%", 100*x);
        case Tune: return std::snprintf(out, size, "%.1f Hz", linear(x, 400, 480));
        case Cutoff: return std::snprintf(out, size, "%.0f Hz", exponential(x, 314, 2394));
        case Decay: return std::snprintf(out, size, "%.0f ms", exponential(x, v.values[Mod] >= .5 && v.pages[0]>.5 ? 30 : 200, v.values[Mod] >= .5 && v.pages[0]>.5 ? 3000 : 2000));
        case Volume: return std::snprintf(out, size, "%.1f dB", linear(x, -60, 0));
        case NormalDecay: case AccentDecay: return std::snprintf(out, size, "%.0f ms", linear(x, 30, 3000));
        case Feedback: return std::snprintf(out, size, "%.0f Hz", exponential(x, 350, 100));
        case Attack: return std::snprintf(out, size, "%.1f ms", exponential(x, .3, 3000));
        case Slide: return std::snprintf(out, size, "%.0f ms", linear(x, 2, 1000));
        case Shaper: return std::snprintf(out, size, "%.1f dB", linear(x, 25, 80));
        case FilterDrive:
            if (v.values[Mod] < .5f) return std::snprintf(out, size, "Mods off");
            if (static_cast<int>(v.values[Filter]) == rosic::FILTER_TEEBEE) return std::snprintf(out, size, "Diode only");
            return std::snprintf(out, size, "%.1f dB", 9*x);
        case LfoRate: return std::snprintf(out, size, "%.2f Hz", exponential(x, .1, 20));
        default: return std::snprintf(out, size, "%.0f %%", 100*x);
        }
    }
    return 0;
}
void render(void* instance, mpc_sample_t* out, int frames) {
    auto& v = *static_cast<Voice*>(instance);
    v.drive.setMixModulation(v.isModern()&&v.values[Mod]>=.5f?static_cast<float>(v.modern.driveModulation()):0.0f);
    // Only the active voice's final table is needed. All intermediate parameter
    // writes before this block remain visible through the unchanged saved values.
    if(!v.isModern() && v.legacyShaperDesired!=v.legacyShaperApplied){v.core.setTanhShaperDrive(v.legacyShaperDesired);v.legacyShaperApplied=v.legacyShaperDesired;}
    if(v.isModern())v.modern.prepareRender();
    v.drive.beginRender();
    v.routeTarget=v.isModern()&&v.driveFx[0]>.5f;
    if(!v.drive.processing() || v.silenced){v.route=v.routeTarget;v.routeGain=1;}
    for (int i = 0; i < frames; ++i) {
        v.sequence.sample();
        double pitch=v.sequence.pitchMod();
        if(pitch!=v.lastPitchMod){if(v.isModern())v.modern.pitchBend(pitch);else v.core.setPitchBend(pitch);v.lastPitchMod=pitch;}
        const float routeGain=v.advanceRoute();
        const bool pre=v.isModern() && v.route==1 && v.drive.processing();
        const float dry = static_cast<float>(v.isModern()?v.modern.sample(pre?&v.drive:nullptr):v.core.getSample());
        const float x=(pre?dry:v.drive.process(v.silenced ? 0.0f : dry))*routeGain;
        out[2*i] = out[2*i+1] = v.silenced ? 0.0f : x;
    }
}
void position(void* instance,double ppq,double bpm,int playing,int frames) {
    auto& v=*static_cast<Voice*>(instance);
    if(std::isfinite(bpm)&&bpm>0)v.modern.tempo(bpm);
    v.sequence.begin(ppq,bpm,playing!=0,frames);
}
const mpc_engine_t api {create, destroy, midi, set, get, render, nullptr, position};
} // namespace

extern "C" const mpc_engine_t* mpc_engine() { return &api; }
extern "C" void mpc_engine_cycle(void* instance,double start,double end,int active) {
    static_cast<Voice*>(instance)->sequence.cycle(start,end,active!=0);
}
extern "C" int mpc_engine_changed(void* instance,char* key,int size) {
    return static_cast<Voice*>(instance)->changed(key,size);
}
