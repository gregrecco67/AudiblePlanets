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
#include "DSP/PluginProcessor.h"
#include "APColors.h"
#include "MoonKnob.h"
#include "APModAdditions.h"
#include "EnvelopeComponent.h"


inline void gradientRect (juce::Graphics& g, juce::Rectangle<int> rc, juce::Colour c1, juce::Colour c2)
{
    juce::ColourGradient gradient (c1, (float) rc.getX(), (float) rc.getY(), c2, (float) rc.getRight(), (float) rc.getBottom(), false);

    g.setGradientFill (gradient);
    g.fillRect (rc);
}

//===============================================================================

class OSCBox : public gin::ParamBox
{
public:
	OSCBox(APAudioProcessor& proc_, APAudioProcessor::OSCParams& params_, int num)
		: gin::ParamBox(juce::String("  OSC ") += (num+1)), proc(proc_), params(params_)
	{
		addControl(c1 = new APKnob(params.coarse), 0, 0); // coarse
		addControl(f1 = new APKnob(params.fine), 1, 0); // fine
		addControl(v1 = new APKnob(params.volume), 2, 0); // volume
		// addControl(t1 = new APKnob(params.tones), 3, 0); // tones
		addControl(p1 = new APKnob(params.phase), 2, 1); // phase

		c1->setLookAndFeel(lnfs[num]);
		f1->setLookAndFeel(lnfs[num]);
		v1->setLookAndFeel(lnfs[num]);

		addControl(wave1 = new gin::Select(params.wave)); // saw
		addControl(env1 = new gin::Select(params.env)); // env select
		addControl(fixed1 = new gin::Select(params.fixed)); // fixed
		watchParam(params.fixed);
		watchParam(params.wave);
		watchParam(params.coarse);
		watchParam(params.fine);
		addAndMakeVisible(fixedHz1);
		setColour(juce::TextButton::buttonOnColourId, juce::Colours::beige);

		addAndMakeVisible(coarseLabel);
		coarseLabel.setJustificationType(juce::Justification::centredBottom);
		coarseLabel.setFont(juce::Font(14.0f));
		fixedHz1.setJustificationType(juce::Justification::centred);

	}

	void paint(juce::Graphics& g) override
	{
		auto rc = getLocalBounds().withTrimmedTop(23);
		juce::ColourGradient gradient(
			findColour(gin::PluginLookAndFeel::matte2ColourId).darker(0.05f),
			(float)rc.getX(), (float)rc.getY(),
			findColour(gin::PluginLookAndFeel::matte2ColourId).darker(0.08f),
			(float)rc.getWidth(), (float)rc.getBottom(), false);
		gradient.addColour(0.3f, findColour(gin::PluginLookAndFeel::matte2ColourId).brighter(0.08f));
		g.setGradientFill(gradient);
		g.fillRect(rc);
		g.setColour(juce::Colour(0xFF888888));
		if (right)
			g.fillRect(getWidth() - 1, 0, 1, getHeight());
	}

	bool right{ false };
	void setRight(bool r) { right = r; }
	
	~OSCBox() override
	{
		c1->setLookAndFeel(nullptr);
		f1->setLookAndFeel(nullptr);
		v1->setLookAndFeel(nullptr);
	}

	class APLookAndFeel1 : public APLNF
	{
	public:
		APLookAndFeel1()
		{
			setColour(juce::Slider::rotarySliderFillColourId, APColors::red);
			setColour(juce::Slider::trackColourId, juce::Colours::black);
		}
	};
	class APLookAndFeel2 : public APLNF
	{
	public:
		APLookAndFeel2()
		{
			setColour(juce::Slider::rotarySliderFillColourId, APColors::yellow);
			setColour(juce::Slider::trackColourId, juce::Colours::black);
		}
	};
	class APLookAndFeel3 : public APLNF
	{
	public:
		APLookAndFeel3()
		{
			setColour(juce::Slider::rotarySliderFillColourId, APColors::green);
			setColour(juce::Slider::trackColourId, juce::Colours::black);
		}
	};
	class APLookAndFeel4 : public APLNF
	{
	public:
		APLookAndFeel4()
		{
			setColour(juce::Slider::rotarySliderFillColourId, APColors::blue);
			setColour(juce::Slider::trackColourId, juce::Colours::black);
		}
	};

