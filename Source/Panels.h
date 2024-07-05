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
#include "EnvelopeComponent.h"

//==============================================================================
class OscillatorBox : public gin::ParamBox
{
public:
    OscillatorBox(APAudioProcessor& proc_)
        : gin::ParamBox("OSC"), proc(proc_)
    {
        addControl(c1 = new APKnob(proc.osc1Params.coarse), 0, 0); // coarse
		addControl(c2 = new APKnob(proc.osc2Params.coarse), 0, 0);
		addControl(c3 = new APKnob(proc.osc3Params.coarse), 0, 0);
		addControl(c4 = new APKnob(proc.osc4Params.coarse), 0, 0);
		addControl(f1 = new APKnob(proc.osc1Params.fine), 1, 0); // fine
		addControl(f2 = new APKnob(proc.osc2Params.fine, true), 1, 0);
		addControl(f3 = new APKnob(proc.osc3Params.fine, true), 1, 0);
		addControl(f4 = new APKnob(proc.osc4Params.fine, true), 1, 0);
        addControl(v1 = new APKnob(proc.osc1Params.volume), 2, 0); // volume
		addControl(v2 = new APKnob(proc.osc2Params.volume), 2, 0);
		addControl(v3 = new APKnob(proc.osc3Params.volume), 2, 0);
		addControl(v4 = new APKnob(proc.osc4Params.volume), 2, 0);
        addControl(t1 = new APKnob(proc.osc1Params.tones), 3, 0); // tones
		addControl(t2 = new APKnob(proc.osc2Params.tones), 3, 0);
		addControl(t3 = new APKnob(proc.osc3Params.tones), 3, 0);
		addControl(t4 = new APKnob(proc.osc4Params.tones), 3, 0);

		addControl(p1 = new MoonKnob(proc.osc1Params.phase), 4, 0); // phase
		addControl(p2 = new MoonKnob(proc.osc2Params.phase), 4, 0);
		addControl(p3 = new MoonKnob(proc.osc3Params.phase), 4, 0);
		addControl(p4 = new MoonKnob(proc.osc4Params.phase), 4, 0);

		c1->setLookAndFeel(&lnf1);
		f1->setLookAndFeel(&lnf1);
		v1->setLookAndFeel(&lnf1);
		c2->setLookAndFeel(&lnf2);
		f2->setLookAndFeel(&lnf2);
		v2->setLookAndFeel(&lnf2);
		c3->setLookAndFeel(&lnf3);
		f3->setLookAndFeel(&lnf3);
		v3->setLookAndFeel(&lnf3);
		c4->setLookAndFeel(&lnf4);
		f4->setLookAndFeel(&lnf4);
		v4->setLookAndFeel(&lnf4);
	
        addControl(d1 = new APKnob(proc.osc1Params.detune), 2, 1); // detune
		addControl(d2 = new APKnob(proc.osc2Params.detune), 2, 1);
		addControl(d3 = new APKnob(proc.osc3Params.detune), 2, 1);
		addControl(d4 = new APKnob(proc.osc4Params.detune), 2, 1);

        addControl(s1 = new APKnob(proc.osc1Params.spread), 3, 1); // spread
		addControl(s2 = new APKnob(proc.osc2Params.spread), 3, 1);
		addControl(s3 = new APKnob(proc.osc3Params.spread), 3, 1);
		addControl(s4 = new APKnob(proc.osc4Params.spread), 3, 1);

        addControl(n1 = new APKnob(proc.osc1Params.pan, true), 4, 1); // pan
		addControl(n2 = new APKnob(proc.osc2Params.pan, true), 4, 1);
		addControl(n3 = new APKnob(proc.osc3Params.pan, true), 4, 1);
		addControl(n4 = new APKnob(proc.osc4Params.pan, true), 4, 1);

        addControl(saw1 = new gin::Select(proc.osc1Params.saw)); // saw
		addControl(saw2 = new gin::Select(proc.osc2Params.saw));
		addControl(saw3 = new gin::Select(proc.osc3Params.saw));
		addControl(saw4 = new gin::Select(proc.osc4Params.saw));

        addControl(env1 = new gin::Select(proc.osc1Params.env)); // env select
		addControl(env2 = new gin::Select(proc.osc2Params.env));
		addControl(env3 = new gin::Select(proc.osc3Params.env));
		addControl(env4 = new gin::Select(proc.osc4Params.env));
		
		addControl(fixed1 = new gin::Select(proc.osc1Params.fixed)); // fixed
		addControl(fixed2 = new gin::Select(proc.osc2Params.fixed));
		addControl(fixed3 = new gin::Select(proc.osc3Params.fixed));
		addControl(fixed4 = new gin::Select(proc.osc4Params.fixed));

		watchParam(proc.osc1Params.fixed);
		watchParam(proc.osc2Params.fixed);
		watchParam(proc.osc3Params.fixed);
		watchParam(proc.osc4Params.fixed);
		watchParam(proc.osc1Params.saw);
		watchParam(proc.osc2Params.saw);
		watchParam(proc.osc3Params.saw);
		watchParam(proc.osc4Params.saw);
		watchParam(proc.osc1Params.coarse);
		watchParam(proc.osc2Params.coarse);
		watchParam(proc.osc3Params.coarse);
		watchParam(proc.osc4Params.coarse);
		watchParam(proc.osc1Params.fine);
		watchParam(proc.osc2Params.fine);
		watchParam(proc.osc3Params.fine);
		watchParam(proc.osc4Params.fine);

		addAndMakeVisible(fixedHz1);
		addAndMakeVisible(fixedHz2);
		addAndMakeVisible(fixedHz3);
		addAndMakeVisible(fixedHz4);
		addAndMakeVisible(select1);
		addAndMakeVisible(select2);
		addAndMakeVisible(select3);
		addAndMakeVisible(select4);

		fixedHz1.setJustificationType(Justification::centred);
		fixedHz2.setJustificationType(Justification::centred);
		fixedHz3.setJustificationType(Justification::centred);
		fixedHz4.setJustificationType(Justification::centred);

		select1.onClick = [this]() {show(1);};
		select2.onClick = [this]() {show(2);};
		select3.onClick = [this]() {show(3);};
		select4.onClick = [this]() {show(4);};

		show(1);
    }

