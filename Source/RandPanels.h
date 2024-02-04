#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "EnvelopeComponent.h"

class RandMatrixBox : public gin::ParamBox
{
public:
	RandMatrixBox(const juce::String& name, APAudioProcessor& proc_)
		: gin::ParamBox(name), proc(proc_)
	{
		setName("mtx");

		addControl(new gin::ModMatrixBox(proc, proc.modMatrix), 0, 0, 5, 8);
	}

	APAudioProcessor& proc;
};

class RandENVBox : public gin::ParamBox
{
public:
	RandENVBox(const juce::String& name, APAudioProcessor& proc_, APAudioProcessor::ENVParams& envparams_)
		: gin::ParamBox(name), proc(proc_), envparams(envparams_), display(proc, envparams.num)
	{
		setName(name);
		addControl(new gin::Knob(envparams.attack), 0, 0);
		addControl(new gin::Knob(envparams.decay), 1, 0);
		addControl(new gin::Knob(envparams.sustain), 2, 0);
		addControl(new gin::Knob(envparams.release), 3, 0);
		addControl(new gin::Knob(envparams.acurve, true), 4, 0);
		addControl(new gin::Knob(envparams.drcurve), 5, 0);
		addControl(new gin::Select(envparams.syncrepeat), 0, 1);
		addControl(r = new gin::Knob(envparams.time, true), 1, 1);
		addControl(b = new gin::Select(envparams.duration), 1, 1);
		watchParam(envparams.syncrepeat);
		addAndMakeVisible(display);
	}

	void paramChanged() override
	{
		gin::ParamBox::paramChanged();

		if (r && b)
		{
			auto choice = envparams.syncrepeat->getUserValueInt();
			r->setVisible(choice == 2);
			b->setVisible(choice == 1);
		}
	}

	void resized() override
	{
		gin::ParamBox::resized();
		display.setBounds(113, 93, 223, 70);
	}

	APAudioProcessor& proc;
	gin::ParamComponent::Ptr r = nullptr;
	gin::ParamComponent::Ptr b = nullptr;
	APAudioProcessor::ENVParams& envparams;
	EnvelopeComponent display;
};