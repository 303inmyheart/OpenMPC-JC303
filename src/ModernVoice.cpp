// MPC integration: GPL-3.0-only; linked v0.14 DSP has file-specific MIT/AGPL notices.
#include "ModernVoice.h"
#include "drive/Drive.h"
#include "dsp/v014/open303/rosic_Open303.h"
#include <cmath>
#include <array>
#include <algorithm>
struct ModernVoice::Impl { rosic014::Open303 voice; std::array<float,22> old{};std::array<float,Count> extra{};std::array<float,9> matrix{};std::array<float,3> pages{1,1,1};std::array<float,5> extensions{};bool extConfigured=false;bool matrixConfigured=false;bool configured=false; Impl(){voice.setSampleRate(44100);} };
ModernVoice::ModernVoice():impl(std::make_unique<Impl>()){}
ModernVoice::~ModernVoice()=default;
void ModernVoice::noteOn(int n,int v,bool mute,bool hammer,bool reverse,double gateSamples){impl->voice.noteOn(n,v,0,mute,hammer,reverse,gateSamples);}
void ModernVoice::pitchBend(double s){impl->voice.setPitchBend(s);}
void ModernVoice::tempo(double bpm){impl->voice.setReverseTempo(bpm);}
void ModernVoice::configureMatrix(const float* m,const float* pages){
 auto& v=impl->voice;for(int i=0;i<4;++i)v.setModSlotEnabled(i,pages[3+i]>.5f);auto changed=[&](int p){return !impl->matrixConfigured||m[p]!=impl->matrix[p];};
 if(changed(0))v.setLfoDestination(static_cast<int>(m[0]));
 if(changed(1)||changed(2))v.setModSlot(1,rosic014::MOD_SRC_ENV,static_cast<int>(m[1]),m[2]);
 if(changed(3)||changed(4)||changed(5))v.setModSlot(2,static_cast<int>(m[3]),static_cast<int>(m[4]),m[5]);
 if(changed(6)||changed(7)||changed(8))v.setModSlot(3,static_cast<int>(m[6]),static_cast<int>(m[7]),m[8]);
 std::copy_n(m,9,impl->matrix.begin());impl->matrixConfigured=true;
}
double ModernVoice::driveModulation()const{return impl->voice.getLfoOverdriveMod();}
void ModernVoice::allNotesOff(){impl->voice.allNotesOff();}
double ModernVoice::sample(GuitarDrive* insert){return insert?impl->voice.getSampleWithPreVca([insert](float x){return insert->process(x);}):impl->voice.getSample();}
void ModernVoice::configure(const float* p,const float* x,const float* pages,const float* extensions){
 auto& v=impl->voice;const bool mods=p[8]>=.5f;
 const bool circuit=mods&&pages[0]>.5f,oscillator=mods&&pages[1]>.5f,filter=mods&&pages[2]>.5f;
 for(int i=0;i<3;++i)if(pages[i]!=impl->pages[i])impl->configured=false;
 std::copy_n(pages,3,impl->pages.begin());
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
 if(old(5,true))v.setDecay(exp(circuit?30:200,circuit?3000:2000,p[5]));
 if(old(6))v.setAccent(100*p[6]);
 if(old(7))v.setVolume(linear(-60,0,p[7]));
 if(old(9,true))v.setAmpDecay(circuit?linear(30,3000,p[9]):1230);
 if(old(10,true))v.setAccentDecay(circuit?linear(30,3000,p[10]):200);
 if(old(11,true))v.setFeedbackHighpass(circuit?exp(350,100,p[11]):150);
 if(old(12,true))v.setNormalAttack(circuit?exp(.3,3000,p[12]):3);
 if(old(13,true))v.setSlideTime(circuit?linear(2,1000,p[13]):60);
 if(old(14,true)){v.setTanhShaperDrive(circuit?linear(25,80,p[14]):36.9);v.setPulseSag(rosic014::Open303::sagForSquareDrive(circuit?p[14]:.25));}
 if(changed(Filter))v.setFilterType(static_cast<dfl014::FilterType>(static_cast<int>(mods&&filter?e(Filter):0)));
 if(changed(Saturation))v.setFilterDrive(mods&&filter?e(Saturation):0);
 if(old(17,true))v.setPassbandCompensation(circuit?p[17]:.1);
 if(changed(Morph))v.setFilterMode((filter?e(Morph):0));
 if(changed(Quality))v.setFilterHiFi((filter?e(Quality):0)>.5);
 if(changed(Tension))v.setFilterTension((filter?e(Tension):0));
 if(changed(Trim))v.setResonanceTrim((filter?e(Trim):1));
 if(changed(Fm))v.setFilterFmDepth((filter?e(Fm):0));
 if(changed(Bias))v.setFilterBias((filter?e(Bias):.5));
 if(changed(Rail))v.setFilterRail((filter?e(Rail):1));
 if(changed(Osc))v.setOscModel(static_cast<rosic014::OscModel>(static_cast<int>(oscillator?e(Osc):0)));
 if(changed(Square))v.setSquareModel(static_cast<rosic014::SquareModel>(static_cast<int>(oscillator?e(Square):0)));
 if(changed(Juno))v.setJunoWaveform(static_cast<int>(e(Juno)));
 if(changed(Detune))v.setSuperDetune(e(Detune));
 if(changed(SuperMix))v.setSuperMix(e(SuperMix));
 if(changed(Shape))v.setPhaseshaperMode(static_cast<int>(e(Shape)));
 if(changed(Drift))v.setPhaseshaperDrift(e(Drift));
 if(changed(Split))v.setPhaseshaperSplit(e(Split));
 if(changed(SubGain))v.setSubOscGain(oscillator?e(SubGain):0);
 if(changed(SubBlend))v.setSubOscBlend(e(SubBlend));
 if(old(18))v.setLfoWaveform(static_cast<int>(p[18]));
 if(old(19))v.setLfoRate(exp(.1,20,p[19]));
 if(old(20))v.setLfoDepth(p[20]);
 if(old(21))v.setLfoDestination(impl->matrixConfigured?impl->matrix[0]:p[21]);
 if(old(8)){v.setLfoOn(mods);v.setCircuitPulseShape(oscillator);}
 auto extChanged=[&](int i){return !impl->extConfigured||!impl->configured||extensions[i]!=impl->extensions[i]||p[8]!=impl->old[8];};
 if(extChanged(0))v.setLfoPhase(extensions[0]);
 if(extChanged(1))v.setLfoKeySync(mods&&extensions[1]>.5f);
 if(extChanged(2))v.setLfoContour(mods?extensions[2]:0);
 if(extChanged(3))v.setAccentSweep(circuit?15.0*std::pow(2000.0/15.0,extensions[3]):15.0);
 if(extChanged(4))v.setReverseGate(circuit?extensions[4]:0);
 std::copy_n(extensions,5,impl->extensions.begin());impl->extConfigured=true;
 std::copy_n(p,22,impl->old.begin());std::copy_n(x,Count,impl->extra.begin());impl->configured=true;
}