	void show(int osc) {
		for (gin::ParamComponent::Ptr c : {c1, f1, v1, t1, d1, s1, p1, n1, saw1, env1, fixed1,
			c2, f2, v2, t2, d2, s2, p2, n2, saw2, env2, fixed2,	
			c3, f3, v3, t3, d3, s3, p3, n3, saw3, env3, fixed3,
			c4, f4, v4, t4, d4, s4, p4, n4, saw4, env4, fixed4
		})
		{
			c->setVisible(false);
		}
		switch(osc) {
		case 1:
			currentOsc = 1;
			c1->setVisible(true);
			f1->setVisible(true);
			v1->setVisible(true);
			t1->setVisible(true);
			d1->setVisible(true);
			s1->setVisible(true);
			p1->setVisible(true);
			n1->setVisible(true);
			saw1->setVisible(true);
			env1->setVisible(true);
			fixed1->setVisible(true);
			break;
		case 2:
			currentOsc = 2;
			c2->setVisible(true);
			f2->setVisible(true);
			v2->setVisible(true);
			t2->setVisible(true);
			d2->setVisible(true);
			s2->setVisible(true);
			p2->setVisible(true);
			n2->setVisible(true);
			saw2->setVisible(true);
			env2->setVisible(true);
			fixed2->setVisible(true);
			break;
		case 3:
			currentOsc = 3;
			c3->setVisible(true);
			f3->setVisible(true);
			v3->setVisible(true);
			t3->setVisible(true);
			d3->setVisible(true);
			s3->setVisible(true);
			p3->setVisible(true);
			n3->setVisible(true);
			saw3->setVisible(true);
			env3->setVisible(true);
			fixed3->setVisible(true);
			break;
		case 4:
			currentOsc = 4;
			c4->setVisible(true);
			f4->setVisible(true);
			v4->setVisible(true);
			t4->setVisible(true);
			d4->setVisible(true);
			s4->setVisible(true);
			p4->setVisible(true);
			n4->setVisible(true);
			saw4->setVisible(true);
			env4->setVisible(true);
			fixed4->setVisible(true);
			break;
		}
	}

