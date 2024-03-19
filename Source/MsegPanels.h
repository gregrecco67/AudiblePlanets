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

 //==============================================================================
class MsegModBox : public gin::ParamBox
{
public:
	MsegModBox(const juce::String& name, APAudioProcessor& proc_)
		: gin::ParamBox(name), proc(proc_)
	{
		setName("msegmod");
		setTitle("mod sources");
		addControl(new gin::ModSrcListBox(proc.modMatrix), 0, 0, 5, 4);
	}

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

		addControl(new gin::ModMatrixBox(proc, proc.modMatrix), 0, 0, 5, 4.224);
	}

	APAudioProcessor& proc;

};

class MsegBox : public gin::ParamBox
{
public:
	MsegBox(const juce::String& name, APAudioProcessor& proc_, APAudioProcessor::MSEGParams& msegParams_, gin::MSEG::Data& data_)
		: gin::ParamBox(name), proc(proc_), msegParams(msegParams_), msegComponent(data_)
	{
		setName(name);
		//addMouseListener(this, false);
		msegComponent.setParams(msegParams.sync, msegParams.rate, msegParams.beat, msegParams.depth, msegParams.offset,
			msegParams.phase, msegParams.enable, msegParams.xgrid, msegParams.ygrid, msegParams.loop);
		msegComponent.setEditable(true);
		msegComponent.setDrawMode(true, static_cast<gin::MSEGComponent::DrawMode>(msegParams.drawmode->getUserValue()));
		addControl(r = new gin::Knob(msegParams.rate), 0, 0);
		addControl(b = new gin::Select(msegParams.beat), 0, 0);
		addControl(new gin::Switch(msegParams.sync), 1, 0);
		addControl(new gin::Switch(msegParams.loop), 2, 0);
		addControl(new gin::Knob(msegParams.depth), 3, 0);
		addControl(new gin::Knob(msegParams.offset), 4, 0);
		addControl(new gin::Switch(msegParams.draw), 5, 0);
		addControl(new gin::Select(msegParams.drawmode), 6, 0);
		watchParam(msegParams.sync);
		watchParam(msegParams.draw);
		watchParam(msegParams.drawmode);
		addAndMakeVisible(msegComponent);
		switch (msegParams.num) {
			case 1:
				msegComponent.phaseCallback = [this]() { return proc.synth.getMSEG1Phases(); };
				break;
			case 2:
				msegComponent.phaseCallback = [this]() { return proc.synth.getMSEG2Phases(); };
				break;
			case 3:
				msegComponent.phaseCallback = [this]() { return proc.synth.getMSEG3Phases(); };
				break;
			case 4:
				msegComponent.phaseCallback = [this]() { return proc.synth.getMSEG4Phases(); };
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
	}

	APAudioProcessor& proc;
	gin::ParamComponent::Ptr r = nullptr;
	gin::ParamComponent::Ptr b = nullptr;
	APAudioProcessor::MSEGParams& msegParams;
	gin::MSEGComponent msegComponent;
};