	void paramChanged() override
	{
		gin::ParamBox::paramChanged();
		if (params.fixed->isOn())
		{
			fixedHz1.setVisible(true);
			fixedHz1.setText(juce::String((params.coarse->getUserValue() + params.fine->getUserValue()) * 100, 2) + juce::String(" Hz"), juce::dontSendNotification);
		}
		else
		{
			fixedHz1.setVisible(false);
		}
		auto coarseString = juce::String(params.coarse->getUserValueInt());
		if (params.fine->getUserValue() > 0.01f)
			coarseString += "+";
		else if (params.fine->getUserValue() < -0.01f)
			coarseString += "-";
		else
			coarseString += "x";
		coarseLabel.setText(coarseString, juce::dontSendNotification);
	}

	void resized() override
	{
		gin::ParamBox::resized();
		fixedHz1.setBounds(56, 23 + 70 + 10, 56, 15); // 23/70/10 header, first row, padding
		p1->setBounds(56 * 2, 70 * 1 + 23, 56, 70);
		wave1->setBounds(56 * 3, 93, 56, 70);
		env1->setBounds(56 * 3, 23, 56, 70);
		fixed1->setBounds(56, 128, 56, 35);
		coarseLabel.setBounds(4, 93, 52, 45);
	}

	APLookAndFeel1 lnf1;
	APLookAndFeel2 lnf2;
	APLookAndFeel3 lnf3;
	APLookAndFeel4 lnf4;
	std::array<APLNF*, 4> lnfs{ &lnf1, &lnf2, &lnf3, &lnf4 };
	APAudioProcessor& proc;
	gin::ParamComponent::Ptr c1, f1, v1, t1, p1, wave1, env1, fixed1;
	APAudioProcessor::OSCParams& params;

	juce::Label fixedHz1, coarseLabel;

	int currentOsc{ 1 };
};

//==============================================================================
class ENVBox : public gin::ParamBox
{
public:
	ENVBox(APAudioProcessor &proc_, APAudioProcessor::ENVParams& params_, int num)
		: gin::ParamBox(juce::String("  ENV ") += (num+1)), proc(proc_), params(params_)
	{
		// in reverse order
		switch (num) {
		case 3:
			addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcEnv4, true));
			break;
		case 2:
			addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcEnv3, true));
			break;
		case 1:
			addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcEnv2, true));
			break;
		case 0:
			addModSource(new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcEnv1, true));
			break;
		}
		addControl(a1 = new APKnob(params.attack), 0, 0);
		addControl(d1 = new APKnob(params.decay), 1, 0);
		addControl(s1 = new APKnob(params.sustain), 2, 0);
		addControl(r1 = new APKnob(params.release), 3, 0);
		addControl(ac1 = new APKnob(params.acurve, true), 4, 0);
		addControl(dc1 = new APKnob(params.drcurve, true), 5, 0);
		addControl(rpt1 = new gin::Select(params.syncrepeat), 4, 1);
		addControl(beats1 = new gin::Select(params.duration), 5, 1);
		addControl(rate1 = new APKnob(params.time), 5, 1);

		beats1->setVisible(false);
		rate1->setVisible(false);

		watchParam(params.syncrepeat);

		envViz = new EnvelopeComponent(proc, num+1);
		addAndMakeVisible(envViz);
	}

	void paint (juce::Graphics& g) override
    {
        auto rc = getLocalBounds().withTrimmedTop (23);
		juce::ColourGradient gradient(
			findColour (gin::PluginLookAndFeel::matte2ColourId).darker(0.05f),
			(float) rc.getX(), (float) rc.getY(), 
			findColour (gin::PluginLookAndFeel::matte2ColourId).darker(0.08f),
			(float) rc.getWidth(), (float) rc.getBottom(), false);
		gradient.addColour (0.3f, findColour (gin::PluginLookAndFeel::matte2ColourId).brighter(0.08f));
		g.setGradientFill (gradient);
		g.fillRect (rc);
		g.setColour(juce::Colour(0xFF888888));
		if (right)
			g.fillRect(getWidth() - 1, 0, 1, getHeight());
    }

	bool right{false};
	void setRight(bool r) { right = r; }

	void paramChanged() override
	{
		gin::ParamBox::paramChanged();
		auto choice = params.syncrepeat->getUserValueInt();
		switch (choice) {
		case 0:
			beats1->setVisible(false);
			rate1->setVisible(false);
			break;
		case 1:
			beats1->setVisible(true);
			rate1->setVisible(false);
			break;
		case 2:
			beats1->setVisible(false);
			rate1->setVisible(true);
			break;
		}
	}

	void resized() override
	{
		gin::ParamBox::resized();
		envViz->setBounds(0, 23 + 70, 56 * 4, 70);
	}

	APAudioProcessor &proc;
	gin::ParamComponent::Ptr a1, d1, s1, r1, ac1, dc1, rpt1, beats1, rate1;

	EnvelopeComponent* envViz;
	int currentEnv{1};
	APAudioProcessor::ENVParams& params;
};

