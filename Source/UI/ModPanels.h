#pragma once

#include <gin_plugin/gin_plugin.h>
#include "APColors.h"
#include "APModAdditions.h"
#include "DSP/PluginProcessor.h"

//==============================================================================
class LFOBox : public gin::ParamBox {
public:
	LFOBox(APAudioProcessor &proc_,
	    const APAudioProcessor::LFOParams &lfoParams_,
	    int num_)
	    : ParamBox(juce::String("  LFO ") += juce::String(num_)), proc(proc_),
	      lfoParams(lfoParams_), num(num_), monoSelect(proc, *(proc.monoLfoIds[num - 1])),
		  polySelect(proc, *(proc.polyLfoIds[num - 1]))
	{
		setName("lfo");
		addAndMakeVisible(monoSelect);
		addAndMakeVisible(polySelect);
		monoSelect.setText("+M", juce::dontSendNotification);
		polySelect.setText("+P", juce::dontSendNotification);

		gin::ModSrcId src;
		gin::ModSrcId monoSrc;
		switch (num) {
			case 1:
				src = proc.modSrcLFO1;
				monoSrc = proc.modSrcMonoLFO1;
				break;
			case 2:
				src = proc.modSrcLFO2;
				monoSrc = proc.modSrcMonoLFO2;
				break;
			case 3:
				src = proc.modSrcLFO3;
				monoSrc = proc.modSrcMonoLFO3;
				break;
			case 4:
				src = proc.modSrcLFO4;
				monoSrc = proc.modSrcMonoLFO4;
				break;
		}
		addModSource(
		    poly1 = new gin::ModulationSourceButton(proc.modMatrix, src, true));
		addModSource(mono1 = new gin::ModulationSourceButton(
		                 proc.modMatrix, monoSrc, false));
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
		l1->monoCallback = [this] {
			return proc.monoLFOs[num - 1]->getCurrentPhase();
		};
		l1->phaseCallback = [this] {
			std::vector<float> phases;
			switch (num) {
				case 1:
					phases = proc.synth.getLFO1Phases();
					break;
				case 2:
					phases = proc.synth.getLFO2Phases();
					break;
				case 3:
					phases = proc.synth.getLFO3Phases();
					break;
				case 4:
					phases = proc.synth.getLFO4Phases();
					break;
			}
			return phases;
		};
		l1->setParams(lfoParams.wave, lfoParams.sync, lfoParams.rate,
		    lfoParams.beat, lfoParams.depth, lfoParams.offset, lfoParams.phase,
		    lfoParams.enable);
		addControl(l1, 1, 0, 4, 1);

		watchParam(lfoParams.sync);
	}

	void paramChanged() override
	{
		gin::ParamBox::paramChanged();
		r1->setVisible(!lfoParams.sync->isOn());
		b1->setVisible(lfoParams.sync->isOn());
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
		monoSelect.setBounds(5 * 56 - 85, 3, 32, 16);
		polySelect.setBounds(5 * 56 - 61, 3, 32, 16);
	}

	APAudioProcessor &proc;

	gin::ParamComponent::Ptr s1, w1, r1, b1, dp1, p1, o1, f1, dl1;

	juce::Button *poly1, *mono1;

	int currentLFO{1};
	const APAudioProcessor::LFOParams &lfoParams;
	int num;
	gin::LFOComponent *l1;
    ParameterSelector monoSelect, polySelect;
	
};

//=============================================================================

