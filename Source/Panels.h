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

#include <gin_plugin/gin_plugin.h>
#include "PluginProcessor.h"
#include "APColors.h"
#include "MoonKnob.h"
#include "APModAdditions.h"
#include "EnvelopeComponent.h"

//==============================================================================
class OscillatorBox : public gin::ParamBox
{
public:
	OscillatorBox(APAudioProcessor &proc_)
		: gin::ParamBox("  OSC"), proc(proc_)
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
		lnf1.setColour(juce::Slider::trackColourId, juce::Colours::black);
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

		setColour(juce::TextButton::buttonOnColourId, juce::Colours::beige);

		fixedHz1.setJustificationType(juce::Justification::centred);
		fixedHz2.setJustificationType(juce::Justification::centred);
		fixedHz3.setJustificationType(juce::Justification::centred);
		fixedHz4.setJustificationType(juce::Justification::centred);

		select1.onClick = [this]()
		{ show(1); };
		select2.onClick = [this]()
		{ show(2); };
		select3.onClick = [this]()
		{ show(3); };
		select4.onClick = [this]()
		{ show(4); };

		show(1);
	}

	void show(int osc)
	{
		for (gin::ParamComponent::Ptr c : {c1, f1, v1, t1, d1, s1, p1, n1, saw1, env1, fixed1,
										   c2, f2, v2, t2, d2, s2, p2, n2, saw2, env2, fixed2,
										   c3, f3, v3, t3, d3, s3, p3, n3, saw3, env3, fixed3,
										   c4, f4, v4, t4, d4, s4, p4, n4, saw4, env4, fixed4})
		{
			c->setVisible(false);
		}
		for (auto &viz : {&fixedHz1, &fixedHz2, &fixedHz3, &fixedHz4})
		{
			viz->setVisible(false);
		}
		select1.setToggleState(false, juce::dontSendNotification);
		select2.setToggleState(false, juce::dontSendNotification);
		select3.setToggleState(false, juce::dontSendNotification);
		select4.setToggleState(false, juce::dontSendNotification);
		switch (osc)
		{
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
			select1.setToggleState(true, juce::dontSendNotification);
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
			select2.setToggleState(true, juce::dontSendNotification);
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
			select3.setToggleState(true, juce::dontSendNotification);
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
			select4.setToggleState(true, juce::dontSendNotification);
			break;
		}
		paramChanged();
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
		APLookAndFeel1()
		{
			setColour(juce::Slider::rotarySliderFillColourId, APColors::red);
			setColour(juce::Slider::trackColourId, APColors::redMuted);
		}
	};
	class APLookAndFeel2 : public APLNF
	{
	public:
		APLookAndFeel2()
		{
			setColour(juce::Slider::rotarySliderFillColourId, APColors::yellow);
			setColour(juce::Slider::trackColourId, APColors::yellowMuted);
		}
	};
	class APLookAndFeel3 : public APLNF
	{
	public:
		APLookAndFeel3()
		{
			setColour(juce::Slider::rotarySliderFillColourId, APColors::green);
			setColour(juce::Slider::trackColourId, APColors::greenMuted);
		}
	};
	class APLookAndFeel4 : public APLNF
	{
	public:
		APLookAndFeel4()
		{
			setColour(juce::Slider::rotarySliderFillColourId, APColors::blue);
			setColour(juce::Slider::trackColourId, APColors::blueMuted);
		}
	};

	void paramChanged() override
	{
		gin::ParamBox::paramChanged();
		if (proc.osc1Params.fixed->isOn() && currentOsc == 1)
		{
			fixedHz1.setVisible(true);
			fixedHz1.setText(juce::String((proc.osc1Params.coarse->getUserValue() + proc.osc1Params.fine->getUserValue()) * 100, 2) + juce::String(" Hz"), juce::dontSendNotification);
		}
		else
		{
			fixedHz1.setVisible(false);
		}
		if (proc.osc2Params.fixed->isOn() && currentOsc == 2)
		{
			fixedHz2.setVisible(true);
			fixedHz2.setText(juce::String((proc.osc2Params.coarse->getUserValue() + proc.osc2Params.fine->getUserValue()) * 100, 2) + juce::String(" Hz"), juce::dontSendNotification);
		}
		else
		{
			fixedHz2.setVisible(false);
		}
		if (proc.osc3Params.fixed->isOn() && currentOsc == 3)
		{
			fixedHz3.setVisible(true);
			fixedHz3.setText(juce::String((proc.osc3Params.coarse->getUserValue() + proc.osc3Params.fine->getUserValue()) * 100, 2) + juce::String(" Hz"), juce::dontSendNotification);
		}
		else
		{
			fixedHz3.setVisible(false);
		}
		if (proc.osc4Params.fixed->isOn() && currentOsc == 4)
		{
			fixedHz4.setVisible(true);
			fixedHz4.setText(juce::String((proc.osc4Params.coarse->getUserValue() + proc.osc4Params.fine->getUserValue()) * 100, 2) + juce::String(" Hz"), juce::dontSendNotification);
		}
		else
		{
			fixedHz4.setVisible(false);
		}
	}

	void resized() override
	{
		gin::ParamBox::resized();
		fixedHz1.setBounds(56, 23 + 70 + 10, 56, 15); // 23/70/10 header, first row, padding
		fixedHz2.setBounds(56, 23 + 70 + 10, 56, 15);
		fixedHz3.setBounds(56, 23 + 70 + 10, 56, 15);
		fixedHz4.setBounds(56, 23 + 70 + 10, 56, 15);
		p1->setBounds(56 * 4, 23, 56, 70);
		p2->setBounds(56 * 4, 23, 56, 70);
		p3->setBounds(56 * 4, 23, 56, 70);
		p4->setBounds(56 * 4, 23, 56, 70);
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
	APAudioProcessor &proc;
	gin::ParamComponent::Ptr c1, f1, v1, t1, p1, d1, s1, n1, saw1, env1, fixed1;
	gin::ParamComponent::Ptr c2, f2, v2, t2, p2, d2, s2, n2, saw2, env2, fixed2;
	gin::ParamComponent::Ptr c3, f3, v3, t3, p3, d3, s3, n3, saw3, env3, fixed3;
	gin::ParamComponent::Ptr c4, f4, v4, t4, p4, d4, s4, n4, saw4, env4, fixed4;

	juce::Label fixedHz1, fixedHz2, fixedHz3, fixedHz4;
	juce::TextButton select1{"1"}, select2{"2"}, select3{"3"}, select4{"4"};

	int currentOsc{1};
};