//==============================================================================
class TimbreBox : public gin::ParamBox
{
public:
	TimbreBox(const juce::String &name, APAudioProcessor &proc)
		: gin::ParamBox(name)
	{
		setName(name);
		addControl(equant = new APKnob(proc.timbreParams.equant, true), 0, 0);
		addControl(new APKnob(proc.timbreParams.pitch), 0, 1);
		addControl(algo = new APKnob(proc.timbreParams.algo), 1, 0);
		addControl(new APKnob(proc.globalParams.velSens), 1, 1);
	}
	
	void paint (juce::Graphics& g) override
    {
        auto rc = getLocalBounds().withTrimmedTop (23);
		juce::ColourGradient gradient(
			findColour (gin::PluginLookAndFeel::matte2ColourId).darker(0.05f),
			(float) rc.getX(), (float) rc.getY(), 
			findColour (gin::PluginLookAndFeel::matte2ColourId).darker(0.08f),
			(float) rc.getWidth(), (float) rc.getBottom(), false);
		gradient.addColour (0.3f, findColour (gin::PluginLookAndFeel::matte2ColourId).brighter(0.08f));
		g.setGradientFill (gradient);
		g.fillRect (rc);
		g.setColour(juce::Colour(0xFF888888));
		if (top)
			g.fillRect(0, 0, getWidth(), 1);
		if (right)
			g.fillRect(getWidth() - 1, 0, 1, getHeight());
		if (bottom)
			g.fillRect(0, getHeight() - 1, getWidth(), 1);
		if (left)
			g.fillRect(0, 0, 1, getHeight());
    }

	APKnob* equant, *algo, *squash;
	bool top{false}, right{false}, bottom{false}, left{false};
	void setTop(bool t) { top = t; }
	void setRight(bool r) { right = r; }
	void setBottom(bool b) { bottom = b; }
	void setLeft(bool l) { left = l; }
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
	
	void paint (juce::Graphics& g) override
    {
        auto rc = getLocalBounds().withTrimmedTop (23);
		juce::ColourGradient gradient(
			findColour (gin::PluginLookAndFeel::matte2ColourId).darker(0.05f),
			(float) rc.getX(), (float) rc.getY(), 
			findColour (gin::PluginLookAndFeel::matte2ColourId).darker(0.08f),
			(float) rc.getWidth(), (float) rc.getBottom(), false);
		gradient.addColour (0.3f, findColour (gin::PluginLookAndFeel::matte2ColourId).brighter(0.08f));
		g.setGradientFill (gradient);
		g.fillRect (rc);
		g.setColour(juce::Colour(0xFF888888));
		if (top)
			g.fillRect(0, 0, getWidth(), 1);
		if (right)
			g.fillRect(getWidth() - 1, 0, 1, getHeight());
		if (bottom)
			g.fillRect(0, getHeight() - 1, getWidth(), 1);
		if (left)
			g.fillRect(0, 0, 1, getHeight());
    }

	bool top{false}, right{false}, bottom{false}, left{false};
	void setTop(bool t) { top = t; }
	void setRight(bool r) { right = r; }
	void setBottom(bool b) { bottom = b; }
	void setLeft(bool l) { left = l; }

