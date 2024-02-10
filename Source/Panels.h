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

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "APColors.h"
#include "MoonKnob.h"



//==============================================================================
class OscillatorBox : public gin::ParamBox
{
public:
    OscillatorBox (const juce::String& name, APAudioProcessor& proc_, APAudioProcessor::OSCParams& oscparams_)
        : gin::ParamBox (name), proc (proc_), osc(oscparams_)
    {
        setName ( name );

        addControl (c = new gin::Knob (osc.coarse), 0, 0);
        addControl (f = new gin::Knob (osc.fine, true), 1, 0);
        addControl (r = new gin::Knob (osc.volume), 2, 0);
        addControl (new gin::Knob (osc.tones), 3, 0);
		addControl(new gin::Switch(osc.saw), 4, 0);
		addControl(phaseKnob = new MoonKnob(osc.phase), 5, 0);

		
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

        addControl (new gin::Knob (osc.detune), 2, 2);
        addControl (new gin::Knob (osc.spread), 3, 2);
        addControl (new gin::Knob (osc.pan, true), 4, 2);

        addControl(new gin::Select(osc.env));
		addControl(new gin::Switch(osc.fixed));
		
		watchParam(osc.fixed);
		watchParam(osc.saw);
		watchParam(osc.coarse);
		watchParam(osc.fine);

		addAndMakeVisible(fixedHz);
		//addAndMakeVisible(phaseKnob);

		fixedHz.setJustificationType(Justification::centred);
    }

    ~OscillatorBox() override
    {
		c->setLookAndFeel(nullptr);
		f->setLookAndFeel(nullptr);
		r->setLookAndFeel(nullptr);
    }

	class APLookAndFeel1 : public gin::CopperLookAndFeel
	{
	public:
		APLookAndFeel1(){
			setColour(juce::Slider::rotarySliderFillColourId, APColors::redLight);
			setColour(juce::Slider::trackColourId, APColors::redMuted);
		}
	};
	class APLookAndFeel2 : public gin::CopperLookAndFeel
	{
	public:
		APLookAndFeel2() {
			setColour(juce::Slider::rotarySliderFillColourId, APColors::yellowLight);
			setColour(juce::Slider::trackColourId, APColors::yellowMuted);
		}
	};
	class APLookAndFeel3 : public gin::CopperLookAndFeel
	{
	public:
		APLookAndFeel3() {
			setColour(juce::Slider::rotarySliderFillColourId, APColors::greenLight);
			setColour(juce::Slider::trackColourId, APColors::greenMuted);
		}
	};
	class APLookAndFeel4 : public gin::CopperLookAndFeel
	{
	public:
		APLookAndFeel4() {
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

		fixedHz.setBounds(56, 76+23, 56, 35);
		phaseKnob->setBounds(56*4, 23, 56, 56);
	}
	APLookAndFeel1 lnf1;
	APLookAndFeel2 lnf2;
	APLookAndFeel3 lnf3;
	APLookAndFeel4 lnf4;
    APAudioProcessor& proc;
    APAudioProcessor::OSCParams& osc;
	gin::ParamComponent::Ptr c = nullptr, f = nullptr, r = nullptr;
	Label fixedHz;
	MoonKnob* phaseKnob;
};

//==============================================================================
class ENVBox : public gin::ParamBox
{
public:
    ENVBox (const juce::String& name, APAudioProcessor& proc_, APAudioProcessor::ENVParams& envparams_)
        : gin::ParamBox (name), proc (proc_), envparams(envparams_)
    {
        setName ( name );

		switch (envparams.num) {
		case 1:
			addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcEnv1, true));
			break;
		case 2:
			addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcEnv2, true));
			break;
		case 3:
			addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcEnv3, true));
			break;
		case 4:
			addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcEnv4, true));
			break;
		}

        //auto& preset = envparams;

        addControl (new gin::Knob (envparams.attack), 0, 1);
        addControl (new gin::Knob (envparams.decay), 1, 1);
        addControl (new gin::Knob (envparams.sustain), 2, 1);
        addControl (new gin::Knob (envparams.release), 3, 1);
        addControl (new gin::Knob (envparams.velocityTracking), 4, 1);
		addControl(new gin::Knob(envparams.acurve, true), 0, 2);
		addControl(new gin::Knob(envparams.drcurve), 1, 2);
		addControl(r = new gin::Knob(envparams.time, true), 2, 2);
		addControl(b =new gin::Select(envparams.duration), 3, 2);
		addControl(new gin::Select(envparams.syncrepeat), 4, 2);
		watchParam(envparams.syncrepeat);
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

	APAudioProcessor& proc;
	gin::ParamComponent::Ptr r = nullptr;
	gin::ParamComponent::Ptr b = nullptr;
	APAudioProcessor::ENVParams& envparams;
};

//==============================================================================
class TimbreBox : public gin::ParamBox
{
public:
	TimbreBox(const juce::String& name, APAudioProcessor& proc)
		: gin::ParamBox(name)
	{
		setName(name);

		auto& timbreparams = proc.timbreParams;
		addControl(new gin::Knob(timbreparams.equant, true), 0, 0);
		addControl(new gin::Knob(timbreparams.pitch, true), 0, 1);
		addControl(new gin::Knob(timbreparams.blend), 1, 1);
        addControl(new gin::Knob(timbreparams.algo), 1, 0);
		addControl(new gin::Knob(timbreparams.demodmix, true), 1, 2);
	}
};

