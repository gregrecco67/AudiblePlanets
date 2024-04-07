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
#include "EnvelopeComponent.h"
#include "APColors.h"
#include "MoonKnob.h"
#include "APModAdditions.h"

class RandMatrixBox : public gin::ParamBox
{
public:
	RandMatrixBox(const juce::String& name, APAudioProcessor& proc_)
		: gin::ParamBox(name), proc(proc_)
	{
		setName("mtx");

		addControl(new APModMatrixBox(proc, proc.modMatrix), 0, 0, 5, 8);
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
		addControl(new gin::Knob(envparams.drcurve, true), 5, 0);
		addControl(new gin::Select(envparams.syncrepeat), 0, 1);
		addControl(r = new gin::Knob(envparams.time), 1, 1);
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
		if (osc.num == 1) {
			addControl(f = new gin::Knob(osc.fine), 1, 0);
		}
		else {
			addControl(f = new gin::Knob(osc.fine, true), 1, 0);
		}
		addControl(r = new gin::Knob(osc.volume), 2, 0);
		addControl(new gin::Knob(osc.tones), 3, 0);
		addControl(new MoonKnob(osc.phase), 1, 1);


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
		addControl(new gin::Knob(osc.spread, true), 3, 1);

		addControl(env = new gin::Select(osc.env), 0, 1);
		addControl(fixed = new gin::Switch(osc.fixed));
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


	void resized() override {
		gin::ParamBox::resized();
		env->setBounds(0, 70 + 23 + 35, 56, 35);
		fixed->setBounds(0, 70 + 23, 56, 35);
	}
	rAPLookAndFeel1 lnf1;
	rAPLookAndFeel2 lnf2;
	rAPLookAndFeel3 lnf3;
	rAPLookAndFeel4 lnf4;
	APAudioProcessor& proc;
	APAudioProcessor::OSCParams& osc;
	gin::ParamComponent::Ptr c = nullptr, f = nullptr, r = nullptr, fixed = nullptr, env = nullptr;
};
