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
#include "APModAdditions.h"



//==============================================================================
class OscillatorBox : public gin::ParamBox
{
public:
    OscillatorBox(const juce::String& name, APAudioProcessor& proc_, APAudioProcessor::OSCParams& oscparams_)
        : gin::ParamBox(name), proc(proc_), osc(oscparams_)
    {
        setName( name );

        addControl(c = new APKnob(osc.coarse), 0, 0);
		if (osc.num == 1) {
			addControl(f = new APKnob(osc.fine), 1, 0);
		}
		else {
	        addControl(f = new APKnob(osc.fine, true), 1, 0);
		}
        addControl(r = new APKnob(osc.volume), 2, 0);
        addControl(new APKnob(osc.tones), 3, 0);
		addControl(phaseKnob = new MoonKnob(osc.phase), 4, 0);

		
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

        addControl(new APKnob(osc.detune), 2, 1);
        addControl(new APKnob(osc.spread), 3, 1);
        addControl(new APKnob(osc.pan, true), 4, 1);

        addControl(saw = new gin::Select(osc.saw));
        addControl(env = new gin::Select(osc.env));
		addControl(fixed = new gin::Select(osc.fixed));
		
		watchParam(osc.fixed);
		watchParam(osc.saw);
		watchParam(osc.coarse);
		watchParam(osc.fine);

		addAndMakeVisible(fixedHz);

		fixedHz.setJustificationType(Justification::centred);
    }

    ~OscillatorBox() override
    {
		c->setLookAndFeel(nullptr);
		f->setLookAndFeel(nullptr);
		r->setLookAndFeel(nullptr);
        //fixedHz.setLookAndFeel(nullptr);
    }

	class APLookAndFeel1 : public APLNF
	{
	public:
		APLookAndFeel1(){
			setColour(juce::Slider::rotarySliderFillColourId, APColors::red);
			setColour(juce::Slider::trackColourId, APColors::redMuted);
		}
	};
	class APLookAndFeel2 : public APLNF
	{
	public:
		APLookAndFeel2() {
			setColour(juce::Slider::rotarySliderFillColourId, APColors::yellow);
			setColour(juce::Slider::trackColourId, APColors::yellowMuted);
		}
	};
	class APLookAndFeel3 : public APLNF
	{
	public:
		APLookAndFeel3() {
			setColour(juce::Slider::rotarySliderFillColourId, APColors::green);
			setColour(juce::Slider::trackColourId, APColors::greenMuted);
		}
	};
	class APLookAndFeel4 : public APLNF
	{
	public:
		APLookAndFeel4() {
			setColour(juce::Slider::rotarySliderFillColourId, APColors::blue);
			setColour(juce::Slider::trackColourId, APColors::blueMuted);
		}
	};

	void paramChanged() override {
		gin::ParamBox::paramChanged();
		if (osc.fixed->isOn()) {
			fixedHz.setVisible(true);
			fixedHz.setText(String((osc.coarse->getUserValue() + osc.fine->getUserValue()) * 100, 2) + String(" Hz"), juce::dontSendNotification);
		}
		else {
			fixedHz.setVisible(false);
		}
	}

	void resized() override {
		gin::ParamBox::resized();
		fixedHz.setBounds(56, 23+70+10, 56, 15); // 23/70/10 header, first row, padding
		phaseKnob->setBounds(56*4, 23, 56, 70);
        saw->setBounds(0, 93, 56, 35);
        env->setBounds(0, 128, 56, 35);
        fixed->setBounds(56, 128, 56, 35);
        
	}
    
	APLookAndFeel1 lnf1;
	APLookAndFeel2 lnf2;
	APLookAndFeel3 lnf3;
	APLookAndFeel4 lnf4;
    APAudioProcessor& proc;
    APAudioProcessor::OSCParams& osc;
	gin::ParamComponent::Ptr c = nullptr, f = nullptr, r = nullptr, saw = nullptr,
        env = nullptr, fixed = nullptr;
	Label fixedHz;
	MoonKnob* phaseKnob;
    //juce::Font regularFont{ juce::Typeface::createSystemTypefaceFor(BinaryData::latoregular_otf, BinaryData::latoregular_otfSize) };
};

//==============================================================================
class ENVBox : public gin::ParamBox
{
public:
    ENVBox(const juce::String& name, APAudioProcessor& proc_, APAudioProcessor::ENVParams& envparams_)
        : gin::ParamBox(name), proc(proc_), envparams(envparams_)
    {
        setName(name);

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

        addControl(new APKnob(envparams.attack), 0, 0);
        addControl(new APKnob(envparams.decay), 1, 0);
        addControl(new APKnob(envparams.sustain), 2, 0);
        addControl(new APKnob(envparams.release), 3, 0);
		addControl(new APKnob(envparams.acurve, true), 0, 1);
		addControl(new APKnob(envparams.drcurve, true), 1, 1);
        addControl(new gin::Select(envparams.syncrepeat), 2, 1);
        addControl(r = new APKnob(envparams.time), 3, 1);
		addControl(b =new gin::Select(envparams.duration), 3, 1);
		
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
		auto& globalParams = proc.globalParams;
		addControl(new APKnob(timbreparams.equant, true), 0, 0);
		addControl(new APKnob(timbreparams.pitch), 0, 1);
		addControl(new APKnob(timbreparams.blend), 1, 1);
        addControl(new APKnob(timbreparams.algo), 1, 0);
		addControl(new APKnob(timbreparams.demodmix), 2, 0);
		addControl(new APKnob(timbreparams.demodVol, true), 2, 1);
		addControl(new APKnob(globalParams.squash), 3, 0);
		addControl(new APKnob(proc.globalParams.velSens), 3, 1);
	}

};