//==============================================================================
class ENVBox : public gin::ParamBox
{
public:
	ENVBox(APAudioProcessor &proc_)
		: gin::ParamBox("  ENV"), proc(proc_)
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

		select1.onClick = [this]()
		{ show(1); };
		select2.onClick = [this]()
		{ show(2); };
		select3.onClick = [this]()
		{ show(3); };
		select4.onClick = [this]()
		{ show(4); };

		show(1);
	}

	void show(int env)
	{
		for (gin::ParamComponent::Ptr c : {a1, d1, s1, r1, ac1, dc1, rpt1, beats1, rate1,
										   a2, d2, s2, r2, ac2, dc2, rpt2, beats2, rate2,
										   a3, d3, s3, r3, ac3, dc3, rpt3, beats3, rate3,
										   a4, d4, s4, r4, ac4, dc4, rpt4, beats4, rate4})
		{
			c->setVisible(false);
		}
		for (auto &viz : {&env1Viz, &env2Viz, &env3Viz, &env4Viz})
		{
			viz->setVisible(false);
		}
		select1.setToggleState(false, juce::dontSendNotification);
		select2.setToggleState(false, juce::dontSendNotification);
		select3.setToggleState(false, juce::dontSendNotification);
		select4.setToggleState(false, juce::dontSendNotification);
		int choice;
		switch (env)
		{
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
			beats1->setVisible(choice == 1);
			rate1->setVisible(choice == 2);
			select1.setToggleState(true, juce::dontSendNotification);
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
			beats2->setVisible(choice == 1);
			rate2->setVisible(choice == 2);
			select2.setToggleState(true, juce::dontSendNotification);
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
			beats3->setVisible(choice == 1);
			rate3->setVisible(choice == 2);
			select3.setToggleState(true, juce::dontSendNotification);
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
			beats4->setVisible(choice == 1);
			rate4->setVisible(choice == 2);
			select4.setToggleState(true, juce::dontSendNotification);
			break;
		}
	}

	void paramChanged() override
	{
		gin::ParamBox::paramChanged();

		show(currentEnv);
	}

	void resized() override
	{
		gin::ParamBox::resized();
		select1.setBounds(100, 0, 20, 23);
		select2.setBounds(120, 0, 20, 23);
		select3.setBounds(140, 0, 20, 23);
		select4.setBounds(160, 0, 20, 23);
		env1Viz.setBounds(0, 23 + 70, 56 * 4, 70);
		env2Viz.setBounds(0, 23 + 70, 56 * 4, 70);
		env3Viz.setBounds(0, 23 + 70, 56 * 4, 70);
		env4Viz.setBounds(0, 23 + 70, 56 * 4, 70);
	}

	APAudioProcessor &proc;
	gin::ParamComponent::Ptr a1, d1, s1, r1, ac1, dc1, rpt1, beats1, rate1;
	gin::ParamComponent::Ptr a2, d2, s2, r2, ac2, dc2, rpt2, beats2, rate2;
	gin::ParamComponent::Ptr a3, d3, s3, r3, ac3, dc3, rpt3, beats3, rate3;
	gin::ParamComponent::Ptr a4, d4, s4, r4, ac4, dc4, rpt4, beats4, rate4;

	EnvelopeComponent env1Viz{proc, 1}, env2Viz{proc, 2}, env3Viz{proc, 3}, env4Viz{proc, 4};
	int currentEnv{1};
	juce::TextButton select1{"1"}, select2{"2"}, select3{"3"}, select4{"4"};
};

