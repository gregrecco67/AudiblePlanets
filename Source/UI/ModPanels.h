#pragma once

#include <gin_plugin/gin_plugin.h>
#include "DSP/PluginProcessor.h"
#include "APColors.h"
#include "APModAdditions.h"
#include "EnvelopeComponent.h"



//==============================================================================
class LFOBox : public gin::ParamBox
{
public:
	LFOBox(APAudioProcessor &proc_, APAudioProcessor::LFOParams& lfoParams_, int num_)
		: ParamBox(juce::String("  LFO ") += juce::String(num_)), proc(proc_), lfoParams(lfoParams_), num(num_)
	{
		setName("lfo");

		addModSource(poly1 = new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcLFO1, true));
		addModSource(mono1 = new gin::ModulationSourceButton(proc.modMatrix, proc.modSrcMonoLFO1, false));
        poly1->getProperties().set("polysrc", true);

		addControl(r1 = new APKnob(lfoParams.rate), 0, 0);
		addControl(b1 = new gin::Select(lfoParams.beat), 0, 0);

		addControl(s1 = new gin::Select(lfoParams.sync));

		addControl(w1 = new gin::Select(lfoParams.wave));

		addControl(dp1 = new APKnob(lfoParams.depth, true), 1, 1);

		addControl(o1 = new APKnob(lfoParams.offset, true), 1, 2);

		addControl(p1 = new APKnob(lfoParams.phase, true), 1, 3);

		addControl(dl1 = new APKnob(lfoParams.delay), 1, 4);

		addControl(f1 = new APKnob(lfoParams.fade, true), 1, 5);

		l1 = new gin::LFOComponent();
		l1->phaseCallback = [this]
		{
			std::vector<float> res;
			res.push_back(proc.monoLFOs[0]->getCurrentPhase());
			return res;
		};
		l1->setParams(lfoParams.wave, lfoParams.sync, lfoParams.rate, lfoParams.beat, lfoParams.depth,
					  lfoParams.offset, lfoParams.phase, lfoParams.enable);
		addControl(l1, 1, 0, 4, 1);


		watchParam(lfoParams.sync);
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
        r1->setVisible(!proc.lfo1Params.sync->isOn());
        b1->setVisible(proc.lfo1Params.sync->isOn());
	}

	void resized() override
	{
		gin::ParamBox::resized();
		s1->setBounds(0, 93, 56, 35);
		w1->setBounds(0, 128, 56, 35);
		o1->setBounds(112, 108, 42, 57);
		p1->setBounds(158, 108, 42, 57);
		f1->setBounds(242, 108, 42, 57);
		dl1->setBounds(200, 108, 42, 57);
	}


	APAudioProcessor &proc;

	gin::ParamComponent::Ptr s1, w1, r1, b1, dp1, p1, o1, f1, dl1;

	juce::Button *poly1, *mono1;

	int currentLFO{1};
    int num;
    APAudioProcessor::LFOParams& lfoParams;
	gin::LFOComponent *l1;
};


//=============================================================================


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



