#pragma once
#include <array>
#include <cstdint>
// Pitch-class guidance only. Never changes stored or emitted MIDI notes.
struct ScaleGuide {
    int root=9,kind=0;
    static constexpr int Count=20;
    static constexpr std::array<const char*,Count> Names{{"Off","Major","Minor","Harmonic Minor","Melodic Minor","Dorian","Phrygian","Lydian","Mixolydian","Locrian","Major Pentatonic","Minor Pentatonic","Minor Blues","Major Blues","Whole Tone","Dim Half-Whole","Dim Whole-Half","Phrygian Dominant","Double Harmonic","Chromatic"}};
    static constexpr std::array<uint16_t,Count> Masks{{
        0,0xAB5,0x5AD,0x9AD,0xAAD,0x6AD,0x5AB,0xAD5,0x6B5,0x56B,
        0x295,0x4A9,0x4E9,0x29D,0x555,0x6DB,0xB6D,0x5B3,0x9B3,0xFFF
    }};
    bool includes(int pitch)const{return kind>0 && kind<Count && (Masks[kind]&(1u<<((pitch-root+12)%12)));}
};