//==============================================================================
class TimbreBox : public gin::ParamBox
{
public:
	TimbreBox(const juce::String &name, APAudioProcessor &proc)
		: gin::ParamBox(name)
	{
		setName(name);

		auto &timbreparams = proc.timbreParams;
		auto &globalParams = proc.globalParams;
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
	FilterBox(const juce::String &name, APAudioProcessor &proc_)
		: gin::ParamBox(name), proc(proc_)
	{
		setName("flt");
		setTitle("  filter");

		auto &flt = proc.filterParams;

		auto freq = new APKnob(flt.frequency);
		addControl(freq, 0, 0);
		addControl(new APKnob(flt.resonance), 1, 0);

		addControl(new APKnob(flt.keyTracking), 0, 1);
		addControl(new gin::Select(flt.type), 1, 1);

		freq->setLiveValuesCallback([this]()
									{
            if (proc.filterParams.keyTracking->getUserValue() != 0.0f ||
                proc.modMatrix.isModulated(gin::ModDstId(proc.filterParams.frequency->getModIndex())))
                return proc.getLiveFilterCutoff();
            return juce::Array<float>(); });
	}

	APAudioProcessor &proc;
};

//==============================================================================
class LFOBox : public gin::ParamBox
{
public:
	LFOBox(APAudioProcessor &proc_)
		: ParamBox("  LFO"), proc(proc_)
	{
		setName("lfo");

		addModSource(poly1 = new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcLFO1, true));
		addModSource(poly2 = new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcLFO2, true));
		addModSource(poly3 = new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcLFO3, true));
		addModSource(poly4 = new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcLFO4, true));
		addModSource(mono1 = new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcMonoLFO1, false));
		addModSource(mono2 = new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcMonoLFO2, false));
		addModSource(mono3 = new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcMonoLFO3, false));
		addModSource(mono4 = new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcMonoLFO4, false));
        poly1->getProperties().set("polysrc", true);
        poly2->getProperties().set("polysrc", true);
        poly3->getProperties().set("polysrc", true);
        poly4->getProperties().set("polysrc", true);

		addControl(r1 = new APKnob(proc.lfo1Params.rate), 0, 0);
		addControl(r2 = new APKnob(proc.lfo2Params.rate), 0, 0);
		addControl(r3 = new APKnob(proc.lfo3Params.rate), 0, 0);
		addControl(r4 = new APKnob(proc.lfo4Params.rate), 0, 0);
		addControl(b1 = new gin::Select(proc.lfo1Params.beat), 0, 0);
		addControl(b2 = new gin::Select(proc.lfo2Params.beat), 0, 0);
		addControl(b3 = new gin::Select(proc.lfo3Params.beat), 0, 0);
		addControl(b4 = new gin::Select(proc.lfo4Params.beat), 0, 0);

		addControl(s1 = new gin::Select(proc.lfo1Params.sync));
		addControl(s2 = new gin::Select(proc.lfo2Params.sync));
		addControl(s3 = new gin::Select(proc.lfo3Params.sync));
		addControl(s4 = new gin::Select(proc.lfo4Params.sync));

		addControl(w1 = new gin::Select(proc.lfo1Params.wave));
		addControl(w2 = new gin::Select(proc.lfo2Params.wave));
		addControl(w3 = new gin::Select(proc.lfo3Params.wave));
		addControl(w4 = new gin::Select(proc.lfo4Params.wave));

		addControl(dp1 = new APKnob(proc.lfo1Params.depth, true), 1, 1);
		addControl(dp2 = new APKnob(proc.lfo2Params.depth, true), 1, 1);
		addControl(dp3 = new APKnob(proc.lfo3Params.depth, true), 1, 1);
		addControl(dp4 = new APKnob(proc.lfo4Params.depth, true), 1, 1);

		addControl(o1 = new APKnob(proc.lfo1Params.offset, true), 1, 2);
		addControl(o2 = new APKnob(proc.lfo2Params.offset, true), 1, 2);
		addControl(o3 = new APKnob(proc.lfo3Params.offset, true), 1, 2);
		addControl(o4 = new APKnob(proc.lfo4Params.offset, true), 1, 2);

		addControl(p1 = new APKnob(proc.lfo1Params.phase, true), 1, 3);
		addControl(p2 = new APKnob(proc.lfo2Params.phase, true), 1, 3);
		addControl(p3 = new APKnob(proc.lfo3Params.phase, true), 1, 3);
		addControl(p4 = new APKnob(proc.lfo4Params.phase, true), 1, 3);

		addControl(dl1 = new APKnob(proc.lfo1Params.delay), 1, 4);
		addControl(dl2 = new APKnob(proc.lfo2Params.delay), 1, 4);
		addControl(dl3 = new APKnob(proc.lfo3Params.delay), 1, 4);
		addControl(dl4 = new APKnob(proc.lfo4Params.delay), 1, 4);

		addControl(f1 = new APKnob(proc.lfo1Params.fade, true), 1, 5);
		addControl(f2 = new APKnob(proc.lfo2Params.fade, true), 1, 5);
		addControl(f3 = new APKnob(proc.lfo3Params.fade, true), 1, 5);
		addControl(f4 = new APKnob(proc.lfo4Params.fade, true), 1, 5);

		l1 = new gin::LFOComponent();
		l1->phaseCallback = [this]
		{
			std::vector<float> res;
			res.push_back(proc.monoLFOs[0]->getCurrentPhase());
			return res;
		};
		l1->setParams(proc.lfo1Params.wave, proc.lfo1Params.sync, proc.lfo1Params.rate, proc.lfo1Params.beat, proc.lfo1Params.depth,
					  proc.lfo1Params.offset, proc.lfo1Params.phase, proc.lfo1Params.enable);
		addControl(l1, 1, 0, 4, 1);

		l2 = new gin::LFOComponent();
		l2->phaseCallback = [this]
		{
			std::vector<float> res;
			res.push_back(proc.monoLFOs[1]->getCurrentPhase());
			return res;
		};
		l2->setParams(proc.lfo2Params.wave, proc.lfo2Params.sync, proc.lfo2Params.rate, proc.lfo2Params.beat, proc.lfo2Params.depth,
					  proc.lfo2Params.offset, proc.lfo2Params.phase, proc.lfo2Params.enable);
		addControl(l2, 1, 0, 4, 1);

		l3 = new gin::LFOComponent();
		l3->phaseCallback = [this]
		{
			std::vector<float> res;
			res.push_back(proc.monoLFOs[2]->getCurrentPhase());
			return res;
		};
		l3->setParams(proc.lfo3Params.wave, proc.lfo3Params.sync, proc.lfo3Params.rate, proc.lfo3Params.beat, proc.lfo3Params.depth,
					  proc.lfo3Params.offset, proc.lfo3Params.phase, proc.lfo3Params.enable);
		addControl(l3, 1, 0, 4, 1);

		l4 = new gin::LFOComponent();
		l4->phaseCallback = [this]
		{
			std::vector<float> res;
			res.push_back(proc.monoLFOs[3]->getCurrentPhase());
			return res;
		};
		l4->setParams(proc.lfo4Params.wave, proc.lfo4Params.sync, proc.lfo4Params.rate, proc.lfo4Params.beat, proc.lfo4Params.depth,
					  proc.lfo4Params.offset, proc.lfo4Params.phase, proc.lfo4Params.enable);
		addControl(l4, 1, 0, 4, 1);

		addAndMakeVisible(select1);
		addAndMakeVisible(select2);
		addAndMakeVisible(select3);
		addAndMakeVisible(select4);

		select1.onClick = [this]()
		{ show(1); };
		select2.onClick = [this]()
		{ show(2); };
		select3.onClick = [this]()
		{ show(3); };
		select4.onClick = [this]()
		{ show(4); };

		watchParam(proc.lfo1Params.sync);
		watchParam(proc.lfo2Params.sync);
		watchParam(proc.lfo3Params.sync);
		watchParam(proc.lfo4Params.sync);

		show(1);
	}

	void show(int lfo)
	{
		for (gin::ParamComponent::Ptr c : {
				 r1, b1, s1, w1, dp1, p1, o1, f1, dl1,
				 r2, b2, s2, w2, dp2, p2, o2, f2, dl2,
				 r3, b3, s3, w3, dp3, p3, o3, f3, dl3,
				 r4, b4, s4, w4, dp4, p4, o4, f4, dl4})
		{
			c->setVisible(false);
		}
		for (auto viz : {l1, l2, l3, l4})
		{
			viz->setVisible(false);
		}
		for (auto button : {poly1, poly2, poly3, poly4, mono1, mono2, mono3, mono4})
		{
			button->setVisible(false);
		}
		select1.setToggleState(false, juce::dontSendNotification);
		select2.setToggleState(false, juce::dontSendNotification);
		select3.setToggleState(false, juce::dontSendNotification);
		select4.setToggleState(false, juce::dontSendNotification);
		bool choice;
		switch (lfo)
		{
		case 1:
			currentLFO = 1;
			choice = proc.lfo1Params.sync->isOn();
			r1->setVisible(!choice);
			b1->setVisible(choice);
			s1->setVisible(true);
			w1->setVisible(true);
			dp1->setVisible(true);
			p1->setVisible(true);
			o1->setVisible(true);
			f1->setVisible(true);
			dl1->setVisible(true);
			l1->setVisible(true);
			select1.setToggleState(true, juce::dontSendNotification);
			poly1->setVisible(true);
			mono1->setVisible(true);
			break;
		case 2:
			currentLFO = 2;
			choice = proc.lfo2Params.sync->isOn();
			r2->setVisible(!choice);
			b2->setVisible(choice);
			s2->setVisible(true);
			w2->setVisible(true);
			dp2->setVisible(true);
			p2->setVisible(true);
			o2->setVisible(true);
			f2->setVisible(true);
			dl2->setVisible(true);
			l2->setVisible(true);
			select2.setToggleState(true, juce::dontSendNotification);
			poly2->setVisible(true);
			mono2->setVisible(true);
			break;
		case 3:
			currentLFO = 3;
			choice = proc.lfo3Params.sync->isOn();
			r3->setVisible(!choice);
			b3->setVisible(choice);
			s3->setVisible(true);
			w3->setVisible(true);
			dp3->setVisible(true);
			p3->setVisible(true);
			o3->setVisible(true);
			f3->setVisible(true);
			dl3->setVisible(true);
			l3->setVisible(true);
			select3.setToggleState(true, juce::dontSendNotification);
			poly3->setVisible(true);
			mono3->setVisible(true);
			break;
		case 4:
			currentLFO = 4;
			choice = proc.lfo4Params.sync->isOn();
			r4->setVisible(!choice);
			b4->setVisible(choice);
			s4->setVisible(true);
			w4->setVisible(true);
			dp4->setVisible(true);
			p4->setVisible(true);
			o4->setVisible(true);
			f4->setVisible(true);
			dl4->setVisible(true);
			l4->setVisible(true);
			select4.setToggleState(true, juce::dontSendNotification);
			poly4->setVisible(true);
			mono4->setVisible(true);
			break;
		}
		paramChanged();
	}

	void paramChanged() override
	{
		gin::ParamBox::paramChanged();
		switch (currentLFO)
		{
		case 1:
			r1->setVisible(!proc.lfo1Params.sync->isOn());
			b1->setVisible(proc.lfo1Params.sync->isOn());
			break;
		case 2:
			r2->setVisible(!proc.lfo2Params.sync->isOn());
			b2->setVisible(proc.lfo2Params.sync->isOn());
			break;
		case 3:
			r3->setVisible(!proc.lfo3Params.sync->isOn());
			b3->setVisible(proc.lfo3Params.sync->isOn());
			break;
		case 4:
			r4->setVisible(!proc.lfo4Params.sync->isOn());
			b4->setVisible(proc.lfo4Params.sync->isOn());
			break;
		}
	}

	void resized() override
	{
		gin::ParamBox::resized();
		s1->setBounds(0, 93, 56, 35);
		w1->setBounds(0, 128, 56, 35);
		s2->setBounds(0, 93, 56, 35);
		w2->setBounds(0, 128, 56, 35);
		s3->setBounds(0, 93, 56, 35);
		w3->setBounds(0, 128, 56, 35);
		s4->setBounds(0, 93, 56, 35);
		w4->setBounds(0, 128, 56, 35);
		select1.setBounds(100, 0, 20, 23);
		select2.setBounds(120, 0, 20, 23);
		select3.setBounds(140, 0, 20, 23);
		select4.setBounds(160, 0, 20, 23);
		o1->setBounds(112, 108, 42, 57);
		o2->setBounds(112, 108, 42, 57);
		o3->setBounds(112, 108, 42, 57);
		o4->setBounds(112, 108, 42, 57);
		p1->setBounds(158, 108, 42, 57);
		p2->setBounds(158, 108, 42, 57);
		p3->setBounds(158, 108, 42, 57);
		p4->setBounds(158, 108, 42, 57);
		f1->setBounds(242, 108, 42, 57);
		f2->setBounds(242, 108, 42, 57);
		f3->setBounds(242, 108, 42, 57);
		f4->setBounds(242, 108, 42, 57);
		dl1->setBounds(200, 108, 42, 57);
		dl2->setBounds(200, 108, 42, 57);
		dl3->setBounds(200, 108, 42, 57);
		dl4->setBounds(200, 108, 42, 57);
	}


	APAudioProcessor &proc;

	gin::ParamComponent::Ptr s1, w1, r1, b1, dp1, p1, o1, f1, dl1,
		s2, w2, r2, b2, dp2, p2, o2, f2, dl2,
		s3, w3, r3, b3, dp3, p3, o3, f3, dl3,
		s4, w4, r4, b4, dp4, p4, o4, f4, dl4;

	juce::Button *poly1, *poly2, *poly3, *poly4,
		*mono1, *mono2, *mono3, *mono4;

	int currentLFO{1};
	juce::TextButton select1{"1"}, select2{"2"}, select3{"3"}, select4{"4"};
	gin::LFOComponent *l1, *l2, *l3, *l4;
};

