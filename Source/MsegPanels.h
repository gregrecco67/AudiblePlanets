/*
 * Audible Planets - an expressive, quasi-Ptolemaic semi-modular synthesizer
 *
 * Copyright 2024, Greg Recco
 *
 * Audible Planets is released under the GNU General Public Licence v3
 * or later (GPL-3.0-or-later). The license is found in the "LICENSE"
 * file in the root of this repository, or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Audible Planets is available at
 * https://github.com/gregrecco67/AudiblePlanets
 */

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "APModAdditions.h"

 //==============================================================================
class MsegModBox : public gin::ParamBox
{
public:
	MsegModBox(const juce::String& name, APAudioProcessor& proc_)
		: gin::ParamBox(name), proc(proc_)
	{
		setName("msegmod");
		setTitle("mod sources");
		addControl(srclist = new gin::ModSrcListBox(proc.modMatrix), 0, 0, 5, 4.286);
        srclist->setRowHeight(20);
	}
    gin::ModSrcListBox* srclist;
	APAudioProcessor& proc;
};


//==============================================================================
class MsegMatrixBox : public gin::ParamBox
{
public:
	MsegMatrixBox(const juce::String& name, APAudioProcessor& proc_)
		: gin::ParamBox(name), proc(proc_)
	{
		setName("msegmtx");
		addAndMakeVisible(clearAllButton);
		addControl(new APModMatrixBox(proc, proc.modMatrix), 0, 0, 5, 4.286);
		clearAllButton.onClick = [this] { clearAll(); };
	}

	void resized() override {
		gin::ParamBox::resized();
		clearAllButton.setBounds(getWidth() - 60, 0, 55, 23);
	}

	void clearAll() {
		auto& params = proc.getPluginParameters();
		for (auto* param : params) {
			if (param->getModIndex() == -1) continue;
			if (proc.modMatrix.isModulated(gin::ModDstId(param->getModIndex()))) {
				auto modSrcs = proc.modMatrix.getModSources(param);
				for (auto& modSrc : modSrcs) {
					proc.modMatrix.clearModDepth(modSrc, gin::ModDstId(param->getModIndex()));
				}
			}
		}
	}

	TextButton clearAllButton{ "Clear All" };
	APAudioProcessor& proc;

};

class MsegBox : public gin::ParamBox
{
public:
	MsegBox(const juce::String& name,
		APAudioProcessor& proc_,
		APAudioProcessor::MSEGParams& msegParams_,
		gin::MSEG::Data& data_,
		gin::ModSrcId msegSrc_)
		: gin::ParamBox(name), proc(proc_), msegParams(msegParams_), msegComponent(data_), msegSrc(msegSrc_)
	{
		setName(name);
		//addMouseListener(this, false);
		msegComponent.setParams(msegParams.sync, msegParams.rate, msegParams.beat, msegParams.depth, msegParams.offset,
			msegParams.phase, msegParams.enable, msegParams.xgrid, msegParams.ygrid, msegParams.loop);
		msegComponent.setEditable(true);
		msegComponent.setDrawMode(true, static_cast<gin::MSEGComponent::DrawMode>(msegParams.drawmode->getUserValue()));
		addControl(r = new APKnob(msegParams.rate), 0, 0);
		addControl(b = new gin::Select(msegParams.beat), 0, 0);
		addControl(new gin::Select(msegParams.sync), 1, 0);
		addControl(new gin::Select(msegParams.loop), 2, 0);
		addControl(new APKnob(msegParams.depth), 3, 0);
		addControl(new APKnob(msegParams.offset), 4, 0);
		addControl(new gin::Select(msegParams.draw), 5, 0);
		addControl(new gin::Select(msegParams.drawmode), 6, 0);
		addControl(x = new gin::Select(msegParams.xgrid));
		addControl(y = new gin::Select(msegParams.ygrid));
		watchParam(msegParams.sync);
		watchParam(msegParams.draw);
		watchParam(msegParams.drawmode);
		addAndMakeVisible(msegComponent);
		addAndMakeVisible(msegDstSelector);
		switch (msegParams.num) {
		case 1:
			msegComponent.phaseCallback = [this]() { 
				std::vector<float> auxVals = proc.auxSynth.getMSEG1Phases();
				std::vector<float> synthVals = proc.synth.getMSEG1Phases();
				synthVals.insert(synthVals.end(), auxVals.begin(), auxVals.end());
				return synthVals;
				};
			break;
		case 2:
                msegComponent.phaseCallback = [this]() {
                    std::vector<float> auxVals = proc.auxSynth.getMSEG2Phases();
                    std::vector<float> synthVals = proc.synth.getMSEG2Phases();
                    synthVals.insert(synthVals.end(), auxVals.begin(), auxVals.end());
                    return synthVals;
                };
			break;
		case 3:
                msegComponent.phaseCallback = [this]() {
                    std::vector<float> auxVals = proc.auxSynth.getMSEG3Phases();
                    std::vector<float> synthVals = proc.synth.getMSEG3Phases();
                    synthVals.insert(synthVals.end(), auxVals.begin(), auxVals.end());
                    return synthVals;
                };
			break;
		case 4:
                msegComponent.phaseCallback = [this]() {
                    std::vector<float> auxVals = proc.auxSynth.getMSEG4Phases();
                    std::vector<float> synthVals = proc.synth.getMSEG4Phases();
                    synthVals.insert(synthVals.end(), auxVals.begin(), auxVals.end());
                    return synthVals;
                };
			break;
		}
	}