//==============================================================================
class FilterBox : public gin::ParamBox
{
public:
    FilterBox(const juce::String& name, APAudioProcessor& proc_)
        : gin::ParamBox(name), proc(proc_)
    {
        setName("flt");
		setTitle("filter");

        auto& flt = proc.filterParams;

        addEnable(flt.enable);

        auto freq = new APKnob(flt.frequency);
        addControl(freq, 0, 0);
        addControl(new APKnob(flt.resonance), 1, 0);

        addControl(new APKnob(flt.keyTracking), 0, 1);
        addControl(new gin::Select(flt.type), 1, 1);

        freq->setLiveValuesCallback([this] ()
        {
            if (proc.filterParams.keyTracking->getUserValue() != 0.0f ||
                proc.modMatrix.isModulated(gin::ModDstId(proc.filterParams.frequency->getModIndex())))
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
    LFOBox(const juce::String& num, APAudioProcessor& proc_, APAudioProcessor::LFOParams& lfoparams_, gin::ModSrcId& modsrcID, gin::ModSrcId& monoID)
        : gin::ParamBox(num), proc(proc_), lfoparams(lfoparams_)
    {
        setName(num);

		addEnable(lfoparams.enable);

        addModSource(new gin::ModulationSourceButton(proc.modMatrix, monoID, false));
        addModSource(new gin::ModulationSourceButton(proc.modMatrix, modsrcID, true));

        addControl(r = new APKnob(lfoparams.rate), 0, 0);
        addControl(b = new gin::Select(lfoparams.beat), 0, 0);
        addControl(sync = new gin::Select(lfoparams.sync));
        addControl(wave = new gin::Select(lfoparams.wave));
        addControl(new APKnob(lfoparams.depth, true), 1, 1);

        addControl(offset = new APKnob(lfoparams.offset, true));
        addControl(phase = new APKnob(lfoparams.phase, true));
        addControl(delay = new APKnob(lfoparams.delay));
        addControl(fade = new APKnob(lfoparams.fade, true));
        
        auto l = new gin::LFOComponent();
        l->phaseCallback = [this, num]   
        {
            std::vector<float> res;
			auto unit = num.getTrailingIntValue() - 1;
            res.push_back(proc.monoLFOs[unit]->getCurrentPhase());
            return res;
        };
        l->setParams(lfoparams.wave, lfoparams.sync, lfoparams.rate, lfoparams.beat, lfoparams.depth, 
            lfoparams.offset, lfoparams.phase, lfoparams.enable);
        addControl(l, 1, 0, 4, 1);

        watchParam(lfoparams.sync);

        setSize(112, 163);
    }

    void paramChanged() override
    {
        gin::ParamBox::paramChanged();

        if (r && b)
        {
            r->setVisible(!lfoparams.sync->isOn());
            b->setVisible(lfoparams.sync->isOn());
        }
    }
    
    void resized() override {
        gin::ParamBox::resized();
        sync->setBounds(0, 93, 56, 35);
        wave->setBounds(0, 128, 56, 35);
        offset->setBounds(112, 108, 42, 57);
        phase->setBounds(158,  108, 42, 57);
        delay->setBounds(200,  108, 42, 57);
        fade->setBounds(242,   108, 42, 57);
    }

    APAudioProcessor& proc;
    gin::ParamComponent::Ptr r = nullptr, b = nullptr, offset = nullptr, phase = nullptr,
        delay = nullptr, fade = nullptr, sync = nullptr, wave = nullptr;
    APAudioProcessor::LFOParams& lfoparams;
};

//==============================================================================
class ModBox : public gin::ParamBox
{
public:
    ModBox(const juce::String& name, APAudioProcessor& proc_)
        : gin::ParamBox(name), proc(proc_)
    {
        setName("mod");
        setTitle("mod sources");
        addControl(modlist = new gin::ModSrcListBox(proc.modMatrix), 0, 0, 3, 2);
        modlist->setRowHeight(20);
    }

    gin::ModSrcListBox* modlist;
    APAudioProcessor& proc;
};

//==============================================================================
class GlobalBox : public gin::ParamBox
{
public:
    GlobalBox(const juce::String& name, APAudioProcessor& proc_)
        : gin::ParamBox(name), proc(proc_)
    {
        setName("global");

        addControl(new APKnob(proc.globalParams.level), 2, 1);
        addControl(new APKnob(proc.globalParams.glideRate), 2, 0);
		addControl(new gin::Switch(proc.globalParams.mpe), 0, 1);
		addControl(new APKnob(proc.globalParams.pitchbendRange), 1, 1);
        //  velsens
        addControl(legato = new gin::Select(proc.globalParams.legato));
        addControl(mono = new gin::Select(proc.globalParams.mono));
        addControl(glideMode = new gin::Select(proc.globalParams.glideMode));
        addControl(sidechain = new gin::Select(proc.globalParams.sidechainEnable));
    }

    void resized() override {
        gin::ParamBox::resized();
        legato->setBounds(0, 23, 56, 35);
        mono->setBounds(0, 58, 56, 35);
        glideMode->setBounds(56, 23, 56, 35);
        sidechain->setBounds(56, 58, 56, 35);
    }
    
    gin::ParamComponent::Ptr legato = nullptr, mono = nullptr, glideMode = nullptr, sidechain = nullptr;
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

		addControl(new APKnob(proc.orbitParams.speed), 0, 0);
		addControl(new APKnob(proc.orbitParams.scale), 1, 0);
		addControl(algo = new gin::Select(proc.timbreParams.algo));
	}

    void resized() override {
        gin::ParamBox::resized();
        algo->setBounds(0, 93, 112, 70);
    }
    
    gin::ParamComponent::Ptr algo = nullptr;
	APAudioProcessor& proc;
};