//==============================================================================
class ModBox : public gin::ParamBox
{
public:
	ModBox(const juce::String &name, APAudioProcessor &proc_)
		: gin::ParamBox(name), proc(proc_)
	{
		setName("mod");
		setTitle("  mod sources");
		addControl(modlist = new gin::ModSrcListBox(proc.modMatrix), 0, 0, 5, 4);
		modlist->setRowHeight(20);
	}

	gin::ModSrcListBox *modlist;
	APAudioProcessor &proc;
};

//==============================================================================
class GlobalBox : public gin::ParamBox
{
public:
	GlobalBox(const juce::String &name, APAudioProcessor &proc_)
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
	}

	void resized() override
	{
		gin::ParamBox::resized();
		legato->setBounds(0, 23, 56, 35);
		mono->setBounds(0, 58, 56, 35);
		glideMode->setBounds(56, 23, 56, 35);
	}

	gin::ParamComponent::Ptr legato = nullptr, mono = nullptr, glideMode = nullptr;
	APAudioProcessor &proc;
};

//==============================================================================

class AuxBox : public gin::ParamBox
{
public:
	AuxBox(const juce::String &name, APAudioProcessor &proc_)
		: gin::ParamBox(name), proc(proc_)
	{

		setName("aux");
		addEnable(proc.auxParams.enable);

		addControl(wave = new gin::Select(proc.auxParams.wave), 0, 0);
		addControl(env = new gin::Select(proc.auxParams.env), 0, 0);
		addControl(new APKnob(proc.auxParams.octave), 1, 0);
		addControl(new APKnob(proc.auxParams.volume), 2, 0);
		addControl(new APKnob(proc.auxParams.detune), 3, 0);
		addControl(new APKnob(proc.auxParams.spread), 4, 0);

		addControl(prefx = new gin::Select(proc.auxParams.prefx), 0, 1);
		addControl(filtertype = new gin::Select(proc.auxParams.filtertype), 0, 1);
		addControl(new APKnob(proc.auxParams.filtercutoff), 1, 1);
		addControl(new APKnob(proc.auxParams.filterres), 2, 1);
		addControl(new APKnob(proc.auxParams.filterkeytrack), 3, 1);
		addControl(new gin::Switch(proc.auxParams.ignorepb), 4, 1);
	}

