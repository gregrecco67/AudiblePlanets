#include "RandEditor.h"

RandEditor::RandEditor(APAudioProcessor& proc_) : proc(proc_)
{
	addAndMakeVisible(matrix);
	addAndMakeVisible(randomizeButton);
	randomizeButton.onClick = [this] { randomize(); };
	//addAndMakeVisible(hello);
    addAndMakeVisible(randNumber);
    addAndMakeVisible(randAmount);
    randNumber.setNormalisableRange({1.0, 5.0, 1.0});
    randAmount.setRange(0.0, 0.2);
}

void RandEditor::randomize()
{
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> modDist(-1.f, 1.f);
    auto& params = proc.getPluginParameters();
    std::uniform_int_distribution<> paramsDist{0, params.size() - 1};
    auto numSrcs = proc.modMatrix.getNumModSources();
    std::uniform_int_distribution<> srcsDist{0, numSrcs - 1};
    
    int numMods = randNumber.getValue();
    std::array<int, 5> modSrcs;
    for (int i = 0; i < numMods; i++) {
        auto modSrc = gin::ModSrcId(srcsDist(gen));
        auto modDst = gin::ModDstId(paramsDist(gen));
        auto depth = proc.modMatrix.getModDepth(modSrc, modDst);
        proc.modMatrix.setModDepth(modSrc, modDst, depth + modDist(gen) * randAmount.getValue());
    }
}

void RandEditor::resized()
{
	matrix.setBounds(5, 5, 5*56, 5*70+23);
	randomizeButton.setBounds(5, 378, 3*56, 20);
    randNumber.setBounds(5, 404, 3*56, 20);
    randAmount.setBounds(5, 429, 3*56, 20);
	//hello.setBounds(5, 335, 150, 20);
}
