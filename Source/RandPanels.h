#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "EnvelopeComponent.h"
#include "APColors.h"

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

//==============================================================================
class RandOSCBox : public gin::ParamBox
{
public:
	RandOSCBox(const juce::String& name, APAudioProcessor& proc_, APAudioProcessor::OSCParams& oscparams_)
		: gin::ParamBox(name), proc(proc_), osc(oscparams_)
	{
		setName(name);

		addControl(c = new gin::Knob(osc.coarse), 0, 0);
		addControl(f = new gin::Knob(osc.fine, true), 1, 0);
		addControl(r = new gin::Knob(osc.volume), 2, 0);
		addControl(new gin::Knob(osc.tones), 3, 0);
		


		switch (osc.num) {
		case 1:
			c->setLookAndFeel(&lnf1);
			f->setLookAndFeel(&lnf1);
			r->setLookAndFeel(&lnf1);
			break;
		case 2:
			c->setLookAndFeel(&lnf2);
			f->setLookAndFeel(&lnf2);
			r->setLookAndFeel(&lnf2);
			break;
		case 3:
			c->setLookAndFeel(&lnf3);
			f->setLookAndFeel(&lnf3);
			r->setLookAndFeel(&lnf3);
			break;
		case 4:
			c->setLookAndFeel(&lnf4);
			f->setLookAndFeel(&lnf4);
			r->setLookAndFeel(&lnf4);
			break;
		}

		addControl(new gin::Knob(osc.detune), 2, 1);
		addControl(new gin::Knob(osc.spread), 3, 1);

		addControl(new gin::Select(osc.env), 0, 1);
		addControl(fixed = new gin::Switch(osc.fixed));

		watchParam(osc.fixed);
		watchParam(osc.saw);
		watchParam(osc.coarse);
		watchParam(osc.fine);

		addAndMakeVisible(fixedHz);

		fixedHz.setJustificationType(Justification::centred);
	}

	~RandOSCBox() override
	{
		c->setLookAndFeel(nullptr);
		f->setLookAndFeel(nullptr);
		r->setLookAndFeel(nullptr);
	}

	class rAPLookAndFeel1 : public gin::CopperLookAndFeel
	{
	public:
		rAPLookAndFeel1() {
			setColour(juce::Slider::rotarySliderFillColourId, APColors::redLight);
			setColour(juce::Slider::trackColourId, APColors::redMuted);
		}
	};
	class rAPLookAndFeel2 : public gin::CopperLookAndFeel
	{
	public:
		rAPLookAndFeel2() {
			setColour(juce::Slider::rotarySliderFillColourId, APColors::yellowLight);
			setColour(juce::Slider::trackColourId, APColors::yellowMuted);
		}
	};
	class rAPLookAndFeel3 : public gin::CopperLookAndFeel
	{
	public:
		rAPLookAndFeel3() {
			setColour(juce::Slider::rotarySliderFillColourId, APColors::greenLight);
			setColour(juce::Slider::trackColourId, APColors::greenMuted);
		}
	};
	class rAPLookAndFeel4 : public gin::CopperLookAndFeel
	{
	public:
		rAPLookAndFeel4() {
			setColour(juce::Slider::rotarySliderFillColourId, APColors::blueLight);
			setColour(juce::Slider::trackColourId, APColors::blueMuted);
		}
	};


	void paramChanged() override {
		gin::ParamBox::paramChanged();
		if (osc.fixed->isOn()) {
			fixedHz.setVisible(true);
			fixedHz.setText(String((osc.coarse->getUserValue() + osc.fine->getUserValue()) * 100) + String(" Hz"), juce::dontSendNotification);
		}
		else {
			fixedHz.setVisible(false);
		}
	}

	void resized() override {
		gin::ParamBox::resized();

		fixedHz.setBounds(56, 76 + 23, 56, 35);
		fixed->setBounds(56, 70 + 35 + 23, 56, 35);
	}
	rAPLookAndFeel1 lnf1;
	rAPLookAndFeel2 lnf2;
	rAPLookAndFeel3 lnf3;
	rAPLookAndFeel4 lnf4;
	APAudioProcessor& proc;
	APAudioProcessor::OSCParams& osc;
	gin::ParamComponent::Ptr c = nullptr, f = nullptr, r = nullptr, fixed = nullptr;
	Label fixedHz;
};