	void resized() override
	{
		ParamBox::resized();
		wave->setBounds(0, 23, 56, 35);
		env->setBounds(0, 58, 56, 35);
		prefx->setBounds(0, 93, 56, 35);
		filtertype->setBounds(0, 128, 56, 35);
	}

	APAudioProcessor &proc;
	gin::ParamComponent::Ptr wave, env, prefx, filtertype;
};

//==============================================================================

class MainMatrixBox : public gin::ParamBox
{
public:
	MainMatrixBox(const juce::String &name, APAudioProcessor &proc_)
		: gin::ParamBox(name), proc(proc_)
	{
		setName("mtx");

		addControl(new APModMatrixBox(proc, proc.modMatrix), 0, 0, 5, 4);
		addAndMakeVisible(clearAllButton);
		clearAllButton.onClick = [this]
		{ clearAll(); };
	}

	void resized() override
	{
		gin::ParamBox::resized();
		clearAllButton.setBounds(getWidth() - 60, 0, 55, 23);
	}

	void clearAll()
	{
		auto &params = proc.getPluginParameters();
		for (auto *param : params)
		{
			if (param->getModIndex() == -1)
				continue;
			if (proc.modMatrix.isModulated(gin::ModDstId(param->getModIndex())))
			{
				auto modSrcs = proc.modMatrix.getModSources(param);
				for (auto &modSrc : modSrcs)
				{
					proc.modMatrix.clearModDepth(modSrc, gin::ModDstId(param->getModIndex()));
				}
			}
		}
	}

	juce::TextButton clearAllButton{"Clear All"};

	APAudioProcessor &proc;
};

//==============================================================================

