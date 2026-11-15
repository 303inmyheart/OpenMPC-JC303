#pragma once
#include <memory>
// MPC adapter around upstream v0.14 DSP. Opaque to avoid mixing old/new headers.
class GuitarDrive;
class ModernVoice {
public:
    static constexpr int First=66, Count=20;
    enum Param { Engine=66,Osc,Filter,Morph,Saturation,Quality,Tension,Trim,Fm,Bias,Rail,
                 Square,Juno,Detune,SuperMix,Shape,Drift,Split,SubGain,SubBlend };
    ModernVoice();
    ~ModernVoice();
    ModernVoice(const ModernVoice&)=delete;
    void configure(const float* oldValues,const float* extra,const float* pages,const float* extensions);
    void configureMatrix(const float* values,const float* pages);
    double driveModulation() const;
    void noteOn(int note,int velocity,bool mute=false,bool hammer=false,bool reverse=false,double gateSamples=0);
    void pitchBend(double semitones);
    void tempo(double bpm);
    void allNotesOff();
    void prepareRender();
    double sample(GuitarDrive* preVca=nullptr);
private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
