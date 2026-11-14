// MPC integration: GPL-3.0-only; linked v0.14 DSP has file-specific MIT/AGPL notices.
#include "ModernVoice.h"
#include "dsp/v014/open303/rosic_Open303.h"
#include <cmath>
#include <array>
#include <algorithm>
struct ModernVoice::Impl { rosic014::Open303 voice; std::array<float,22> old{};std::array<float,Count> extra{};std::array<float,9> matrix{};bool matrixConfigured=false;bool configured=false; Impl(){voice.setSampleRate(44100);} };
ModernVoice::ModernVoice():impl(std::make_unique<Impl>()){}
ModernVoice::~ModernVoice()=default;
void ModernVoice::noteOn(int n,int v,bool mute,bool hammer,bool reverse,double gateSamples){impl->voice.noteOn(n,v,0,mute,hammer,reverse,gateSamples);}
void ModernVoice::pitchBend(double s){impl->voice.setPitchBend(s);}
void ModernVoice::tempo(double bpm){impl->voice.setReverseTempo(bpm);}
void ModernVoice::configureMatrix(const float* m){
 auto& v=impl->voice;auto changed=[&](int p){return !impl->matrixConfigured||m[p]!=impl->matrix[p];};
 if(changed(0))v.setLfoDestination(static_cast<int>(m[0]));
 if(changed(1)||changed(2))v.setModSlot(1,rosic014::MOD_SRC_ENV,static_cast<int>(m[1]),m[2]);
 if(changed(3)||changed(4)||changed(5))v.setModSlot(2,static_cast<int>(m[3]),static_cast<int>(m[4]),m[5]);
 if(changed(6)||changed(7)||changed(8))v.setModSlot(3,static_cast<int>(m[6]),static_cast<int>(m[7]),m[8]);
 std::copy_n(m,9,impl->matrix.begin());impl->matrixConfigured=true;
}
double ModernVoice::driveModulation()const{return impl->voice.getLfoOverdriveMod();}
void ModernVoice::allNotesOff(){impl->voice.allNotesOff();}
double ModernVoice::sample(){return impl->voice.getSample();}
void ModernVoice::configure(const float* p,const float* x){
 auto& v=impl->voice;const bool mods=p[8]>=.5f;
 auto linear=[](double a,double b,double t){return a+(b-a)*t;};
 auto exp=[](double a,double b,double t){return a*std::pow(b/a,t);};
 auto e=[&](Param id){return x[id-First];};
 auto old=[&](int i,bool mod=false){return !impl->configured || p[i]!=impl->old[i] || (mod && p[8]!=impl->old[8]);};
 auto changed=[&](Param i){return !impl->configured || e(i)!=impl->extra[i-First] || p[8]!=impl->old[8];};
 if(old(0))v.setWaveform(p[0]);
 if(old(1))v.setTuning(linear(400,480,p[1]));
 if(old(2))v.setCutoff(exp(314,2394,p[2]));
 if(old(3))v.setResonance(100*p[3]);
 if(old(4))v.setEnvMod(100*p[4]);
 if(old(5,true))v.setDecay(exp(mods?30:200,mods?3000:2000,p[5]));
 if(old(6))v.setAccent(100*p[6]);
 if(old(7))v.setVolume(linear(-60,0,p[7]));
 if(old(9,true))v.setAmpDecay(mods?linear(30,3000,p[9]):1230);
 if(old(10,true))v.setAccentDecay(mods?linear(30,3000,p[10]):200);
 if(old(11,true))v.setFeedbackHighpass(mods?exp(350,100,p[11]):150);
 if(old(12,true))v.setNormalAttack(mods?exp(.3,3000,p[12]):3);
 if(old(13,true))v.setSlideTime(mods?linear(2,1000,p[13]):60);
 if(old(14,true)){v.setTanhShaperDrive(mods?linear(25,80,p[14]):36.9);v.setPulseSag(rosic014::Open303::sagForSquareDrive(mods?p[14]:.25));}
 if(changed(Filter))v.setFilterType(static_cast<dfl014::FilterType>(static_cast<int>(mods?e(Filter):0)));
 if(changed(Saturation))v.setFilterDrive(mods?e(Saturation):0);
 if(old(17,true))v.setPassbandCompensation(mods?p[17]:.1);
 if(changed(Morph))v.setFilterMode(e(Morph));
 if(changed(Quality))v.setFilterHiFi(e(Quality)>.5);
 if(changed(Tension))v.setFilterTension(e(Tension));
 if(changed(Trim))v.setResonanceTrim(e(Trim));
 if(changed(Fm))v.setFilterFmDepth(e(Fm));
 if(changed(Bias))v.setFilterBias(e(Bias));
 if(changed(Rail))v.setFilterRail(e(Rail));
 if(changed(Osc))v.setOscModel(static_cast<rosic014::OscModel>(static_cast<int>(mods?e(Osc):0)));
 if(changed(Square))v.setSquareModel(static_cast<rosic014::SquareModel>(static_cast<int>(mods?e(Square):0)));
 if(changed(Juno))v.setJunoWaveform(static_cast<int>(e(Juno)));
 if(changed(Detune))v.setSuperDetune(e(Detune));
 if(changed(SuperMix))v.setSuperMix(e(SuperMix));
 if(changed(Shape))v.setPhaseshaperMode(static_cast<int>(e(Shape)));
 if(changed(Drift))v.setPhaseshaperDrift(e(Drift));
 if(changed(Split))v.setPhaseshaperSplit(e(Split));
 if(changed(SubGain))v.setSubOscGain(mods?e(SubGain):0);
 if(changed(SubBlend))v.setSubOscBlend(e(SubBlend));
 if(old(18))v.setLfoWaveform(static_cast<int>(p[18]));
 if(old(19))v.setLfoRate(exp(.1,20,p[19]));
 if(old(20))v.setLfoDepth(p[20]);
 if(old(21))v.setLfoDestination(impl->matrixConfigured?impl->matrix[0]:p[21]);
 if(old(8)){v.setLfoOn(mods);v.setCircuitPulseShape(mods);}
 std::copy_n(p,22,impl->old.begin());std::copy_n(x,Count,impl->extra.begin());impl->configured=true;
}