class MsegBox : public gin::ParamBox
{
public:
	MsegBox(APAudioProcessor &proc_)
		: gin::ParamBox("  MSEG"), proc(proc_), msegComponent1(proc.mseg1Data), msegComponent2(proc.mseg2Data),
		  msegComponent3(proc.mseg3Data), msegComponent4(proc.mseg4Data)
	{
		setName("MSEG");

		msegComponent1.setParams(proc.mseg1Params.sync, proc.mseg1Params.rate, proc.mseg1Params.beat,
								 proc.mseg1Params.depth, proc.mseg1Params.offset, proc.mseg1Params.phase, proc.mseg1Params.enable,
								 proc.mseg1Params.xgrid, proc.mseg1Params.ygrid, proc.mseg1Params.loop);
		msegComponent1.setEditable(true);
		msegComponent1.setDrawMode(true, static_cast<gin::MSEGComponent::DrawMode>(proc.mseg1Params.drawmode->getUserValue()));

		msegComponent2.setParams(proc.mseg2Params.sync, proc.mseg2Params.rate, proc.mseg2Params.beat,
								 proc.mseg2Params.depth, proc.mseg2Params.offset, proc.mseg2Params.phase, proc.mseg2Params.enable,
								 proc.mseg2Params.xgrid, proc.mseg2Params.ygrid, proc.mseg2Params.loop);
		msegComponent2.setEditable(true);
		msegComponent2.setDrawMode(true, static_cast<gin::MSEGComponent::DrawMode>(proc.mseg2Params.drawmode->getUserValue()));

		msegComponent3.setParams(proc.mseg3Params.sync, proc.mseg3Params.rate, proc.mseg3Params.beat,
								 proc.mseg3Params.depth, proc.mseg3Params.offset, proc.mseg3Params.phase, proc.mseg3Params.enable,
								 proc.mseg3Params.xgrid, proc.mseg3Params.ygrid, proc.mseg3Params.loop);
		msegComponent3.setEditable(true);
		msegComponent3.setDrawMode(true, static_cast<gin::MSEGComponent::DrawMode>(proc.mseg3Params.drawmode->getUserValue()));

		msegComponent4.setParams(proc.mseg4Params.sync, proc.mseg4Params.rate, proc.mseg4Params.beat,
								 proc.mseg4Params.depth, proc.mseg4Params.offset, proc.mseg4Params.phase, proc.mseg4Params.enable,
								 proc.mseg4Params.xgrid, proc.mseg4Params.ygrid, proc.mseg4Params.loop);
		msegComponent4.setEditable(true);
		msegComponent4.setDrawMode(true, static_cast<gin::MSEGComponent::DrawMode>(proc.mseg4Params.drawmode->getUserValue()));

		addModSource(poly4 = new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcMSEG4, true));
		addModSource(poly3 = new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcMSEG3, true));
		addModSource(poly2 = new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcMSEG2, true));
		addModSource(poly1 = new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcMSEG1, true));

		addControl(r1 = new APKnob(proc.mseg1Params.rate), 0, 0);
		addControl(r2 = new APKnob(proc.mseg2Params.rate), 0, 0);
		addControl(r3 = new APKnob(proc.mseg3Params.rate), 0, 0);
		addControl(r4 = new APKnob(proc.mseg4Params.rate), 0, 0);

		addControl(b1 = new gin::Select(proc.mseg1Params.beat), 0, 0);
		addControl(b2 = new gin::Select(proc.mseg2Params.beat), 0, 0);
		addControl(b3 = new gin::Select(proc.mseg3Params.beat), 0, 0);
		addControl(b4 = new gin::Select(proc.mseg4Params.beat), 0, 0);

		addControl(s1 = new gin::Select(proc.mseg1Params.sync), 1, 0);
		addControl(s2 = new gin::Select(proc.mseg2Params.sync), 1, 0);
		addControl(s3 = new gin::Select(proc.mseg3Params.sync), 1, 0);
		addControl(s4 = new gin::Select(proc.mseg4Params.sync), 1, 0);

		addControl(l1 = new gin::Select(proc.mseg1Params.loop), 2, 0);
		addControl(l2 = new gin::Select(proc.mseg2Params.loop), 2, 0);
		addControl(l3 = new gin::Select(proc.mseg3Params.loop), 2, 0);
		addControl(l4 = new gin::Select(proc.mseg4Params.loop), 2, 0);

		addControl(dp1 = new APKnob(proc.mseg1Params.depth), 3, 0);
		addControl(dp2 = new APKnob(proc.mseg2Params.depth), 3, 0);
		addControl(dp3 = new APKnob(proc.mseg3Params.depth), 3, 0);
		addControl(dp4 = new APKnob(proc.mseg4Params.depth), 3, 0);

		addControl(o1 = new APKnob(proc.mseg1Params.offset), 4, 0);
		addControl(o2 = new APKnob(proc.mseg2Params.offset), 4, 0);
		addControl(o3 = new APKnob(proc.mseg3Params.offset), 4, 0);
		addControl(o4 = new APKnob(proc.mseg4Params.offset), 4, 0);

		addControl(dr1 = new gin::Select(proc.mseg1Params.draw), 5, 0);
		addControl(dr2 = new gin::Select(proc.mseg2Params.draw), 5, 0);
		addControl(dr3 = new gin::Select(proc.mseg3Params.draw), 5, 0);
		addControl(dr4 = new gin::Select(proc.mseg4Params.draw), 5, 0);

		addControl(m1 = new gin::Select(proc.mseg1Params.drawmode), 6, 0);
		addControl(m2 = new gin::Select(proc.mseg2Params.drawmode), 6, 0);
		addControl(m3 = new gin::Select(proc.mseg3Params.drawmode), 6, 0);
		addControl(m4 = new gin::Select(proc.mseg4Params.drawmode), 6, 0);

		addControl(x1 = new gin::Select(proc.mseg1Params.xgrid));
		addControl(x2 = new gin::Select(proc.mseg2Params.xgrid));
		addControl(x3 = new gin::Select(proc.mseg3Params.xgrid));
		addControl(x4 = new gin::Select(proc.mseg4Params.xgrid));

		addControl(y1 = new gin::Select(proc.mseg1Params.ygrid));
		addControl(y2 = new gin::Select(proc.mseg2Params.ygrid));
		addControl(y3 = new gin::Select(proc.mseg3Params.ygrid));
		addControl(y4 = new gin::Select(proc.mseg4Params.ygrid));

		// rate, beat, sync, loop, depth, offset, draw, drawmode, xgrid, ygrid
		// r, b, s, l, d, o, d, m, x, y
		// msegComponent
		// msegDstSelector
		watchParam(proc.mseg1Params.sync);
		watchParam(proc.mseg2Params.sync);
		watchParam(proc.mseg3Params.sync);
		watchParam(proc.mseg4Params.sync);
		watchParam(proc.mseg1Params.draw);
		watchParam(proc.mseg2Params.draw);
		watchParam(proc.mseg3Params.draw);
		watchParam(proc.mseg4Params.draw);
		watchParam(proc.mseg1Params.drawmode);
		watchParam(proc.mseg2Params.drawmode);
		watchParam(proc.mseg3Params.drawmode);
		watchParam(proc.mseg4Params.drawmode);
		addAndMakeVisible(msegComponent1);
		addAndMakeVisible(msegComponent2);
		addAndMakeVisible(msegComponent3);
		addAndMakeVisible(msegComponent4);
		addAndMakeVisible(select1);
		addAndMakeVisible(select2);
		addAndMakeVisible(select3);
		addAndMakeVisible(select4);
		addAndMakeVisible(learn1);
		addAndMakeVisible(learn2);
		addAndMakeVisible(learn3);
		addAndMakeVisible(learn4);

		select1.onClick = [this]()
		{ show(1); };
		select2.onClick = [this]()
		{ show(2); };
		select3.onClick = [this]()
		{ show(3); };
		select4.onClick = [this]()
		{ show(4); };

		msegComponent1.phaseCallback = [this]()
		{
			std::vector<float> auxVals = proc.auxSynth.getMSEG1Phases();
			std::vector<float> synthVals = proc.synth.getMSEG1Phases();
			synthVals.insert(synthVals.end(), auxVals.begin(), auxVals.end());
			return synthVals;
		};
		msegComponent2.phaseCallback = [this]()
		{
			std::vector<float> auxVals = proc.auxSynth.getMSEG2Phases();
			std::vector<float> synthVals = proc.synth.getMSEG2Phases();
			synthVals.insert(synthVals.end(), auxVals.begin(), auxVals.end());
			return synthVals;
		};
		msegComponent3.phaseCallback = [this]()
		{
			std::vector<float> auxVals = proc.auxSynth.getMSEG3Phases();
			std::vector<float> synthVals = proc.synth.getMSEG3Phases();
			synthVals.insert(synthVals.end(), auxVals.begin(), auxVals.end());
			return synthVals;
		};
		msegComponent4.phaseCallback = [this]()
		{
			std::vector<float> auxVals = proc.auxSynth.getMSEG4Phases();
			std::vector<float> synthVals = proc.synth.getMSEG4Phases();
			synthVals.insert(synthVals.end(), auxVals.begin(), auxVals.end());
			return synthVals;
		};

		show(1);
	}

	void show(int selected)
	{
		for (gin::ParamComponent::Ptr c : {
				 r1, b1, s1, l1, dp1, o1, dr1, m1, x1, y1,
				 r2, b2, s2, l2, dp2, o2, dr2, m2, x2, y2,
				 r3, b3, s3, l3, dp3, o3, dr3, m3, x3, y3,
				 r4, b4, s4, l4, dp4, o4, dr4, m4, x4, y4})
		{
			c->setVisible(false);
		}
		for (juce::Button *c : {poly1, poly2, poly3, poly4}) 
		{
			c->setVisible(false);
		}
		for (auto &viz : {&msegComponent1, &msegComponent2, &msegComponent3, &msegComponent4})
		{
			viz->setVisible(false);
		}
		select1.setToggleState(false, juce::dontSendNotification);
		select2.setToggleState(false, juce::dontSendNotification);
		select3.setToggleState(false, juce::dontSendNotification);
		select4.setToggleState(false, juce::dontSendNotification);
		switch (selected)
		{
		case 1:
			currentMSEG = 1;
			r1->setVisible(!proc.mseg1Params.sync->isOn());
			b1->setVisible(proc.mseg1Params.sync->isOn());
			s1->setVisible(true);
			l1->setVisible(true);
			dp1->setVisible(true);
			o1->setVisible(true);
			dr1->setVisible(true);
			m1->setVisible(true);
			x1->setVisible(true);
			y1->setVisible(true);
			poly1->setVisible(true);
			msegComponent1.setVisible(true);
			select1.setToggleState(true, juce::dontSendNotification);
			break;
		case 2:
			currentMSEG = 2;
			r2->setVisible(!proc.mseg2Params.sync->isOn());
			b2->setVisible(proc.mseg2Params.sync->isOn());
			s2->setVisible(true);
			l2->setVisible(true);
			dp2->setVisible(true);
			o2->setVisible(true);
			dr2->setVisible(true);
			m2->setVisible(true);
			x2->setVisible(true);
			y2->setVisible(true);
			poly2->setVisible(true);
			msegComponent2.setVisible(true);
			select2.setToggleState(true, juce::dontSendNotification);
			break;
		case 3:
			currentMSEG = 3;
			r3->setVisible(!proc.mseg3Params.sync->isOn());
			b3->setVisible(proc.mseg3Params.sync->isOn());
			s3->setVisible(true);
			l3->setVisible(true);
			dp3->setVisible(true);
			o3->setVisible(true);
			dr3->setVisible(true);
			m3->setVisible(true);
			x3->setVisible(true);
			y3->setVisible(true);
			poly3->setVisible(true);
			msegComponent3.setVisible(true);
			select3.setToggleState(true, juce::dontSendNotification);
			break;
		case 4:
			currentMSEG = 4;
			r4->setVisible(!proc.mseg4Params.sync->isOn());
			b4->setVisible(proc.mseg4Params.sync->isOn());
			s4->setVisible(true);
			l4->setVisible(true);
			dp4->setVisible(true);
			o4->setVisible(true);
			dr4->setVisible(true);
			m4->setVisible(true);
			x4->setVisible(true);
			y4->setVisible(true);
			poly4->setVisible(true);
			msegComponent4.setVisible(true);
			select4.setToggleState(true, juce::dontSendNotification);
			break;
		}
		paramChanged();
	}

	void paramChanged() override
	{
		gin::ParamBox::paramChanged();
		if (currentMSEG == 1)
		{
			bool sync1 = proc.mseg1Params.sync->getUserValueBool();
			r1->setVisible(!sync1);
			b1->setVisible(sync1);
		}
		if (currentMSEG == 2)
		{
			bool sync2 = proc.mseg2Params.sync->getUserValueBool();
			r2->setVisible(!sync2);
			b2->setVisible(sync2);
		}
		if (currentMSEG == 3)
		{
			bool sync3 = proc.mseg3Params.sync->getUserValueBool();
			r3->setVisible(!sync3);
			b3->setVisible(sync3);
		}
		if (currentMSEG == 4)
		{
			bool sync4 = proc.mseg4Params.sync->getUserValueBool();
			r4->setVisible(!sync4);
			b4->setVisible(sync4);
		}

		if (proc.mseg1Params.draw->getUserValueBool())
		{
			msegComponent1.setDrawMode(true,
									   static_cast<gin::MSEGComponent::DrawMode>(proc.mseg1Params.drawmode->getUserValue()));
		}
		else
		{
			msegComponent1.setDrawMode(false, gin::MSEGComponent::DrawMode::step);
		}

		if (proc.mseg2Params.draw->getUserValueBool())
		{
			msegComponent2.setDrawMode(true,
									   static_cast<gin::MSEGComponent::DrawMode>(proc.mseg2Params.drawmode->getUserValue()));
		}
		else
		{
			msegComponent2.setDrawMode(false, gin::MSEGComponent::DrawMode::step);
		}

		if (proc.mseg3Params.draw->getUserValueBool())
		{
			msegComponent3.setDrawMode(true,
									   static_cast<gin::MSEGComponent::DrawMode>(proc.mseg3Params.drawmode->getUserValue()));
		}
		else
		{
			msegComponent3.setDrawMode(false, gin::MSEGComponent::DrawMode::step);
		}

		if (proc.mseg4Params.draw->getUserValueBool())
		{
			msegComponent4.setDrawMode(true,
									   static_cast<gin::MSEGComponent::DrawMode>(proc.mseg4Params.drawmode->getUserValue()));
		}
		else
		{
			msegComponent4.setDrawMode(false, gin::MSEGComponent::DrawMode::step);
		}
	}

	void resized() override
	{
		gin::ParamBox::resized();
		msegComponent1.setBounds(0, 93, getWidth(), getHeight() - 93);
		msegComponent2.setBounds(0, 93, getWidth(), getHeight() - 93);
		msegComponent3.setBounds(0, 93, getWidth(), getHeight() - 93);
		msegComponent4.setBounds(0, 93, getWidth(), getHeight() - 93);
		x1->setBounds(7 * 56, 23, 56, 35);
		x2->setBounds(7 * 56, 23, 56, 35);
		x3->setBounds(7 * 56, 23, 56, 35);
		x4->setBounds(7 * 56, 23, 56, 35);
		y1->setBounds(7 * 56, 58, 56, 35);
		y2->setBounds(7 * 56, 58, 56, 35);
		y3->setBounds(7 * 56, 58, 56, 35);
		y4->setBounds(7 * 56, 58, 56, 35);
		select1.setBounds(100, 0, 20, 23);
		select2.setBounds(120, 0, 20, 23);
		select3.setBounds(140, 0, 20, 23);
		select4.setBounds(160, 0, 20, 23);
	}

	APAudioProcessor &proc;
	// r, b, s, l, d, o, d, m, x, y
	gin::ParamComponent::Ptr r1, b1, s1, l1, dp1, o1, dr1, m1, x1, y1,
		r2, b2, s2, l2, dp2, o2, dr2, m2, x2, y2,
		r3, b3, s3, l3, dp3, o3, dr3, m3, x3, y3,
		r4, b4, s4, l4, dp4, o4, dr4, m4, x4, y4;
	juce::Button *poly1, *poly2, *poly3, *poly4;

	// create textbuttons for learn

	gin::MSEGComponent msegComponent1, msegComponent2, msegComponent3, msegComponent4;
	int currentMSEG{1};
	juce::TextButton select1{"1"}, select2{"2"}, select3{"3"}, select4{"4"};
	juce::TextButton learn1{"learn"}, learn2{"learn"}, learn3{"learn"}, learn4{"learn"};
};