	APAudioProcessor &proc;
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
		addControl(modlist = new gin::ModSrcListBox(proc.modMatrix), 0.f, 0.f, 5.f, 4.3f);
		modlist->setRowHeight(20);
	}

	void paint (juce::Graphics& g) override
    {
        auto rc = getLocalBounds().withTrimmedTop (23);
		juce::ColourGradient gradient(
			findColour (gin::PluginLookAndFeel::matte2ColourId).darker(0.05f),
			(float) rc.getX(), (float) rc.getY(), 
			findColour (gin::PluginLookAndFeel::matte2ColourId).darker(0.08f),
			(float) rc.getWidth(), (float) rc.getBottom(), false);
		gradient.addColour (0.3f, findColour (gin::PluginLookAndFeel::matte2ColourId).brighter(0.08f));
		g.setGradientFill (gradient);
		g.fillRect (rc);
		g.setColour(juce::Colour(0xFF888888));
		if (top)
			g.fillRect(0, 0, getWidth(), 1);
		if (right)
			g.fillRect(getWidth() - 1, 0, 1, getHeight());
		if (bottom)
			g.fillRect(0, getHeight() - 1, getWidth(), 1);
		if (left)
			g.fillRect(0, 0, 1, getHeight());
    }

	bool top{false}, right{false}, bottom{false}, left{false};
	void setTop(bool t) { top = t; }
	void setRight(bool r) { right = r; }
	void setBottom(bool b) { bottom = b; }
	void setLeft(bool l) { left = l; }
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

		addControl(vol = new APKnob(proc.globalParams.level), 2, 0);
		addControl(auxvol = new APKnob(proc.auxParams.volume), 2, 1);
		addControl(new APKnob(proc.globalParams.glideRate), 1, 1);
		addControl(new APKnob(proc.globalParams.pitchbendRange), 0, 1);
		auxvol->setDisplayName("Aux Vol.");
		addControl(mpe = new gin::Select(proc.globalParams.mpe));
		addControl(legato = new gin::Select(proc.globalParams.legato));
		addControl(mono = new gin::Select(proc.globalParams.mono));
		addControl(glideMode = new gin::Select(proc.globalParams.glideMode));

		vol->setLookAndFeel(&aplnf5);
		auxvol->setLookAndFeel(&aplnf5);
	}

	~GlobalBox() override { 
		vol->setLookAndFeel(nullptr); 
		auxvol->setLookAndFeel(nullptr);
	}

	void paint (juce::Graphics& g) override
    {
        auto rc = getLocalBounds().withTrimmedTop (23);
		juce::ColourGradient gradient(
			findColour (gin::PluginLookAndFeel::matte2ColourId).darker(0.05f),
			(float) rc.getX(), (float) rc.getY(), 
			findColour (gin::PluginLookAndFeel::matte2ColourId).darker(0.08f),
			(float) rc.getWidth(), (float) rc.getBottom(), false);
		gradient.addColour (0.3f, findColour (gin::PluginLookAndFeel::matte2ColourId).brighter(0.08f));
		g.setGradientFill (gradient);
		g.fillRect (rc);
		g.setColour(juce::Colour(0xFF888888));
		if (top)
			g.fillRect(0, 0, getWidth(), 1);
		if (right)
			g.fillRect(getWidth() - 1, 0, 1, getHeight());
		if (bottom)
			g.fillRect(0, getHeight() - 1, getWidth(), 1);
		if (left)
			g.fillRect(0, 0, 1, getHeight());
    }

	bool top{false}, right{false}, bottom{false}, left{false};
	void setTop(bool t) { top = t; }
	void setRight(bool r) { right = r; }
	void setBottom(bool b) { bottom = b; }
	void setLeft(bool l) { left = l; }

	void resized() override
	{
		gin::ParamBox::resized();
		legato->setBounds(0, 23, 56, 35);
		mono->setBounds(0, 58, 56, 35);
		glideMode->setBounds(56, 23, 56, 35);
		mpe->setBounds(56, 58, 56, 35);
	}

	class APLookAndFeel5 : public APLNF
	{
	public:
		APLookAndFeel5()
		{
			setColour(juce::Slider::rotarySliderFillColourId, 
				gin::CopperLookAndFeel::findColour(accentColourId));
			setColour(juce::Slider::trackColourId, 
				juce::Colours::black);
		}
	} aplnf5;

	gin::ParamComponent::Ptr legato = nullptr, mono = nullptr, glideMode = nullptr,
		mpe = nullptr;
	APKnob  *auxvol, *vol;
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

	void paint (juce::Graphics& g) override
    {
        auto rc = getLocalBounds().withTrimmedTop (23);
		juce::ColourGradient gradient(
			findColour (gin::PluginLookAndFeel::matte2ColourId).darker(0.05f),
			(float) rc.getX(), (float) rc.getY(), 
			findColour (gin::PluginLookAndFeel::matte2ColourId).darker(0.08f),
			(float) rc.getWidth(), (float) rc.getBottom(), false);
		gradient.addColour (0.3f, findColour (gin::PluginLookAndFeel::matte2ColourId).brighter(0.08f));
		g.setGradientFill (gradient);
		g.fillRect (rc);
		g.setColour(juce::Colour(0xFF888888));
		if (top)
			g.fillRect(0, 0, getWidth(), 1);
		if (right)
			g.fillRect(getWidth() - 1, 0, 1, getHeight());
		if (bottom)
			g.fillRect(0, getHeight() - 1, getWidth(), 1);
		if (left)
			g.fillRect(0, 0, 1, getHeight());
    }

	bool top{false}, right{false}, bottom{false}, left{false};
	void setTop(bool t) { top = t; }
	void setRight(bool r) { right = r; }
	void setBottom(bool b) { bottom = b; }
	void setLeft(bool l) { left = l; }
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

		addControl(new APModMatrixBox(proc, proc.modMatrix), 0.f, 0.f, 5.f, 4.3f);
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
		auto &pluginParams = proc.getPluginParameters();
		for (auto *param : pluginParams)
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

	void paint (juce::Graphics& g) override
    {
        auto rc = getLocalBounds().withTrimmedTop (23);
		juce::ColourGradient gradient(
			findColour (gin::PluginLookAndFeel::matte2ColourId).darker(0.05f),
			(float) rc.getX(), (float) rc.getY(), 
			findColour (gin::PluginLookAndFeel::matte2ColourId).darker(0.08f),
			(float) rc.getWidth(), (float) rc.getBottom(), false);
		gradient.addColour (0.3f, findColour (gin::PluginLookAndFeel::matte2ColourId).brighter(0.08f));
		g.setGradientFill (gradient);
		g.fillRect (rc);
		g.setColour(juce::Colour(0xFF888888));
		if (top)
			g.fillRect(0, 0, getWidth(), 1);
		if (right)
			g.fillRect(getWidth() - 1, 0, 1, getHeight());
		if (bottom)
			g.fillRect(0, getHeight() - 1, getWidth(), 1);
		if (left)
			g.fillRect(0, 0, 1, getHeight());
    }
	bool top{false}, right{false}, bottom{false}, left{false};
	juce::TextButton clearAllButton{"Clear All"};

	APAudioProcessor &proc;
};

