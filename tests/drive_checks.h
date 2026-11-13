#pragma once
#include "drive/Drive.h"
#include "drive/RNNAccelerated.h"
#include "drive/ModelData.h"
#include <chrono>
#include <thread>

// An independently written scalar LSTM oracle. PyTorch gate order i,f,g,o;
// no RTNeural weight loader or inference code is used by this reference.
struct ScalarLstm {
    std::vector<std::vector<float>> input,recurrent;
    std::vector<float> bias,linear;
    float linearBias=0;
    std::array<float,40> h{},c{};
    explicit ScalarLstm(const nlohmann::json& json) {
        const auto& s=json.at("state_dict");
        input=s.at("rec.weight_ih_l0").get<std::vector<std::vector<float>>>();
        recurrent=s.at("rec.weight_hh_l0").get<std::vector<std::vector<float>>>();
        bias=s.at("rec.bias_ih_l0").get<std::vector<float>>();
        auto second=s.at("rec.bias_hh_l0").get<std::vector<float>>();
        for(int i=0;i<160;++i) bias[i]+=second[i];
        linear=s.at("lin.weight").at(0).get<std::vector<float>>();
        linearBias=s.at("lin.bias").at(0).get<float>();
    }
    float process(float x,float condition) {
        std::array<float,160> z{};
        for(int j=0;j<160;++j) {
            float sum=0;
            for(int k=0;k<40;++k) sum=std::fma(h[k],recurrent[j][k],sum);
            if(input[j].size()==1) z[j]=std::fma(input[j][0],x,sum+bias[j]);
            else {
                float kernel=std::fma(input[j][0],x,0.0f);
                kernel=std::fma(input[j][1],condition,kernel);
                z[j]=sum+bias[j]+kernel;
            }
        }
        for(int j=0;j<40;++j) {
            const float i=math_approx::sigmoid_exp<5,true>(z[j]);
            const float f=math_approx::sigmoid_exp<5,true>(z[40+j]);
            const float g=math_approx::tanh<7>(z[80+j]);
            const float o=math_approx::sigmoid_exp<5,true>(z[120+j]);
            c[j]=std::fma(i,g,f*c[j]);h[j]=o*math_approx::tanh<7>(c[j]);
        }
        float y=linearBias;
        for(int j=0;j<40;++j) y=std::fma(h[j],linear[j],y);
        return x+y; // GuitarML residual output
    }
};

void neuralReferenceChecks() {
    rnn_sse_arm::RNNAccelerated<1,40,RecurrentLayerType::LSTMLayer,2> plain;
    rnn_sse_arm::RNNAccelerated<2,40,RecurrentLayerType::LSTMLayer,2> conditioned;
    for(int model : {0,1,4,16,36}) {
        auto json=nlohmann::json::parse(jc303drive::models[model].json);
        const int inputs=jc303drive::models[model].inputs;
        if(inputs==1){plain.initialise(json);plain.prepare(1.0f);}
        else{conditioned.initialise(json);conditioned.prepare(1.0f);}
        ScalarLstm reference(json);
        double error=0,energy=0;
        for(int i=0;i<1024;++i) {
            float x=.08f*std::sin(i*.041f),condition=.35f;
            const float expected=reference.process(x,condition);
            if(inputs==1) plain.process({&x,1},true);
            else conditioned.process_conditioned({&x,1},{&condition,1},true);
            error=std::max(error,std::abs(static_cast<double>(x-expected)));energy+=x*x;
        }
        std::printf("GuitarML scalar reference %d: max error %.6g, energy %.6g\n",model,error,energy);
        require(error<.002,"GuitarML inference differs from the scalar PyTorch-weight oracle");
        require(energy>0,"neural model produced no output");
    }
}

// At the fixed 44.1 kHz model rate the resampling delay is redundant. Check
// the direct recurrent state against the original LinInterp(1) on every model.
void fixedRateChecks() {
    rnn_sse_arm::RNNAccelerated<1,40,RecurrentLayerType::LSTMLayer,0> plain;
    rnn_sse_arm::RNNAccelerated<2,40,RecurrentLayerType::LSTMLayer,0> conditioned;
    rnn_sse_arm::RNNAccelerated<1,40,RecurrentLayerType::LSTMLayer,2> legacyPlain;
    rnn_sse_arm::RNNAccelerated<2,40,RecurrentLayerType::LSTMLayer,2> legacyConditioned;
    for(int model=0;model<jc303drive::modelCount;++model) {
        auto json=nlohmann::json::parse(jc303drive::models[model].json);
        const int inputs=jc303drive::models[model].inputs;
        if(inputs==1){plain.initialise(json);plain.prepare(1.0f);legacyPlain.initialise(json);legacyPlain.prepare(1.0f);}
        else{conditioned.initialise(json);conditioned.prepare(1.0f);legacyConditioned.initialise(json);legacyConditioned.prepare(1.0f);}
        for(int i=0;i<512;++i) {
            float x=.12f*std::sin(i*.051f),old=x,level=.37f;
            if(inputs==1){plain.process({&x,1},true);legacyPlain.process({&old,1},true);}
            else{conditioned.process_conditioned({&x,1},{&level,1},true);legacyConditioned.process_conditioned({&old,1},{&level,1},true);}
            require(x==old,"fixed-rate optimization changed neural samples");
        }
    }
    std::puts("all 37 fixed-rate networks match LinInterp(1) sample for sample");
}