//==============================================================================

class MacrosBox : public gin::ParamBox
{
public:
	MacrosBox(APAudioProcessor &proc_)
		: gin::ParamBox("  macros"), proc(proc_)
	{
		setName("macros");

		addControl(new APKnob(proc.macroParams.macro1), 0, 0);
		addControl(new APKnob(proc.macroParams.macro2), 1, 0);
		addControl(new APKnob(proc.macroParams.macro3), 2, 0);

		addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.macroSrc3, true));
		addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.macroSrc2, true));
		addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.macroSrc1, true));

		addAndMakeVisible(midiLearnButton1);
		addAndMakeVisible(midiLearnButton2);
		addAndMakeVisible(midiLearnButton3);
		addChildComponent(clear1);
		addChildComponent(clear2);
		addChildComponent(clear3);

		proc.macroParams.macro1cc->addListener(this);
		proc.macroParams.macro2cc->addListener(this);
		proc.macroParams.macro3cc->addListener(this);

		clear1.onClick = [this]()
		{ cancelAssignment(1); };
		clear2.onClick = [this]()
		{ cancelAssignment(2); };
		clear3.onClick = [this]()
		{ cancelAssignment(3); };

		midiLearnButton1.setMacroNumber(1);
		midiLearnButton2.setMacroNumber(2);
		midiLearnButton3.setMacroNumber(3);
	}

	void cancelAssignment(int macroNumber)
	{
		switch (macroNumber)
		{
		case 1:
			proc.macroParams.macro1cc->setValue(-1.f);
			midiLearnButton1.setCCString("Learn");
			midiLearnButton1.setLearning(false);
			clear1.setVisible(false);
			break;
		case 2:
			proc.macroParams.macro2cc->setValue(-1.f);
			midiLearnButton2.setCCString("Learn");
			midiLearnButton2.setLearning(false);
			clear2.setVisible(false);
			break;
		case 3:
			proc.macroParams.macro3cc->setValue(-1.f);
			midiLearnButton3.setCCString("Learn");
			midiLearnButton3.setLearning(false);
			clear3.setVisible(false);
			break;
		}
	}
	void valueUpdated(gin::Parameter *p) override
	{
		if (p == proc.macroParams.macro1cc)
		{
			auto ccValue = proc.macroParams.macro1cc->getUserValueInt();
			if (ccValue >= 0)
			{
				midiLearnButton1.setCCString("CC " + proc.macroParams.macro1cc->getUserValueText());
				clear1.setVisible(true);
			}
			else
			{
				midiLearnButton1.setCCString("Learn");
				midiLearnButton1.setLearning(false);
				clear1.setVisible(false);
			}
		}
		if (p == proc.macroParams.macro2cc)
		{
			auto ccValue = proc.macroParams.macro2cc->getUserValueInt();
			if (ccValue >= 0)
			{
				midiLearnButton2.setCCString("CC " + proc.macroParams.macro2cc->getUserValueText());
				clear2.setVisible(true);
			}
			else
			{
				midiLearnButton2.setCCString("Learn");
				midiLearnButton2.setLearning(false);
				clear2.setVisible(false);
			}
		}
		if (p == proc.macroParams.macro3cc)
		{
			auto ccValue = proc.macroParams.macro3cc->getUserValueInt();
			if (ccValue >= 0)
			{
				midiLearnButton3.setCCString("CC " + proc.macroParams.macro3cc->getUserValueText());
				clear3.setVisible(true);
			}
			else
			{
				midiLearnButton3.setCCString("Learn");
				midiLearnButton3.setLearning(false);
				clear3.setVisible(false);
			}
		}
	}

	void resized() override
	{
		gin::ParamBox::resized();
		midiLearnButton1.setBounds(0, 93, 56, 35);
		midiLearnButton2.setBounds(56, 93, 56, 35);
		midiLearnButton3.setBounds(112, 93, 56, 35);
		clear1.setBounds(0, 128, 56, 35);
		clear2.setBounds(56, 128, 56, 35);
		clear3.setBounds(112, 128, 56, 35);
	}

	class MIDILearnButton : public juce::Label
	{
	public:
		MIDILearnButton(APAudioProcessor &p) : proc(p)
		{
			setEditable(false, false, false);
			setJustificationType(juce::Justification::left);
			setText("Learn", juce::dontSendNotification);
			setLookAndFeel(&midilearnLNF);
		}

		~MIDILearnButton() override
		{
			setLookAndFeel(nullptr);
		}

		void mouseDown(const juce::MouseEvent & /*ev*/) override
		{
			if (thisMacroNumber == 1)
			{
				if (proc.macroParams.macro1cc->getUserValue() > 0.f)
				{
					return;
				}
			}
			if (thisMacroNumber == 2)
			{
				if (proc.macroParams.macro2cc->getUserValue() > 0.f)
				{
					return;
				}
			}
			if (thisMacroNumber == 3)
			{
				if (proc.macroParams.macro3cc->getUserValue() > 0.f)
				{
					return;
				}
			}
			learning = !learning;
			if (learning)
			{
				proc.macroParams.learning->setUserValue(static_cast<float>(thisMacroNumber));
				setText("Learning", juce::dontSendNotification);
			}
			else
			{
				proc.macroParams.learning->setValue(0.0f);
				setText("Learn", juce::dontSendNotification);
			}
		}

		void setMacroNumber(int n)
		{
			thisMacroNumber = n;
		}

		void setCCString(const juce::String &s)
		{
			currentAssignment = s;
			setText(s, juce::dontSendNotification);
		}

		void setLearning(bool shouldLearn)
		{
			learning = shouldLearn;
		}

		class MIDILearnLNF : public APLNF
		{
		public:
			MIDILearnLNF()
			{
			}

			void drawLabel(juce::Graphics &g, juce::Label &label) override
			{
				auto rc = label.getLocalBounds();
				g.setColour(juce::Colours::white);
				g.setFont(14.f);
				g.drawText(label.getText(), rc, juce::Justification::centred);
			}

		} midilearnLNF;

		juce::String currentAssignment;
		APAudioProcessor &proc;
		bool learning{false};
		int mididCC{-1}, thisMacroNumber{0};
	}; // MIDILearnButton

	APAudioProcessor &proc;
	MIDILearnButton midiLearnButton1{proc}, midiLearnButton2{proc}, midiLearnButton3{proc};
	juce::TextButton clear1{"Clear", "Clear"}, clear2{"Clear", "Clear"}, clear3{"Clear", "Clear"};
}; // MacrosBox

//==============================================================================

class VolumeBox : public gin::ParamBox
{
public:
	VolumeBox(APAudioProcessor &proc_)
		: gin::ParamBox("  volume"), proc(proc_)
	{
		setName("  volume");
		addControl(level = new APKnob(proc.globalParams.level));
		addControl(aux = new APKnob(proc.auxParams.volume));
		addAndMakeVisible(levelMeter);
		level->setDisplayName("Main");
		aux->setDisplayName("Aux");
		proc.auxParams.enable->addListener(this);
		valueUpdated(proc.auxParams.enable);
	}

	void resized() override
	{
		gin::ParamBox::resized();
		level->setBounds(0, 43, 80, 100);
		aux->setBounds(80, 43, 80, 100);
		levelMeter.setBounds(280 - 38, 43, 30, 100);
	}

	void valueUpdated(gin::Parameter *p) override
	{
		if (p == proc.auxParams.enable)
		{
			aux->setEnabled(proc.auxParams.enable->isOn());
		}
	}

	APAudioProcessor &proc;
	APKnob *level, *aux;

	gin::LevelMeter levelMeter{proc.levelTracker};
};