	void paramChanged() override
	{
		gin::ParamBox::paramChanged();

		if (r && b)
		{
			auto sync = msegParams.sync->getUserValueBool();
			r->setVisible(!sync);
			b->setVisible(sync);
		}

		if (msegParams.draw->getUserValueBool())
		{
			msegComponent.setDrawMode(true, static_cast<gin::MSEGComponent::DrawMode>(msegParams.drawmode->getUserValue()));
		}
		else
		{
			msegComponent.setDrawMode(false, gin::MSEGComponent::DrawMode::step);
		}
	}

	void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) {
		float xgrid = msegParams.xgrid->getUserValue();
		float ygrid = msegParams.ygrid->getUserValue();
		if (event.getPosition().y < 93) return;
		if (!juce::ModifierKeys::currentModifiers.isAnyModifierKeyDown()) {
			if (wheel.deltaY > 0) {
				if (!wheel.isReversed)
				{
					msegParams.xgrid->setUserValue(xgrid + 1.f);
				}
				else
				{
					msegParams.xgrid->setUserValue(xgrid - 1.f);
				}
			}
			if (wheel.deltaY <= 0) {
				if (!wheel.isReversed)
				{
					msegParams.xgrid->setUserValue(xgrid - 1.f);
				}
				else
				{
					msegParams.xgrid->setUserValue(xgrid + 1.f);
				}
			}
		}
		else {
			if (wheel.deltaY > 0) {
				if (!wheel.isReversed)
				{
					msegParams.ygrid->setUserValue(ygrid + 1.f);
				}
				else
				{
					msegParams.ygrid->setUserValue(ygrid - 1.f);
				}
			}
			if (wheel.deltaY <= 0) {
				if (!wheel.isReversed)
				{
					msegParams.ygrid->setUserValue(ygrid - 1.f);
				}
				else
				{
					msegParams.ygrid->setUserValue(ygrid + 1.f);
				}
			}
		}
		repaint();
	}

	void resized() override {
		gin::ParamBox::resized();
		msegComponent.setBounds(0, 93, getWidth(), getHeight() - 93);
		x->setBounds(7 * 56, 23, 56, 35);
		y->setBounds(7 * 56, 58, 56, 35);
		msegDstSelector.setBounds(5, 0, 55, 23);
	}

	APAudioProcessor& proc;
	gin::ParamComponent::Ptr r = nullptr;
	gin::ParamComponent::Ptr b = nullptr;
	gin::ParamComponent::Ptr x = nullptr;
	gin::ParamComponent::Ptr y = nullptr;
	gin::ModSrcId msegSrc;
	ParameterSelector msegDstSelector{proc, msegSrc};
	APAudioProcessor::MSEGParams& msegParams;
	gin::MSEGComponent msegComponent;
};