//==============================================================================
class FilterBox : public gin::ParamBox
{
public:
    FilterBox (const juce::String& name, APAudioProcessor& proc_)
        : gin::ParamBox (name), proc (proc_)
    {
        setName ( "flt" );

        auto& flt = proc.filterParams;

        addEnable (flt.enable);

        auto freq = new gin::Knob (flt.frequency);
        addControl (freq, 0, 0);
        addControl (new gin::Knob (flt.resonance), 1, 0);

        addControl (new gin::Knob (flt.keyTracking), 0, 1);
        addControl (new gin::Select (flt.type), 1, 1);

        freq->setLiveValuesCallback ([this] ()
        {
            if (proc.filterParams.keyTracking->getUserValue() != 0.0f ||
                proc.modMatrix.isModulated (gin::ModDstId (proc.filterParams.frequency->getModIndex())))
                return proc.getLiveFilterCutoff();
            return juce::Array<float>();
        });
    }

    APAudioProcessor& proc;
};

//==============================================================================
class LFOBox : public gin::ParamBox
{
public:
    LFOBox (const juce::String& num, APAudioProcessor& proc_, APAudioProcessor::LFOParams& lfoparams_, gin::ModSrcId& modsrcID, gin::ModSrcId& monoID)
        : gin::ParamBox(num), proc(proc_), lfoparams(lfoparams_)
    {
        setName(num);

		addEnable(lfoparams.enable);

        addModSource (new gin::ModulationSourceButton (proc.modMatrix, monoID, false));
        addModSource (new gin::ModulationSourceButton (proc.modMatrix, modsrcID, true));

        addControl (new gin::Switch (lfoparams.sync), 3, 1);
        addControl (new gin::Select (lfoparams.wave), 2, 1);
        addControl (r = new gin::Knob (lfoparams.rate), 0, 0);
        addControl (b = new gin::Select (lfoparams.beat), 0, 0);
        addControl (new gin::Knob (lfoparams.depth, true), 1, 0);

        addControl (new gin::Knob (lfoparams.phase, true), 4, 1);
        addControl (new gin::Knob (lfoparams.offset, true), 5, 1);
        addControl (new gin::Knob (lfoparams.fade, true), 0, 1);
        addControl (new gin::Knob (lfoparams.delay), 1, 1);


        auto l = new gin::LFOComponent();
        l->phaseCallback = [this, num]   
        {
            std::vector<float> res;
			auto unit = num.getTrailingIntValue() - 1;
            res.push_back (proc.monoLFOs[unit]->getCurrentPhase());
            return res;
        };
        l->setParams (lfoparams.wave, lfoparams.sync, lfoparams.rate, lfoparams.beat, lfoparams.depth, 
            lfoparams.offset, lfoparams.phase, lfoparams.enable);
        addControl (l, 2, 0, 4, 1);

        watchParam (lfoparams.sync);

        setSize (112, 163);
    }

    void paramChanged () override
    {
        gin::ParamBox::paramChanged ();

        if (r && b)
        {
            r->setVisible (! lfoparams.sync->isOn());
            b->setVisible (lfoparams.sync->isOn());
        }
    }

    APAudioProcessor& proc;
    gin::ParamComponent::Ptr r = nullptr;
    gin::ParamComponent::Ptr b = nullptr;
    APAudioProcessor::LFOParams& lfoparams;
};

//==============================================================================
class ModBox : public gin::ParamBox
{
public:
    ModBox (const juce::String& name, APAudioProcessor& proc_)
        : gin::ParamBox (name), proc (proc_)
    {
        setName ("mod");
        setTitle("mod sources");
        addControl (new gin::ModSrcListBox (proc.modMatrix), 0, 0, 3, 2);
    }

    APAudioProcessor& proc;
};

//==============================================================================
class GlobalBox : public gin::ParamBox
{
public:
    GlobalBox (const juce::String& name, APAudioProcessor& proc_)
        : gin::ParamBox (name), proc (proc_)
    {
        setName ("global");

        addControl (new gin::Knob (proc.globalParams.level), 2, 1);
        addControl (new gin::Select (proc.globalParams.glideMode), 1, 0);
        addControl (new gin::Knob (proc.globalParams.glideRate), 2, 0);

        addControl (new gin::Knob (proc.globalParams.velSens), 1, 1);
        addControl (new gin::Switch (proc.globalParams.legato), 1, 1);
        addControl (new gin::Switch (proc.globalParams.mono), 2, 1);
    }

    APAudioProcessor& proc;
};

//==============================================================================
class OrbitBox : public gin::ParamBox 
{
public:
	OrbitBox(const juce::String& name, APAudioProcessor& proc_)
		: gin::ParamBox(name), proc(proc_)
	{
		setName("orbit");

		addControl(new gin::Knob(proc.orbitParams.speed), 0, 0);
		addControl(new gin::Knob(proc.orbitParams.scale), 1, 0);
		addControl(new gin::Select(proc.timbreParams.algo), 0, 1);
	}

	APAudioProcessor& proc;
};