void waitDrive(GuitarDrive& drive) {
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(30);
    while(static_cast<int>(drive.get(GuitarDrive::Status))!=2) {
        require(drive.get(GuitarDrive::Status)!=3,"GuitarML worker failed to prepare a built-in model");
        require(std::chrono::steady_clock::now()<deadline,"GuitarML model preparation timed out");
        drive.beginRender();for(int i=0;i<128;++i)drive.process(0);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
void allModelsChecks() {
    GuitarDrive drive;
    for(int i=0;i<256;++i) {
        float x=.13f*std::sin(.12f*i);
        require(drive.process(x)==x,"fresh Init must be bit-exact dry");
    }
    drive.set(GuitarDrive::Enable,1);drive.set(GuitarDrive::Mix,1);
    double largest=0;
    for(int model=0;model<jc303drive::modelCount;++model) {
        drive.set(GuitarDrive::Model,static_cast<float>(model));
        waitDrive(drive);
        drive.set(GuitarDrive::Level,.75f);
        drive.beginRender();
        for(int i=0;i<4096;++i) {
            float x=drive.process(.1f*std::sin(.05f*i));
            require(std::isfinite(x),"built-in GuitarML model produced non-finite output");
            largest=std::max(largest,std::abs(static_cast<double>(x)));
        }
    }
    std::printf("all %d GuitarML models loaded and rendered; largest test peak %.5g\n",jc303drive::modelCount,largest);
    for(int i=0;i<100;++i) {
        drive.set(GuitarDrive::Model,static_cast<float>(i%jc303drive::modelCount));
        drive.beginRender();for(int j=0;j<64;++j)require(std::isfinite(drive.process(.01f)),"rapid model switch failed");
    }
    drive.set(GuitarDrive::Model,3);waitDrive(drive);
    require(drive.get(GuitarDrive::Model)==3,"rapid switches did not settle on the final model");
    drive.set(GuitarDrive::Enable,0);
    for(int i=0;i<1024;++i)drive.process(.01f);
    require(drive.process(.123456f)==.123456f,"disabled drive did not return to exact bypass");
    for(int i=0;i<4;++i) { GuitarDrive pending;pending.set(GuitarDrive::Enable,1); }
    std::puts("model switch cancellation, exact bypass and destruction during worker load: passed");
}

void presetBrowserChecks() {
    Plugin p;
    require(p.fx->programs==12,"MPC browser requires a non-empty VST program bank");
    char name[64]{};
    p.fx->dispatch(p.fx,5,0,0,name,0);
    require(std::string(name)=="Init","new dry plugin must show Init in the MPC browser");
    require(p.fx->get(p.fx,index("driveEnabled"))==0 && p.fx->get(p.fx,index("seqEnabled"))==0,
            "Init must start dry with the sequencer off");
    for(int i=0;i<12;++i) {
        name[0]=0;
        require(p.fx->dispatch(p.fx,29,i,0,name,0)==1 && name[0],"preset bank has an unnamed entry");
        p.fx->dispatch(p.fx,2,0,i,nullptr,0);
        require(p.fx->dispatch(p.fx,3,0,0,nullptr,0)==i,"MPC program selection was not retained");
        p.render(128);
    }
    p.fx->dispatch(p.fx,2,0,9,nullptr,0); // Rat Bass: wet model and settings must save with edits.
    p.set("driveLevel",.63f);
    void* chunk=nullptr;const auto size=p.fx->dispatch(p.fx,23,0,0,&chunk,0);
    std::vector<char> saved(static_cast<char*>(chunk),static_cast<char*>(chunk)+size);
    Plugin restored;restored.fx->dispatch(restored.fx,24,0,size,saved.data(),0);
    require(restored.fx->dispatch(restored.fx,3,0,0,nullptr,0)==9,"project reload lost selected MPC preset name");
    require(std::abs(restored.fx->get(restored.fx,index("driveLevel"))-.63f)<1e-6,
            "project reload reapplied factory settings over edits");
    require(std::abs(restored.fx->get(restored.fx,index("driveModel"))-4.0f/36)<1e-6,
            "project reload lost GuitarML model identity");
    p.fx->dispatch(p.fx,2,0,0,nullptr,0);
    require(p.fx->get(p.fx,index("driveEnabled"))==0 && p.fx->get(p.fx,index("seqEnabled"))==0,
            "Init preset did not clear drive/sequence enable states");
    // Exercise the wet path through the actual plugin, including asynchronous preparation.
    Plugin dry,wet;
    wet.set("driveEnabled",1);wet.set("driveMix",1);
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(30);
    while(wet.fx->get(wet.fx,index("driveStatus"))<.6f) {
        require(std::chrono::steady_clock::now()<deadline,"VST drive preparation timed out");
        wet.render(128);std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    require(wet.fx->get(wet.fx,index("driveStatus"))<.9f,"VST drive preparation failed");
    wet.render(4096);
    dry.note(0x90,48,100);wet.note(0x90,48,100);
    const auto clean=dry.render(4096),driven=wet.render(4096);
    double difference=0;
    for(size_t i=0;i<clean.size();++i)difference+=std::abs(clean[i]-driven[i]);
    require(difference>1,"VST drive enable did not change synth audio");
    // Sound-only / sequencer 0.1.0 chunks must default to dry Init.
    p.fx->dispatch(p.fx,23,0,0,&chunk,0);
    std::string old(static_cast<char*>(chunk));
    old.replace(0,std::strlen("JC303-MPC-4"),"JC303-MPC-3");old.erase(old.find("drive "));
    restored.fx->dispatch(restored.fx,24,0,old.size()+1,old.data(),0);
    require(restored.fx->get(restored.fx,index("driveEnabled"))==0 &&
            restored.fx->dispatch(restored.fx,3,0,0,nullptr,0)==0,"old project must load as dry Init");
    std::puts("MPC browser: Init, 12 named programs, program changes and edited project restore: passed");
}
void driveChecks(){neuralReferenceChecks();fixedRateChecks();allModelsChecks();presetBrowserChecks();}
