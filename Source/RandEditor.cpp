#include "RandEditor.h"

RandEditor::RandEditor(APAudioProcessor& proc_) : proc(proc_)
{
	addAndMakeVisible(matrix);
	addAndMakeVisible(randomizeButton);
	randomizeButton.onClick = [this] { randomize(); };
	addAndMakeVisible(clearAllButton);
	clearAllButton.onClick = [this] { clearAll(); };
    addAndMakeVisible(randNumber);
    addAndMakeVisible(randAmount);
    randNumber.setNormalisableRange({1.0, 5.0, 1.0});
    randAmount.setRange(0.05, 0.5);
    addAndMakeVisible(randOSCsButton);
    randOSCsButton.onClick = [this] { randomizeOSCs(); };
    addAndMakeVisible(test);
    addAndMakeVisible(randENVsButton);
    randENVsButton.onClick = [this] { randomizeENVs(); };
    addAndMakeVisible(inharmonic);
    addAndMakeVisible(reharmonizeButton);
    reharmonizeButton.onClick = [this] { resetFine(); };
    addAndMakeVisible(randLFOsButton);
    randLFOsButton.onClick = [this] { randomizeLFOs(); };
}

void RandEditor::randomize()
{
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> modDist(-1.f, 1.f);
    auto& params = proc.getPluginParameters();
    std::uniform_int_distribution<> paramsDist{0, params.size() - 25};
    auto numSrcs = proc.modMatrix.getNumModSources();
    std::uniform_int_distribution<> srcsDist{0, numSrcs - 1};
    
    int numMods = randNumber.getValue();
    for (int i = 0; i < numMods; i++) {
        auto modSrc = gin::ModSrcId(srcsDist(gen));
        auto modDst = gin::ModDstId(paramsDist(gen));
		auto depth = proc.modMatrix.getModDepth(modSrc, modDst);
		auto sign = modDist(gen) >= 0 ? 1 : -1;
		proc.modMatrix.setModDepth(modSrc, modDst, std::clamp(depth + sign * randAmount.getValue(), -1., 1.));
    }
}

void RandEditor::clearAll() {
	auto& params = proc.getPluginParameters();
	for (auto* param : params) {
		if (param->getModIndex() == -1) continue;
		if (proc.modMatrix.isModulated(gin::ModDstId(param->getModIndex()))) {
			auto modSrcs = proc.modMatrix.getModSources(param);
			for (auto modSrc : modSrcs) {
				proc.modMatrix.clearModDepth(modSrc, gin::ModDstId(param->getModIndex()));
			}
		}
	}
}

void RandEditor::randomizeOSCs() {
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> fullDist(-1.f, 1.f);
    std::vector<gin::Parameter::Ptr> params{
        proc.osc1Params.coarse, proc.osc1Params.volume, proc.osc1Params.tones, proc.osc1Params.pan,
        proc.osc1Params.spread,
        proc.osc2Params.coarse, proc.osc2Params.volume, proc.osc2Params.tones, proc.osc2Params.pan,
        proc.osc2Params.spread,
        proc.osc3Params.coarse, proc.osc3Params.volume, proc.osc3Params.tones, proc.osc3Params.pan,
        proc.osc3Params.spread,
        proc.osc4Params.coarse, proc.osc4Params.volume, proc.osc4Params.tones, proc.osc4Params.pan,
        proc.osc4Params.spread
        };
    if (inharmonic.getToggleState()) {
        params.push_back(proc.osc1Params.fine);
        params.push_back(proc.osc2Params.fine);
        params.push_back(proc.osc3Params.fine);
        params.push_back(proc.osc4Params.fine);
    }
    for (gin::Parameter::Ptr param : params) {
            float randVal = randAmount.getValue();
            float paramVal = param->getValue();
            if (std::abs(fullDist(gen) * 0.5) < randVal) {
                param->setValue(std::clamp((float)(paramVal + randVal * fullDist(gen)), 0.f, 1.f));
            }
        }
}

void RandEditor::resetFine() {
    proc.osc1Params.fine->setValue(0.5);
    proc.osc2Params.fine->setValue(0.5);
    proc.osc3Params.fine->setValue(0.5);
    proc.osc4Params.fine->setValue(0.5);
}


void RandEditor::randomizeENVs() {
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> fullDist(-1.f, 1.f);
    for (gin::Parameter::Ptr param : {
        proc.env1Params.attack, proc.env1Params.decay, proc.env1Params.sustain, proc.env1Params.release,
        proc.env1Params.acurve, proc.env1Params.drcurve,
        proc.env2Params.attack, proc.env2Params.decay, proc.env2Params.sustain, proc.env2Params.release,
        proc.env2Params.acurve, proc.env2Params.drcurve,
        proc.env3Params.attack, proc.env3Params.decay, proc.env3Params.sustain, proc.env3Params.release,
        proc.env3Params.acurve, proc.env3Params.drcurve,
        proc.env4Params.attack, proc.env4Params.decay, proc.env4Params.sustain, proc.env4Params.release,
        proc.env4Params.acurve, proc.env4Params.drcurve} ) {
            float randVal = randAmount.getValue();
            float paramVal = param->getValue();
            if (std::abs(fullDist(gen) * 0.5) < randVal) {
                param->setValue(std::clamp((float)(paramVal + randVal * fullDist(gen)), 0.f, 1.f));
            }
        }
}
void RandEditor::randomizeLFOs() {
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> fullDist(-1.f, 1.f);
    auto randVal = randAmount.getValue();
    for (gin::Parameter::Ptr param : {
        proc.lfo1Params.sync, proc.lfo2Params.sync, proc.lfo3Params.sync, proc.lfo4Params.sync
    }) {
        if (std::abs(fullDist(gen) * 0.5) < randVal ) {
            if (param->isOn()) {
                param->setValue(0.f);
            } else {
                param->setValue(1.f);
            }
        }
    }
    for (gin::Parameter::Ptr param : {
        proc.lfo1Params.beat, proc.lfo2Params.beat, proc.lfo3Params.beat, proc.lfo4Params.beat,
        proc.lfo1Params.rate, proc.lfo2Params.rate, proc.lfo3Params.rate, proc.lfo4Params.rate,
        proc.lfo1Params.depth, proc.lfo2Params.depth, proc.lfo3Params.depth, proc.lfo4Params.depth,
        proc.lfo1Params.wave, proc.lfo2Params.wave, proc.lfo3Params.wave, proc.lfo4Params.wave,
    }) {
        if (std::abs(fullDist(gen) * 0.5) < randVal ) {
            param->setValue((fullDist(gen) + 2.0) * 0.5);
        }
    }
    
}
void RandEditor::randLFOtoOSC() {
    
}
void RandEditor::randENVtoOSC() {
    
}
void RandEditor::randLFOtoTimbre() {
    
}
void RandEditor::randENVtoTimbre() {
    
}

void RandEditor::resized()
{
	matrix.setBounds(5, 5, 5*56, 5*70+23);
	randomizeButton.setBounds(5, 378, 3*56, 20);
	clearAllButton.setBounds(5, 404, 3*56, 20);
    randNumber.setBounds(5, 429, 3*56, 20);
    randAmount.setBounds(5, 454, 3*56, 20);
    randOSCsButton.setBounds(5, 480, 3*56, 20);
    inharmonic.setBounds(178, 480, 2*56, 20);
    reharmonizeButton.setBounds(5, 505, 3*56, 20);
    randENVsButton.setBounds(5, 530, 3*56, 20);
    randLFOsButton.setBounds(5, 555, 3*56, 20);
    test.setBounds(800, 23, 55, 20);
}