class MsegBox : public gin::ParamBox {
public:
	MsegBox(APAudioProcessor &proc_,
	    APAudioProcessor::MSEGParams &m1_,
	    APAudioProcessor::MSEGParams &m2_,
	    gin::MSEG::Data &m1Data,
	    gin::MSEG::Data &m2Data,
	    int num_)
	    : gin::ParamBox(juce::String("  MSEG ")), proc(proc_),
	      msegComponent1(m1Data), msegComponent2(m2Data), m1(m1_), m2(m2_),
	      num(num_), pSelect1(proc, (num == 1) ? gin::ModSrcId(proc.modSrcMSEG1) : gin::ModSrcId(proc.modSrcMSEG3)),
		pSelect2(proc, (num == 1) ? gin::ModSrcId(proc.modSrcMSEG2) : gin::ModSrcId(proc.modSrcMSEG4))
	{
		setName("MSEG");

		if (num != 1) {
			select1.setButtonText("3");
			select2.setButtonText("4");
		}

		addChildComponent(pSelect1);
		addChildComponent(pSelect2);
		pSelect1.setText("+Dest", juce::dontSendNotification);
		pSelect2.setText("+Dest", juce::dontSendNotification);
		
		msegComponent1.setParams(m1.sync, m1.rate, m1.beat, m1.depth, m1.offset,
		    m1.phase, m1.enable, m1.xgrid, m1.ygrid, m1.loop);
		msegComponent1.setEditable(true);
		msegComponent1.setDrawMode(
		    true, static_cast<gin::MSEGComponent::DrawMode>(
		              m1.drawmode->getUserValue()));

		msegComponent2.setParams(m2.sync, m2.rate, m2.beat, m2.depth, m2.offset,
		    m2.phase, m2.enable, m2.xgrid, m2.ygrid, m2.loop);
		msegComponent2.setEditable(true);
		msegComponent2.setDrawMode(
		    true, static_cast<gin::MSEGComponent::DrawMode>(
		              m2.drawmode->getUserValue()));

		if (num == 1) {
			addModSource(poly2 = new gin::ModulationSourceButton(
			                 proc.modMatrix, proc.modSrcMSEG2, true));
			addModSource(poly1 = new gin::ModulationSourceButton(
			                 proc.modMatrix, proc.modSrcMSEG1, true));
		} else {
			addModSource(poly2 = new gin::ModulationSourceButton(
			                 proc.modMatrix, proc.modSrcMSEG4, true));
			addModSource(poly1 = new gin::ModulationSourceButton(
			                 proc.modMatrix, proc.modSrcMSEG3, true));
		}

		addControl(r1 = new APKnob(m1.rate), 0, 0);
		addControl(r2 = new APKnob(m2.rate), 0, 0);

		addControl(b1 = new gin::Select(m1.beat), 0, 0);
		addControl(b2 = new gin::Select(m2.beat), 0, 0);

		addControl(s1 = new gin::Select(m1.sync), 1, 0);
		addControl(s2 = new gin::Select(m2.sync), 1, 0);

		addControl(l1 = new gin::Select(m1.loop), 2, 0);
		addControl(l2 = new gin::Select(m2.loop), 2, 0);

		addControl(dp1 = new APKnob(m1.depth), 3, 0);
		addControl(dp2 = new APKnob(m2.depth), 3, 0);

		addControl(o1 = new APKnob(m1.offset), 4, 0);
		addControl(o2 = new APKnob(m2.offset), 4, 0);

		addControl(dr1 = new gin::Select(m1.draw), 5, 0);
		addControl(dr2 = new gin::Select(m2.draw), 5, 0);

		addControl(ms1 = new gin::Select(m1.drawmode), 6, 0);
		addControl(ms2 = new gin::Select(m2.drawmode), 6, 0);

		addControl(x1 = new gin::Select(m1.xgrid));
		addControl(x2 = new gin::Select(m2.xgrid));

		addControl(y1 = new gin::Select(m1.ygrid));
		addControl(y2 = new gin::Select(m2.ygrid));

		watchParam(m1.sync);
		watchParam(m2.sync);
		watchParam(m1.draw);
		watchParam(m2.draw);
		watchParam(m1.drawmode);
		watchParam(m2.drawmode);
		addAndMakeVisible(msegComponent1);
		addAndMakeVisible(msegComponent2);
		addAndMakeVisible(select1);
		addAndMakeVisible(select2);

		msegComponent1.phaseCallback = [this]() {
			std::vector<float> auxVals, synthVals;
			if (num == 1) {
				auxVals = proc.auxSynth.getMSEG1Phases();
				synthVals = proc.synth.getMSEG1Phases();
			} else {
				auxVals = proc.auxSynth.getMSEG3Phases();
				synthVals = proc.synth.getMSEG3Phases();
			}
			synthVals.insert(synthVals.end(), auxVals.begin(), auxVals.end());
			return synthVals;
		};
		msegComponent2.phaseCallback = [this]() {
			std::vector<float> auxVals, synthVals;
			if (num == 1) {
				auxVals = proc.auxSynth.getMSEG2Phases();
				synthVals = proc.synth.getMSEG2Phases();
			} else {
				auxVals = proc.auxSynth.getMSEG4Phases();
				synthVals = proc.synth.getMSEG4Phases();
			}
			synthVals.insert(synthVals.end(), auxVals.begin(), auxVals.end());
			return synthVals;
		};

		select1.onClick = [this]() { show(1); };
		select2.onClick = [this]() { show(2); };

		show(1);
	}