    ~OscillatorBox() override
    {
		c1->setLookAndFeel(nullptr);
		f1->setLookAndFeel(nullptr);
		v1->setLookAndFeel(nullptr);
		c2->setLookAndFeel(nullptr);
		f2->setLookAndFeel(nullptr);
		v2->setLookAndFeel(nullptr);
		c3->setLookAndFeel(nullptr);
		f3->setLookAndFeel(nullptr);
		v3->setLookAndFeel(nullptr);
		c4->setLookAndFeel(nullptr);
		f4->setLookAndFeel(nullptr);
		v4->setLookAndFeel(nullptr);
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
		if (proc.osc1Params.fixed->isOn() && currentOsc == 1) {
			fixedHz1.setVisible(true);
			fixedHz1.setText(String((proc.osc1Params.coarse->getUserValue() + proc.osc1Params.fine->getUserValue()) * 100, 2) + String(" Hz"), juce::dontSendNotification);
		}
		else {
			fixedHz1.setVisible(false);
		}
		if (proc.osc2Params.fixed->isOn() && currentOsc == 2) {
			fixedHz2.setVisible(true);
			fixedHz2.setText(String((proc.osc2Params.coarse->getUserValue() 
			+ proc.osc2Params.fine->getUserValue()) * 100, 2) + String(" Hz"), juce::dontSendNotification);
		}
		else {
			fixedHz2.setVisible(false);
		}
		if (proc.osc3Params.fixed->isOn() && currentOsc == 3) {
			fixedHz3.setVisible(true);
			fixedHz3.setText(String((proc.osc3Params.coarse->getUserValue() 
			+ proc.osc3Params.fine->getUserValue()) * 100, 2) + String(" Hz"), juce::dontSendNotification);
		}
		else {
			fixedHz3.setVisible(false);
		}
		if (proc.osc4Params.fixed->isOn() && currentOsc == 4) {
			fixedHz4.setVisible(true);
			fixedHz4.setText(String((proc.osc4Params.coarse->getUserValue() 
			+ proc.osc4Params.fine->getUserValue()) * 100, 2) + String(" Hz"), juce::dontSendNotification);
		}
		else {
			fixedHz4.setVisible(false);
		}
	}

	void resized() override {
		gin::ParamBox::resized();
		fixedHz1.setBounds(56, 23+70+10, 56, 15); // 23/70/10 header, first row, padding
		fixedHz2.setBounds(56, 23+70+10, 56, 15);
		fixedHz3.setBounds(56, 23+70+10, 56, 15);
		fixedHz4.setBounds(56, 23+70+10, 56, 15);
		p1->setBounds(56*4, 23, 56, 70);
		p2->setBounds(56*4, 23, 56, 70);
		p3->setBounds(56*4, 23, 56, 70);
		p4->setBounds(56*4, 23, 56, 70);
        saw1->setBounds(0, 93, 56, 35);
		saw2->setBounds(0, 93, 56, 35);
		saw3->setBounds(0, 93, 56, 35);
		saw4->setBounds(0, 93, 56, 35);
        env1->setBounds(0, 128, 56, 35);
		env2->setBounds(0, 128, 56, 35);
		env3->setBounds(0, 128, 56, 35);
		env4->setBounds(0, 128, 56, 35);
        fixed1->setBounds(56, 128, 56, 35);
		fixed2->setBounds(56, 128, 56, 35);
		fixed3->setBounds(56, 128, 56, 35);
		fixed4->setBounds(56, 128, 56, 35);
		select1.setBounds(100, 0, 20, 23);
		select2.setBounds(120, 0, 20, 23);
		select3.setBounds(140, 0, 20, 23);
		select4.setBounds(160, 0, 20, 23);
	}
    
	APLookAndFeel1 lnf1;
	APLookAndFeel2 lnf2;
	APLookAndFeel3 lnf3;
	APLookAndFeel4 lnf4;
    APAudioProcessor& proc;
	gin::ParamComponent::Ptr c1, f1, v1, t1, p1, d1, s1, n1, saw1, env1, fixed1;
	gin::ParamComponent::Ptr c2, f2, v2, t2, p2, d2, s2, n2, saw2, env2, fixed2;
	gin::ParamComponent::Ptr c3, f3, v3, t3, p3, d3, s3, n3, saw3, env3, fixed3;
	gin::ParamComponent::Ptr c4, f4, v4, t4, p4, d4, s4, n4, saw4, env4, fixed4;

