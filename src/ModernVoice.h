#pragma once
#include <memory>
// MPC adapter around upstream v0.14 DSP. Opaque to avoid mixing old/new headers.
class ModernVoice {
public:
    static constexpr int First=66, Count=20;
    enum Param { Engine=66,Osc,Filter,Morph,Saturation,Quality,Tension,Trim,Fm,Bias,Rail,
                 Square,Juno,Detune,SuperMix,Shape,Drift,Split,SubGain,SubBlend };
    ModernVoice();
    ~ModernVoice();
    ModernVoice(const ModernVoice&)=delete;
    void configure(const float* oldValues,const float* extra);
    void noteOn(int note,int velocity);
    void allNotesOff();
    double sample();
private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
