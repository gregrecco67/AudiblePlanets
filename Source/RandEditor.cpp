#include "RandEditor.h"

RandEditor::RandEditor(APAudioProcessor& proc_) : proc(proc_)
{
	addAndMakeVisible(matrix);
	addAndMakeVisible(randomizeButton);
	randomizeButton.onClick = [this] { randomize(); };
	addAndMakeVisible(clearAllButton);
	clearAllButton.onClick = [this] { clearAll(); };
    
	addAndMakeVisible(randNumber);
    randNumber.setNormalisableRange({1.0, 7.0, 1.0});
    randNumber.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
	randNumber.setNumDecimalPlacesToDisplay(0);
	addAndMakeVisible(randNumberLabel);

	addAndMakeVisible(randAmount);
    randAmount.setRange(0.05, 0.5);
	randAmount.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
	randAmount.setNumDecimalPlacesToDisplay(2);
	addAndMakeVisible(randAmountLabel);

    addAndMakeVisible(randOSCsButton);
    randOSCsButton.onClick = [this] { randomizeOSCs(); };
    addAndMakeVisible(test);
    addAndMakeVisible(inharmonic);
	
	addAndMakeVisible(randInharmonicButton);
	randInharmonicButton.onClick = [this] { randomizeInharmonic(); };
    addAndMakeVisible(resetInharmonicButton);
    resetInharmonicButton.onClick = [this] { resetInharmonic(); };

	addAndMakeVisible(randENVsButton);
    randENVsButton.onClick = [this] { randomizeENVs(); };
	addAndMakeVisible(resetENVsButton);
	resetENVsButton.onClick = [this] { resetENVs(); };
    addAndMakeVisible(randLFOsButton);
    randLFOsButton.onClick = [this] { randomizeLFOs(); };
	addAndMakeVisible(resetLFOsButton);
	resetLFOsButton.onClick = [this] { resetLFOs(); };
	
	addAndMakeVisible(randLFOToOSCButton);
	randLFOToOSCButton.onClick = [this] { randLFOtoOSC(); };
	addAndMakeVisible(clearLFOToOSCButton);
	clearLFOToOSCButton.onClick = [this] { resetLFOtoOSC(); };
	addAndMakeVisible(randENVToOSCButton);
	randENVToOSCButton.onClick = [this] { randENVtoOSC(); };
	addAndMakeVisible(clearENVToOSCButton);
	clearENVToOSCButton.onClick = [this] { resetENVtoOSC(); };
	addAndMakeVisible(randKeysToOSCButton);
	randKeysToOSCButton.onClick = [this] { randKeystoOSC(); };
	addAndMakeVisible(clearKeysToOSCButton);
	clearKeysToOSCButton.onClick = [this] { resetKeystoOSC(); };
	addAndMakeVisible(randLFOToTimbreButton);
	randLFOToTimbreButton.onClick = [this] { randLFOtoTimbre(); };
	addAndMakeVisible(clearLFOToTimbreButton);
	clearLFOToTimbreButton.onClick = [this] { resetLFOtoTimbre(); };
	addAndMakeVisible(randENVToTimbreButton);
	randENVToTimbreButton.onClick = [this] { randENVtoTimbre(); };
	addAndMakeVisible(clearENVToTimbreButton);
	clearENVToTimbreButton.onClick = [this] { resetENVtoTimbre(); };
	addAndMakeVisible(randKeysToTimbreButton);
	randKeysToTimbreButton.onClick = [this] { randKeystoTimbre(); };
	addAndMakeVisible(clearKeysToTimbreButton);
	clearKeysToTimbreButton.onClick = [this] { resetKeystoTimbre(); };
	
	addAndMakeVisible(randENVToENVButton);
	randENVToENVButton.onClick = [this] { randENVtoENV(); };
	addAndMakeVisible(clearENVToENVButton);
	clearENVToENVButton.onClick = [this] { resetENVtoENV(); };
	addAndMakeVisible(randLFOToENVButton);
	randLFOToENVButton.onClick = [this] { randLFOtoENV(); };
	addAndMakeVisible(clearLFOToENVButton);
	clearLFOToENVButton.onClick = [this] { resetLFOtoENV(); };
	addAndMakeVisible(randKeysToENVButton);
	randKeysToENVButton.onClick = [this] { randKeystoENV(); };
	addAndMakeVisible(clearKeysToENVButton);
	clearKeysToENVButton.onClick = [this] { resetKeystoENV(); };

	addAndMakeVisible(randENVToLFOButton);
	randENVToLFOButton.onClick = [this] { randENVtoLFO(); };
	addAndMakeVisible(clearENVToLFOButton);
	clearENVToLFOButton.onClick = [this] { resetENVtoLFO(); };
	addAndMakeVisible(randLFOToLfoButton);
	randLFOToLfoButton.onClick = [this] { randLFOtoLFO(); };
	addAndMakeVisible(clearLFOToLFOButton);
	clearLFOToLFOButton.onClick = [this] { resetLFOtoLFO(); };
	addAndMakeVisible(randKeysToLFOButton);
	randKeysToLFOButton.onClick = [this] { randKeystoLFO(); };
	addAndMakeVisible(clearKeysToLFOButton);
	clearKeysToLFOButton.onClick = [this] { resetKeystoLFO(); };

	
	addAndMakeVisible(randFXModsButton);
	randFXModsButton.onClick = [this] { randomizeFXMods(); };
	addAndMakeVisible(clearFXModsButton);
	clearFXModsButton.onClick = [this] { clearFXMods(); };



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

void RandEditor::randomizeInharmonic()
{
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> fullDist(-1.f, 1.f);
	for (gin::Parameter::Ptr param : {
		proc.osc1Params.fine, proc.osc2Params.fine, proc.osc3Params.fine, proc.osc4Params.fine
	}) {
		float randVal = randAmount.getValue();
		float paramVal = param->getValue();
		if (std::abs(fullDist(gen) * 0.5) < randVal) {
			param->setValue(std::clamp((float)(paramVal + randVal * fullDist(gen)), 0.f, 1.f));
		}
	}
}

void RandEditor::resetInharmonic() {
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
void RandEditor::resetENVs()
{
	for (gin::Parameter::Ptr param : {
		proc.env1Params.attack, proc.env1Params.decay, proc.env1Params.sustain, proc.env1Params.release,
			proc.env1Params.acurve, proc.env1Params.drcurve,
			proc.env2Params.attack, proc.env2Params.decay, proc.env2Params.sustain, proc.env2Params.release,
			proc.env2Params.acurve, proc.env2Params.drcurve,
			proc.env3Params.attack, proc.env3Params.decay, proc.env3Params.sustain, proc.env3Params.release,
			proc.env3Params.acurve, proc.env3Params.drcurve,
			proc.env4Params.attack, proc.env4Params.decay, proc.env4Params.sustain, proc.env4Params.release,
			proc.env4Params.acurve, proc.env4Params.drcurve}) {
			param->setValue(param->getDefaultValue());
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
            param->setValue((fullDist(gen) + 2.0) * randVal);
        }
    }
    
}
void RandEditor::resetLFOs()
{
	for (gin::Parameter::Ptr param : lfoDsts) {
		param->setValue(param->getDefaultValue());
	}
}


void RandEditor::randLFOtoOSC() {
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> fullDist(-1.f, 1.f);
	auto randVal = randAmount.getValue();
	auto randNum = randNumber.getValue();
	std::array<gin::ModSrcId, 8> lfos{proc.modSrcMonoLFO1, proc.modSrcMonoLFO2, proc.modSrcMonoLFO3, proc.modSrcMonoLFO4,
		proc.modSrcLFO1, proc.modSrcLFO2, proc.modSrcLFO3, proc.modSrcLFO4};
	std::uniform_int_distribution<> lfoDist(0, 7);
	std::vector<gin::Parameter::Ptr> oscsBasic{
		proc.osc1Params.coarse, proc.osc1Params.volume, proc.osc1Params.tones, proc.osc1Params.fixed, proc.osc1Params.env, proc.osc1Params.pan, proc.osc1Params.spread, 
		proc.osc2Params.coarse, proc.osc2Params.volume, proc.osc2Params.tones, proc.osc2Params.fixed, proc.osc2Params.env, proc.osc2Params.pan, proc.osc2Params.spread,
		proc.osc3Params.coarse, proc.osc3Params.volume, proc.osc3Params.tones, proc.osc3Params.fixed, proc.osc3Params.env, proc.osc3Params.pan, proc.osc3Params.spread,
		proc.osc4Params.coarse, proc.osc4Params.volume, proc.osc4Params.tones, proc.osc4Params.fixed, proc.osc4Params.env, proc.osc4Params.pan, proc.osc4Params.spread
	};
	if (inharmonic.getToggleState()) {
		oscsBasic.push_back(proc.osc1Params.fine);
		oscsBasic.push_back(proc.osc2Params.fine);
		oscsBasic.push_back(proc.osc3Params.fine);
		oscsBasic.push_back(proc.osc4Params.fine);
	}
	std::uniform_int_distribution<> oscsBasicDist(0, oscsBasic.size() - 1);
	for (int i = 0; i < randNum; i++) {
		auto lfoSrc = lfos[lfoDist(gen)];
		auto oscDst = gin::ModDstId(oscsBasic[oscsBasicDist(gen)]->getModIndex());
		auto depth = proc.modMatrix.getModDepth(lfoSrc, oscDst);
		auto sign = fullDist(gen) >= 0 ? 1 : -1;
		proc.modMatrix.setModDepth(lfoSrc, oscDst, std::clamp(depth + sign * randVal, -1., 1.));
	}
}
void RandEditor::resetLFOtoOSC()
{
	for (gin::Parameter::Ptr param : oscDstsBasic) {
		auto modSrcs = proc.modMatrix.getModSources(param);
		for (auto modSrc : modSrcs) {
			if (lfoSrcs.contains(modSrc))
				proc.modMatrix.clearModDepth(modSrc, gin::ModDstId(param->getModIndex()));
		}
	}
}

void RandEditor::randENVtoOSC() {
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> fullDist(-1.f, 1.f);
	auto randVal = randAmount.getValue();
	auto randNum = randNumber.getValue();
	std::array<gin::ModSrcId, 4> envs{ proc.modSrcEnv1, proc.modSrcEnv2, proc.modSrcEnv3, proc.modSrcEnv4 };
	std::uniform_int_distribution<> envsDist(0, 3);
	std::vector<gin::Parameter::Ptr> oscsBasic{
		proc.osc1Params.coarse, proc.osc1Params.volume, proc.osc1Params.tones, proc.osc1Params.fixed, proc.osc1Params.env, proc.osc1Params.pan, proc.osc1Params.spread,
		proc.osc2Params.coarse, proc.osc2Params.volume, proc.osc2Params.tones, proc.osc2Params.fixed, proc.osc2Params.env, proc.osc2Params.pan, proc.osc2Params.spread,
		proc.osc3Params.coarse, proc.osc3Params.volume, proc.osc3Params.tones, proc.osc3Params.fixed, proc.osc3Params.env, proc.osc3Params.pan, proc.osc3Params.spread,
		proc.osc4Params.coarse, proc.osc4Params.volume, proc.osc4Params.tones, proc.osc4Params.fixed, proc.osc4Params.env, proc.osc4Params.pan, proc.osc4Params.spread
	};
	if (inharmonic.getToggleState()) {
		oscsBasic.push_back(proc.osc1Params.fine);
		oscsBasic.push_back(proc.osc2Params.fine);
		oscsBasic.push_back(proc.osc3Params.fine);
		oscsBasic.push_back(proc.osc4Params.fine);
	}
	std::uniform_int_distribution<> oscsBasicDist(0, oscsBasic.size() - 1);
	for (int i = 0; i < randNum; i++) {
		auto envSrc = envs[envsDist(gen)];
		auto oscDst = gin::ModDstId(oscsBasic[oscsBasicDist(gen)]->getModIndex());
		auto depth = proc.modMatrix.getModDepth(envSrc, oscDst);
		auto sign = fullDist(gen) >= 0 ? 1 : -1;
		proc.modMatrix.setModDepth(envSrc, oscDst, std::clamp(depth + sign * randVal, -1., 1.));
	}
}
void RandEditor::resetENVtoOSC()
{
	for (gin::Parameter::Ptr param : oscDstsPlus) {
		auto modSrcs = proc.modMatrix.getModSources(param);
		for (auto modSrc : modSrcs) {
			if (envSrcs.contains(modSrc))
				proc.modMatrix.clearModDepth(modSrc, gin::ModDstId(param->getModIndex()));
		}
	}
}


void RandEditor::randLFOtoTimbre() {
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> fullDist(-1.f, 1.f);
	auto randVal = randAmount.getValue();
	auto randNum = randNumber.getValue();
	std::array<gin::ModSrcId, 8> lfos{ proc.modSrcMonoLFO1, proc.modSrcMonoLFO2, proc.modSrcMonoLFO3, proc.modSrcMonoLFO4,
		proc.modSrcLFO1, proc.modSrcLFO2, proc.modSrcLFO3, proc.modSrcLFO4 };
	std::uniform_int_distribution<> lfoDist(0, 7);
	std::vector<gin::Parameter::Ptr> timbreBasic{
		proc.timbreParams.algo, proc.timbreParams.demodmix, proc.timbreParams.blend, proc.timbreParams.equant, proc.filterParams.frequency, proc.filterParams.resonance
	};
	if (inharmonic.getToggleState()) {
		timbreBasic.push_back(proc.timbreParams.pitch);
	}
	std::uniform_int_distribution<> timbreDist(0, timbreBasic.size() - 1);
	for (int i = 0; i < randNum; i++) {
		auto lfoSrc = lfos[lfoDist(gen)];
		auto timbreDst = gin::ModDstId(timbreBasic[timbreDist(gen)]->getModIndex());
		auto depth = proc.modMatrix.getModDepth(lfoSrc, timbreDst);
		auto sign = fullDist(gen) >= 0 ? 1 : -1;
		proc.modMatrix.setModDepth(lfoSrc, timbreDst, std::clamp(depth + sign * randVal, -1., 1.));
	}
}
void RandEditor::resetLFOtoTimbre()
{
	for (gin::Parameter::Ptr param : timbreDstsPlus) {
		auto modSrcs = proc.modMatrix.getModSources(param);
		if (modSrcs.size() == 0) continue;
		for (auto modSrc : modSrcs) {
			if (lfoSrcs.contains(modSrc))
				proc.modMatrix.clearModDepth(modSrc, gin::ModDstId(param->getModIndex()));
		}
	}
}

void RandEditor::randLFOtoLFO()
{
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> fullDist(-1.f, 1.f);
	auto randVal = randAmount.getValue();
	auto randNum = randNumber.getValue();
	std::uniform_int_distribution<> lfoDist(0, lfoSrcs.size() - 1);

	for (int i = 0; i < randNum; i++) {
		auto lfoSrc = lfoSrcs[lfoDist(gen)];
		auto lfoDst = gin::ModDstId(lfoDsts[lfoDist(gen)]->getModIndex());
		auto depth = proc.modMatrix.getModDepth(lfoSrc, lfoDst);
		auto sign = fullDist(gen) >= 0 ? 1 : -1;
		proc.modMatrix.setModDepth(lfoSrc, lfoDst, std::clamp(depth + sign * randVal, -1., 1.));
	}
}

void RandEditor::resetLFOtoLFO()
{
	for (gin::Parameter::Ptr param : lfoDsts) {
		auto modSrcs = proc.modMatrix.getModSources(param);
		if (modSrcs.size() == 0) continue;
		for (auto modSrc : modSrcs) {
			if (lfoSrcs.contains(modSrc))
				proc.modMatrix.clearModDepth(modSrc, gin::ModDstId(param->getModIndex()));
		}
	}
}
void RandEditor::randLFOtoENV()
{
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> fullDist(-1.f, 1.f);
	auto randVal = randAmount.getValue();
	auto randNum = randNumber.getValue();
	std::uniform_int_distribution<> envDist(0, envDsts.size() - 1);
	std::uniform_int_distribution<> lfoDist(0, lfoSrcs.size() - 1);

	for (int i = 0; i < randNum; i++) {
		auto lfoSrc = lfoSrcs[lfoDist(gen)];
		auto envDst = gin::ModDstId(envDsts[envDist(gen)]->getModIndex());
		auto depth = proc.modMatrix.getModDepth(lfoSrc, envDst);
		auto sign = fullDist(gen) >= 0 ? 1 : -1;
		proc.modMatrix.setModDepth(lfoSrc, envDst, std::clamp(depth + sign * randVal, -1., 1.));
	}
}
void RandEditor::resetLFOtoENV()
{
	for (gin::Parameter::Ptr param : envDsts) {
		auto modSrcs = proc.modMatrix.getModSources(param);
		if (modSrcs.size() == 0) continue;
		for (auto modSrc : modSrcs) {
			if (lfoSrcs.contains(modSrc))
				proc.modMatrix.clearModDepth(modSrc, gin::ModDstId(param->getModIndex()));
		}
	}
}
void RandEditor::randKeystoOSC()
{
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> fullDist(-1.f, 1.f);
	auto randVal = randAmount.getValue();
	auto randNum = randNumber.getValue();
	std::uniform_int_distribution<> keyDist(0, keySrcs.size() - 1);
	
	if (inharmonic.getToggleState()) {
		std::uniform_int_distribution<> oscDist(0, oscDstsPlus.size() - 1);
		for (int i = 0; i < randNum; i++) {
			auto keySrc = keySrcs[keyDist(gen)];
			auto oscDst = gin::ModDstId(oscDstsPlus[oscDist(gen)]->getModIndex());
			auto depth = proc.modMatrix.getModDepth(keySrc, oscDst);
			auto sign = fullDist(gen) >= 0 ? 1 : -1;
			proc.modMatrix.setModDepth(keySrc, oscDst, std::clamp(depth + sign * randVal, -1., 1.));
		}
	}
	else {
		std::uniform_int_distribution<> oscDist(0, oscDstsBasic.size() - 1);
		for (int i = 0; i < randNum; i++) {
			auto keySrc = keySrcs[keyDist(gen)];
			auto oscDst = gin::ModDstId(oscDstsBasic[oscDist(gen)]->getModIndex());
			auto depth = proc.modMatrix.getModDepth(keySrc, oscDst);
			auto sign = fullDist(gen) >= 0 ? 1 : -1;
			proc.modMatrix.setModDepth(keySrc, oscDst, std::clamp(depth + sign * randVal, -1., 1.));
		}
	}
		

}
void RandEditor::resetKeystoOSC()
{
	for (gin::Parameter::Ptr param : oscDstsPlus) {
		auto modSrcs = proc.modMatrix.getModSources(param);
		if (modSrcs.size() == 0) continue;
		for (auto modSrc : modSrcs) {
			if (keySrcs.contains(modSrc))
				proc.modMatrix.clearModDepth(modSrc, gin::ModDstId(param->getModIndex()));
		}
	}
}
void RandEditor::randKeystoTimbre()
{
}
void RandEditor::resetKeystoTimbre()
{
}
void RandEditor::randKeystoLFO()
{
}
void RandEditor::resetKeystoLFO()
{
}
void RandEditor::randKeystoENV()
{
}
void RandEditor::resetKeystoENV()
{
}
void RandEditor::randENVtoTimbre() {
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> fullDist(-1.f, 1.f);
	auto randVal = randAmount.getValue();
	auto randNum = randNumber.getValue();
	std::array<gin::ModSrcId, 4> envs{ proc.modSrcEnv1, proc.modSrcEnv2, proc.modSrcEnv3, proc.modSrcEnv4 };
	std::uniform_int_distribution<> envDist(0, 3);
	std::vector<gin::Parameter::Ptr> timbreBasic{
		proc.timbreParams.algo, proc.timbreParams.demodmix, proc.timbreParams.blend, proc.timbreParams.equant, proc.filterParams.frequency, proc.filterParams.resonance
	};
	if (inharmonic.getToggleState()) {
		timbreBasic.push_back(proc.timbreParams.pitch);
	}
	std::uniform_int_distribution<> timbreDist(0, timbreBasic.size() - 1);
	for (int i = 0; i < randNum; i++) {
		auto envSrc = envs[envDist(gen)];
		auto timbreDst = gin::ModDstId(timbreBasic[timbreDist(gen)]->getModIndex());
		auto depth = proc.modMatrix.getModDepth(envSrc, timbreDst);
		auto sign = fullDist(gen) >= 0 ? 1 : -1;
		proc.modMatrix.setModDepth(envSrc, timbreDst, std::clamp(depth + sign * randVal, -1., 1.));
	}
}

void RandEditor::resetENVtoTimbre()
{
}

void RandEditor::randENVtoLFO()
{
}

void RandEditor::resetENVtoLFO()
{
}

void RandEditor::randENVtoENV()
{
}

void RandEditor::resetENVtoENV()
{
}

void RandEditor::randomizeFXMods()
{
}

void RandEditor::clearFXMods()
{
}

void RandEditor::randomizeFXSelect()
{
}

void RandEditor::resized()
{
	matrix.setBounds(5, 5, 5*56, 8*70+23);

	randomizeButton.setBounds(5*56 + 5, 5, 3*56, 20);
	clearAllButton.setBounds(5 * 56 + 5, 30, 3*56, 20);
    randNumber.setBounds(5 * 56 + 5, 55, 3*56, 20);
	randNumberLabel.setBounds(5* 56 + 5 + 173, 55, 2*56, 20);
    randAmount.setBounds(5 * 56 + 5, 80, 3*56, 20);
	randAmountLabel.setBounds(5 * 56 + 5 + 173, 80, 2*56, 20);

    randOSCsButton.setBounds(5 * 56 + 5, 105, 3*56, 20);
    inharmonic.setBounds(5 * 56 + 5 + 173, 105, 2*56, 20);
	


    randInharmonicButton.setBounds(5 * 56 + 5, 130, 3*56, 20);
	resetInharmonicButton.setBounds(5 * 56 + 5 + 173, 130, 3 * 56, 20);

    randENVsButton.setBounds(5 * 56 + 5, 155, 3*56, 20);
	resetENVsButton.setBounds(5 * 56 + 5 + 173, 155, 3*56, 20);
	
	randLFOsButton.setBounds(5 * 56 + 5, 180, 3*56, 20);
	resetLFOsButton.setBounds(5 * 56 + 5 + 173, 180, 3*56, 20);

	randLFOToOSCButton.setBounds(5 * 56 + 5, 205, 3*56, 20);
	clearLFOToOSCButton.setBounds(5 * 56 + 5 + 173, 205, 3*56, 20);

	randENVToOSCButton.setBounds(5 * 56 + 5, 230, 3*56, 20);
	clearENVToOSCButton.setBounds(5 * 56 + 5 + 173, 230, 3*56, 20);

	randKeysToOSCButton.setBounds(5 * 56 + 5, 255, 3*56, 20);
	clearKeysToOSCButton.setBounds(5 * 56 + 5 + 173, 255, 3*56, 20);

	randLFOToTimbreButton.setBounds(5 * 56 + 5, 280, 3*56, 20);
	clearLFOToTimbreButton.setBounds(5 * 56 + 5 + 173, 280, 3*56, 20);

	randENVToTimbreButton.setBounds(5 * 56 + 5, 305, 3*56, 20);
	clearENVToTimbreButton.setBounds(5 * 56 + 5 + 173, 305, 3*56, 20);

	randKeysToTimbreButton.setBounds(5 * 56 + 5, 330, 3*56, 20);
	clearKeysToTimbreButton.setBounds(5 * 56 + 5 + 173, 330, 3*56, 20);

	randLFOToENVButton.setBounds(5 * 56 + 5, 355, 3*56, 20);
	clearLFOToENVButton.setBounds(5 * 56 + 5 + 173, 355, 3*56, 20);

	randENVToENVButton.setBounds(5 * 56 + 5, 380, 3*56, 20);
	clearENVToENVButton.setBounds(5 * 56 + 5 + 173, 380, 3*56, 20);

	randKeysToENVButton.setBounds(5 * 56 + 5, 405, 3*56, 20);
	clearKeysToENVButton.setBounds(5 * 56 + 5 + 173, 405, 3*56, 20);

	randLFOToLfoButton.setBounds(5 * 56 + 5, 430, 3*56, 20);
	clearLFOToLFOButton.setBounds(5 * 56 + 5 + 173, 430, 3*56, 20);

	randENVToLFOButton.setBounds(5 * 56 + 5, 455, 3*56, 20);
	clearENVToLFOButton.setBounds(5 * 56 + 5 + 173, 455, 3*56, 20);

	randKeysToLFOButton.setBounds(5 * 56 + 5, 480, 3*56, 20);
	clearKeysToLFOButton.setBounds(5 * 56 + 5 + 173, 480, 3*56, 20);



	randFXModsButton.setBounds(5 * 56 + 5, 505, 3*56, 20);
	clearFXModsButton.setBounds(5 * 56 + 5 + 173, 505, 3*56, 20);

    test.setBounds(800, 23, 55, 20);
}