	void show(const int selected)
	{
		for (gin::ParamComponent::Ptr c : {r1, b1, s1, l1, dp1, o1, dr1, ms1,
		         x1, y1, r2, b2, s2, l2, dp2, o2, dr2, ms2, x2, y2}) {
			c->setVisible(false);
		}
		for (auto &viz : {&msegComponent1, &msegComponent2}) {
			viz->setVisible(false);
		}
		pSelect1.setVisible(false);
		pSelect2.setVisible(false);
		select1.setToggleState(false, juce::dontSendNotification);
		select2.setToggleState(false, juce::dontSendNotification);
		switch (selected) {
			case 1:
				currentMSEG = 1;
				r1->setVisible(!m1.sync->isOn());
				b1->setVisible(m1.sync->isOn());
				s1->setVisible(true);
				l1->setVisible(true);
				dp1->setVisible(true);
				o1->setVisible(true);
				dr1->setVisible(true);
				ms1->setVisible(true);
				x1->setVisible(true);
				y1->setVisible(true);
				msegComponent1.setVisible(true);
				select1.setToggleState(true, juce::dontSendNotification);
				pSelect1.setVisible(true);
				break;
			case 2:
				currentMSEG = 2;
				r2->setVisible(!m2.sync->isOn());
				b2->setVisible(m2.sync->isOn());
				s2->setVisible(true);
				l2->setVisible(true);
				dp2->setVisible(true);
				o2->setVisible(true);
				dr2->setVisible(true);
				ms2->setVisible(true);
				x2->setVisible(true);
				y2->setVisible(true);
				msegComponent2.setVisible(true);
				select2.setToggleState(true, juce::dontSendNotification);
				pSelect2.setVisible(true);
				break;
		}
		paramChanged();
	}

	void paramChanged() override
	{
		gin::ParamBox::paramChanged();
		if (currentMSEG == 1) {
			const bool sync1 = m1.sync->getUserValueBool();
			r1->setVisible(!sync1);
			b1->setVisible(sync1);
		}
		if (currentMSEG == 2) {
			const bool sync2 = m2.sync->getUserValueBool();
			r2->setVisible(!sync2);
			b2->setVisible(sync2);
		}

		if (m1.draw->getUserValueBool()) {
			msegComponent1.setDrawMode(
			    true, static_cast<gin::MSEGComponent::DrawMode>(
			              m1.drawmode->getUserValue()));
		} else {
			msegComponent1.setDrawMode(
			    false, gin::MSEGComponent::DrawMode::step);
		}

		if (m2.draw->getUserValueBool()) {
			msegComponent2.setDrawMode(
			    true, static_cast<gin::MSEGComponent::DrawMode>(
			              m2.drawmode->getUserValue()));
		} else {
			msegComponent2.setDrawMode(
			    false, gin::MSEGComponent::DrawMode::step);
		}
	}

	void resized() override
	{
		gin::ParamBox::resized();
		msegComponent1.setBounds(0, 93, getWidth(), getHeight() - 93);
		msegComponent2.setBounds(0, 93, getWidth(), getHeight() - 93);
		x1->setBounds(7 * 56, 23, 56, 35);
		x2->setBounds(7 * 56, 23, 56, 35);
		y1->setBounds(7 * 56, 58, 56, 35);
		y2->setBounds(7 * 56, 58, 56, 35);
        l1->setBounds(2 * 56, 23, 56, 70);
        l2->setBounds(2 * 56, 23, 56, 70);
		select1.setBounds(50, 0, 20, 23);
		select2.setBounds(70, 0, 20, 23);
		pSelect1.setBounds(6 * 56 + 20, 3, 45, 16);
		pSelect2.setBounds(6 * 56 + 20, 3, 45, 16);
	}

	APAudioProcessor &proc;
	gin::ParamComponent::Ptr r1, b1, s1, l1, dp1, o1, dr1, ms1, x1, y1, r2, b2,
	    s2, l2, dp2, o2, dr2, ms2, x2, y2;
	juce::Button *poly1, *poly2;

	
	gin::MSEGComponent msegComponent1, msegComponent2;
	int currentMSEG{1};
	juce::TextButton select1{"1"}, select2{"2"};
	
	APAudioProcessor::MSEGParams &m1;
	APAudioProcessor::MSEGParams &m2;
	int num;
	ParameterSelector pSelect1, pSelect2;
};
