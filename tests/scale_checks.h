#pragma once
void scaleChecks(){
    // Independent interval definitions, rather than copies of the engine masks.
    const std::vector<std::vector<int>> expected{
        {},{0,2,4,5,7,9,11},{0,2,3,5,7,8,10},{0,2,3,5,7,8,11},{0,2,3,5,7,9,11},
        {0,2,3,5,7,9,10},{0,1,3,5,7,8,10},{0,2,4,6,7,9,11},{0,2,4,5,7,9,10},{0,1,3,5,6,8,10},
        {0,2,4,7,9},{0,3,5,7,10},{0,3,5,6,7,10},{0,2,3,4,7,9},{0,2,4,6,8,10},
        {0,1,3,4,6,7,9,10},{0,2,3,5,6,8,9,11},{0,1,4,5,7,8,10},{0,1,4,5,7,8,11},{0,1,2,3,4,5,6,7,8,9,10,11}};
    require(expected.size()==ScaleGuide::Count,"scale list and interval oracle disagree");
    for(int kind=0;kind<ScaleGuide::Count;++kind)for(int root=0;root<12;++root){
        require(std::strcmp(PARAMS[seqparam::seqKeyScale].opts[kind],ScaleGuide::Names[kind])==0,"scale menu labels and masks have different indices");
        ScaleGuide guide;guide.root=root;guide.kind=kind;
        for(int pitch=0;pitch<12;++pitch){int interval=(pitch-root+12)%12;
            bool match=std::find(expected[kind].begin(),expected[kind].end(),interval)!=expected[kind].end();
            require(guide.includes(pitch)==match,"key/scale highlight differs from interval oracle");}
    }
    auto core=std::make_unique<rosic::Open303>();MpcSequence sequence(*core);
    std::ostringstream before;sequence.saveExtended(before);
    sequence.set(seqparam::seqKeyRoot,9);sequence.set(seqparam::seqKeyScale,2);
    const std::vector<int> aMinor{0,2,4,5,7,9,11};
    for(int page=0;page<4;++page){sequence.set(seqparam::seqPage,page);for(int pitch=0;pitch<12;++pitch)
        require(bool(sequence.get(seqparam::ScaleViewFirst+pitch))==(std::find(aMinor.begin(),aMinor.end(),pitch)!=aMinor.end()),"A Minor guide differs across pages");}
    sequence.set(seqparam::seqPage,0);std::ostringstream after;sequence.saveExtended(after);
    require(before.str()==after.str(),"scale selection silently rewrote a pattern or its timing");
    sequence.set(seqparam::ScaleViewFirst,0);require(sequence.get(seqparam::ScaleViewFirst)==1,"host could write the read-only scale highlight");
    sequence.set(seqparam::seqKeyScale,0);for(int pitch=0;pitch<12;++pitch)require(sequence.get(seqparam::ScaleViewFirst+pitch)==0,"Off left a scale highlight active");
    Plugin dry,guided;dry.units("engine014",1);guided.units("engine014",1);
    guided.units("seqKeyRoot",9);guided.units("seqKeyScale",2);
    dry.note(0x90,61,110);guided.note(0x90,61,110);
    require(dry.render(4096)==guided.render(4096),"visual scale guide changed synthesized audio");
    std::puts("12 keys x 19 scales plus Off: interval oracle, four-page highlighting, read-only views and non-destructive selection PASS");
}