//==============================================================================
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

	void paint (juce::Graphics& g) override
    {
        auto rc = getLocalBounds().withTrimmedTop (23);
		juce::ColourGradient gradient(
			findColour (gin::PluginLookAndFeel::matte2ColourId).darker(0.05f),
			(float) rc.getX(), (float) rc.getY(), 
			findColour (gin::PluginLookAndFeel::matte2ColourId).darker(0.08f),
			(float) rc.getWidth(), (float) rc.getBottom(), false);
		gradient.addColour (0.3f, findColour (gin::PluginLookAndFeel::matte2ColourId).brighter(0.08f));
		g.setGradientFill (gradient);
		g.fillRect (rc);
		g.setColour(juce::Colour(0xFF888888));
		if (top)
			g.fillRect(0, 0, getWidth(), 1);
		if (right)
			g.fillRect(getWidth() - 1, 0, 1, getHeight());
		if (bottom)
			g.fillRect(0, getHeight() - 1, getWidth(), 1);
		if (left)
			g.fillRect(0, 0, 1, getHeight());
    }

	bool top{false}, right{false}, bottom{false}, left{false};
	void setTop(bool t) { top = t; }
	void setRight(bool r) { right = r; }
	void setBottom(bool b) { bottom = b; }
	void setLeft(bool l) { left = l; }

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
/*
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

	void paint (juce::Graphics& g) override
    {
        auto rc = getLocalBounds().withTrimmedTop (23);
        gradientRect (g, rc, findColour (gin::PluginLookAndFeel::matte1ColourId), findColour (gin::PluginLookAndFeel::matte2ColourId));
		g.setColour(juce::Colour(0xFF888888));
		if (top)
			g.fillRect(0, 0, getWidth(), 1);
		if (right)
			g.fillRect(getWidth() - 1, 0, 1, getHeight());
		if (bottom)
			g.fillRect(0, getHeight() - 1, getWidth(), 1);
		if (left)
			g.fillRect(0, 0, 2, getHeight());
    }

	bool top{false}, right{false}, bottom{false}, left{false};
	void setTop(bool t) { top = t; }
	void setRight(bool r) { right = r; }
	void setBottom(bool b) { bottom = b; }
	void setLeft(bool l) { left = l; }

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
*/
