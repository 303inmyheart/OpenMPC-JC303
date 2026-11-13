#pragma once
#include <cmath>
#include <cstdint>
#include <algorithm>

// Sample clock for the vendored 96-PPQN event engine. The host supplies one
// position per block; render() may be split at arbitrary incoming MIDI offsets.
// Loop metadata lets a block straddle a loop end without losing its last notes.
class HostClock {
    double base=0, increment=0, lastEnd=0, oldBpm=0;
    double loopStart=0,loopEnd=0;
    int offset=0;
    int64_t lastTick=-1;
    bool valid=false, running=false, loop=false;
    double previousBeat=0;
public:
    bool playing() const { return running; }
    void cycle(double start,double end,bool active) {
        loop=active && std::isfinite(start) && std::isfinite(end) && end>start;
        loopStart=start; loopEnd=end;
    }
    double wrapped(double beat) const {
        if(loop && beat>=loopEnd) beat=loopStart+std::fmod(beat-loopStart,loopEnd-loopStart);
        return beat;
    }
    template<class Reset> void begin(double ppq,double bpm,bool play,int frames,Reset reset) {
        const bool good=play && std::isfinite(ppq) && ppq>=0 && std::isfinite(bpm) && bpm>0 && frames>0;
        if(!good) {
            if(running) reset(false);
            valid=false; running=false; offset=0; return;
        }
        ppq=wrapped(ppq);
        const bool tempoChange=std::abs(bpm-oldBpm)>1e-6;
        const double distance=ppq-lastEnd;
        const bool declaredWrap=loop && valid && ppq<previousBeat-.01;
        // Small host jitter is not a locate. A tempo-change regression of less
        // than a beat holds the high-water mark rather than replaying steps.
        const bool locate=valid && (distance>.02 || (distance<-.02 && (!tempoChange || distance<=-1)));
        if(!valid || declaredWrap || locate) {
            reset(true);
            lastTick=static_cast<int64_t>(std::ceil(ppq*96-1e-7))-1;
        }
        running=true; valid=true; offset=0; base=ppq; increment=bpm/(60*44100);
        oldBpm=bpm; previousBeat=ppq;
        lastEnd=wrapped(ppq+frames*increment);
    }
    template<class Tick,class Reset> void sample(Tick tick,Reset reset) {
        if(!running) return;
        const double beat=wrapped(base+offset*increment);
        if(loop && offset>0 && beat<previousBeat-.01) {
            reset(true);
            lastTick=static_cast<int64_t>(std::ceil(beat*96-1e-7))-1;
        }
        const auto current=static_cast<int64_t>(std::floor(beat*96+1e-7));
        if(current>lastTick) {
            // Normally zero/one tick. The limit guards malformed host jumps;
            // begin() reanchors real locates and clears outstanding notes.
            if(current-lastTick>96) lastTick=current-1;
            while(lastTick<current) tick(static_cast<uint32_t>(++lastTick));
        }
        previousBeat=beat;
        ++offset;
    }
    void invalidate() { valid=false; running=false; }
};