	Label fixedHz1, fixedHz2, fixedHz3, fixedHz4;
	TextButton select1{"1"}, select2{"2"}, select3{"3"}, select4{"4"};

	int currentOsc{1};
};

//==============================================================================
class ENVBox : public gin::ParamBox
{
public:
    ENVBox(APAudioProcessor& proc_)
        : gin::ParamBox("ENV"), proc(proc_)
    {
		// in reverse order
		addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcEnv4, true));
		addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcEnv3, true));
		addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcEnv2, true));
		addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcEnv1, true));


        addControl(a1 = new APKnob(proc.env1Params.attack), 0, 0);
		addControl(a2 = new APKnob(proc.env2Params.attack), 0, 0);
		addControl(a3 = new APKnob(proc.env3Params.attack), 0, 0);
		addControl(a4 = new APKnob(proc.env4Params.attack), 0, 0);
		addControl(d1 = new APKnob(proc.env1Params.decay), 1, 0);
		addControl(d2 = new APKnob(proc.env2Params.decay), 1, 0);
		addControl(d3 = new APKnob(proc.env3Params.decay), 1, 0);
		addControl(d4 = new APKnob(proc.env4Params.decay), 1, 0);
		addControl(s1 = new APKnob(proc.env1Params.sustain), 2, 0);
		addControl(s2 = new APKnob(proc.env2Params.sustain), 2, 0);
		addControl(s3 = new APKnob(proc.env3Params.sustain), 2, 0);
		addControl(s4 = new APKnob(proc.env4Params.sustain), 2, 0);
		addControl(r1 = new APKnob(proc.env1Params.release), 3, 0);
		addControl(r2 = new APKnob(proc.env2Params.release), 3, 0);
		addControl(r3 = new APKnob(proc.env3Params.release), 3, 0);
		addControl(r4 = new APKnob(proc.env4Params.release), 3, 0);
		addControl(ac1 = new APKnob(proc.env1Params.acurve, true), 4, 0);
		addControl(ac2 = new APKnob(proc.env2Params.acurve, true), 4, 0);
		addControl(ac3 = new APKnob(proc.env3Params.acurve, true), 4, 0);
		addControl(ac4 = new APKnob(proc.env4Params.acurve, true), 4, 0);
		addControl(dc1 = new APKnob(proc.env1Params.drcurve, true), 5, 0);
		addControl(dc2 = new APKnob(proc.env2Params.drcurve, true), 5, 0);
		addControl(dc3 = new APKnob(proc.env3Params.drcurve, true), 5, 0);
		addControl(dc4 = new APKnob(proc.env4Params.drcurve, true), 5, 0);
		addControl(rpt1 = new gin::Select(proc.env1Params.syncrepeat), 4, 1);
		addControl(rpt2 = new gin::Select(proc.env2Params.syncrepeat), 4, 1);
		addControl(rpt3 = new gin::Select(proc.env3Params.syncrepeat), 4, 1);
		addControl(rpt4 = new gin::Select(proc.env4Params.syncrepeat), 4, 1);
		addControl(beats1 = new gin::Select(proc.env1Params.duration), 5, 1);
		addControl(beats2 = new gin::Select(proc.env2Params.duration), 5, 1);
		addControl(beats3 = new gin::Select(proc.env3Params.duration), 5, 1);
		addControl(beats4 = new gin::Select(proc.env4Params.duration), 5, 1);
		addControl(rate1 = new APKnob(proc.env1Params.time), 5, 1);
		addControl(rate2 = new APKnob(proc.env2Params.time), 5, 1);
		addControl(rate3 = new APKnob(proc.env3Params.time), 5, 1);
		addControl(rate4 = new APKnob(proc.env4Params.time), 5, 1);
		
		watchParam(proc.env1Params.syncrepeat);
		watchParam(proc.env2Params.syncrepeat);
		watchParam(proc.env3Params.syncrepeat);
		watchParam(proc.env4Params.syncrepeat);

		// TODO: add and implement selectors
		addAndMakeVisible(select1);
		addAndMakeVisible(select2);
		addAndMakeVisible(select3);
		addAndMakeVisible(select4);

		addAndMakeVisible(env1Viz);
		addAndMakeVisible(env2Viz);
		addAndMakeVisible(env3Viz);
		addAndMakeVisible(env4Viz);

		select1.onClick = [this]() {show(1);};
		select2.onClick = [this]() {show(2);};
		select3.onClick = [this]() {show(3);};
		select4.onClick = [this]() {show(4);};

		show(1);
	}

	void show(int osc) {
		for (gin::ParamComponent::Ptr c : {a1, d1, s1, r1, ac1, dc1, rpt1, beats1, rate1,
			a2, d2, s2, r2, ac2, dc2, rpt2, beats2, rate2,
			a3, d3, s3, r3, ac3, dc3, rpt3, beats3, rate3,
			a4, d4, s4, r4, ac4, dc4, rpt4, beats4, rate4
		})
		{
			c->setVisible(false);
		}
		for (auto& viz : {&env1Viz, &env2Viz, &env3Viz, &env4Viz})
		{
			viz->setVisible(false);
		}
		int choice;
		switch (osc) {
		case 1:
			currentEnv = 1;
			a1->setVisible(true);
			d1->setVisible(true);
			s1->setVisible(true);
			r1->setVisible(true);
			ac1->setVisible(true);
			dc1->setVisible(true);
			rpt1->setVisible(true);
			env1Viz.setVisible(true);
			choice = proc.env1Params.syncrepeat->getUserValueInt();
			beats1->setVisible(choice == 2);
			rate1->setVisible(choice == 1);
			break;
		case 2:
			currentEnv = 2;
			a2->setVisible(true);
			d2->setVisible(true);
			s2->setVisible(true);
			r2->setVisible(true);
			ac2->setVisible(true);
			dc2->setVisible(true);
			rpt2->setVisible(true);
			env2Viz.setVisible(true);
			choice = proc.env2Params.syncrepeat->getUserValueInt();
			beats2->setVisible(choice == 2);
			rate2->setVisible(choice == 1);
			break;
		case 3:
			currentEnv = 3;
			a3->setVisible(true);
			d3->setVisible(true);
			s3->setVisible(true);
			r3->setVisible(true);
			ac3->setVisible(true);
			dc3->setVisible(true);
			rpt3->setVisible(true);
			env3Viz.setVisible(true);
			choice = proc.env3Params.syncrepeat->getUserValueInt();
			beats3->setVisible(choice == 2);
			rate3->setVisible(choice == 1);
			break;
		case 4:
			currentEnv = 4;
			a4->setVisible(true);
			d4->setVisible(true);
			s4->setVisible(true);
			r4->setVisible(true);
			ac4->setVisible(true);
			dc4->setVisible(true);
			rpt4->setVisible(true);
			env4Viz.setVisible(true);
			choice = proc.env4Params.syncrepeat->getUserValueInt();
			beats4->setVisible(choice == 2);
			rate4->setVisible(choice == 1);
			break;
		}
	}

	void paramChanged() override
	{
		gin::ParamBox::paramChanged();

		show(currentEnv);
	}

	void resized() override {
		gin::ParamBox::resized();
		select1.setBounds(100, 0, 20, 23);
		select2.setBounds(120, 0, 20, 23);
		select3.setBounds(140, 0, 20, 23);
		select4.setBounds(160, 0, 20, 23);
		env1Viz.setBounds(0, 23+70, 56*4, 70);
		env2Viz.setBounds(0, 23+70, 56*4, 70);
		env3Viz.setBounds(0, 23+70, 56*4, 70);
		env4Viz.setBounds(0, 23+70, 56*4, 70);
	}

	APAudioProcessor& proc;
	gin::ParamComponent::Ptr a1, d1, s1, r1, ac1, dc1, rpt1, beats1, rate1;
	gin::ParamComponent::Ptr a2, d2, s2, r2, ac2, dc2, rpt2, beats2, rate2;
	gin::ParamComponent::Ptr a3, d3, s3, r3, ac3, dc3, rpt3, beats3, rate3;
	gin::ParamComponent::Ptr a4, d4, s4, r4, ac4, dc4, rpt4, beats4, rate4;


	EnvelopeComponent env1Viz{ proc, 1 }, env2Viz{ proc, 2 }, env3Viz{ proc, 3 }, env4Viz{ proc, 4 };
	int currentEnv{1};
	TextButton select1{ "1" }, select2{ "2" }, select3{ "3" }, select4{ "4" };
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


