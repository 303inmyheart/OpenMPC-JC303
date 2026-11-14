#pragma once
// MPC adaptation of midilab/jc303 v0.14 Disperser (src/JC303.h), GPL-3.0-only.
// Original eight-stage dfl filter and phase response are retained; coefficient
// update cadence continues across host/MIDI block boundaries.
#include "../dsp/v014/open303/dfl_DispersionFilter.h"
#include <algorithm>
#include <cmath>
class DriveDisperser {
    dfl014::DispersionFilter filter;
    float amount=0;
    double logCorner=0;
    bool active=false;
    unsigned interval=0;
public:
    DriveDisperser(){filter.setSampleRate(44100);filter.setStages(8);filter.setSpread(2);filter.setQ(1);filter.reset();}
    static double cornerFor(float value){return 150.0*std::pow(20.0,value);}
    void setAmount(float value)noexcept{amount=std::clamp(value,0.0f,1.0f);}
    float process(float x)noexcept{
        if(amount<=0){active=false;return x;}
        const double target=std::log(150.0)+amount*std::log(20.0);
        if(!active){active=true;filter.reset();logCorner=target;filter.setCenterFreq(std::exp(logCorner));interval=0;}
        if(interval++%16==0 && std::abs(target-logCorner)>1e-4){logCorner+=.1*(target-logCorner);filter.setCenterFreq(std::exp(logCorner));}
        return filter.process(x);
    }
};
