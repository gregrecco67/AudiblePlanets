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

#include <juce_gui_basics/juce_gui_basics.h>
#include "DSP/PluginProcessor.h"

//==============================================================================
/*
 */
class EnvelopeComponent : public juce::Component,
                          public gin::Parameter::ParameterListener,
						  public juce::Timer {
public:
	EnvelopeComponent(const APAudioProcessor &proc_, const int number)
	    : proc(proc_), envelopeNumber(number)
	{
		setOpaque(true);
		switch (envelopeNumber) {
			case 1:
				proc.env1Params.acurve->addListener(this);
				attackCurve = proc.env1Params.acurve->getUserValue();
				proc.env1Params.attack->addListener(this);
				attack = proc.env1Params.attack->getUserValue();
				proc.env1Params.decay->addListener(this);
				decay = proc.env1Params.decay->getUserValue();
				proc.env1Params.drcurve->addListener(this);
				decayCurve = proc.env1Params.drcurve->getUserValue();
				proc.env1Params.release->addListener(this);
				release = proc.env1Params.release->getUserValue();
				proc.env1Params.sustain->addListener(this);
				sustain =
				    proc.env1Params.sustain->getValue();  // getValue() -> 0..1
				break;
			case 2:
				proc.env2Params.acurve->addListener(this);
				attackCurve = proc.env2Params.acurve->getUserValue();
				proc.env2Params.attack->addListener(this);
				attack = proc.env2Params.attack->getUserValue();
				proc.env2Params.decay->addListener(this);
				decay = proc.env2Params.decay->getUserValue();
				proc.env2Params.drcurve->addListener(this);
				decayCurve = proc.env2Params.drcurve->getUserValue();
				proc.env2Params.release->addListener(this);
				release = proc.env2Params.release->getUserValue();
				proc.env2Params.sustain->addListener(this);
				sustain = proc.env2Params.sustain->getValue();
				break;
			case 3:
				proc.env3Params.acurve->addListener(this);
				attackCurve = proc.env3Params.acurve->getUserValue();
				proc.env3Params.attack->addListener(this);
				attack = proc.env3Params.attack->getUserValue();
				proc.env3Params.decay->addListener(this);
				decay = proc.env3Params.decay->getUserValue();
				proc.env3Params.drcurve->addListener(this);
				decayCurve = proc.env3Params.drcurve->getUserValue();
				proc.env3Params.release->addListener(this);
				release = proc.env3Params.release->getUserValue();
				proc.env3Params.sustain->addListener(this);
				sustain = proc.env3Params.sustain->getValue();
				break;
			case 4:
				proc.env4Params.acurve->addListener(this);
				attackCurve = proc.env4Params.acurve->getUserValue();
				proc.env4Params.attack->addListener(this);
				attack = proc.env4Params.attack->getUserValue();
				proc.env4Params.decay->addListener(this);
				decay = proc.env4Params.decay->getUserValue();
				proc.env4Params.drcurve->addListener(this);
				decayCurve = proc.env4Params.drcurve->getUserValue();
				proc.env4Params.release->addListener(this);
				release = proc.env4Params.release->getUserValue();
				proc.env4Params.sustain->addListener(this);
				sustain = proc.env4Params.sustain->getValue();
				break;
		}
		startTimerHz(30);
	}

	~EnvelopeComponent() override
	{
		switch (envelopeNumber) {
			case 1:
				proc.env1Params.acurve->removeListener(this);
				proc.env1Params.attack->removeListener(this);
				proc.env1Params.decay->removeListener(this);
				proc.env1Params.drcurve->removeListener(this);
				proc.env1Params.release->removeListener(this);
				proc.env1Params.sustain->removeListener(this);
				break;
			case 2:
				proc.env2Params.acurve->removeListener(this);
				proc.env2Params.attack->removeListener(this);
				proc.env2Params.decay->removeListener(this);
				proc.env2Params.drcurve->removeListener(this);
				proc.env2Params.release->removeListener(this);
				proc.env2Params.sustain->removeListener(this);
				break;
			case 3:
				proc.env3Params.acurve->removeListener(this);
				proc.env3Params.attack->removeListener(this);
				proc.env3Params.decay->removeListener(this);
				proc.env3Params.drcurve->removeListener(this);
				proc.env3Params.release->removeListener(this);
				proc.env3Params.sustain->removeListener(this);
				break;
			case 4:
				proc.env4Params.acurve->removeListener(this);
				proc.env4Params.attack->removeListener(this);
				proc.env4Params.decay->removeListener(this);
				proc.env4Params.drcurve->removeListener(this);
				proc.env4Params.release->removeListener(this);
				proc.env4Params.sustain->removeListener(this);
				break;
		}
	}

	float attack{1.f}, decay{1.f}, sustain{0.5f}, release{1.f},
	    attackCurve{1.0f}, decayCurve{-1.0f};

	inline void setAttack(float attackValue)
	{
		attack = attackValue;
		repaint();
	}

	inline void setDecay(float decayValue)
	{
		decay = decayValue;
		repaint();
	}

	inline void setSustain(float sustainValue)
	{
		sustain = sustainValue;
		repaint();
	}

	inline void setRelease(float releaseValue)
	{
		release = releaseValue;
		repaint();
	}

	inline void setAttackCurve(float attackCurveValue)
	{
		attackCurve = attackCurveValue;
		repaint();
	}

	inline void setDecayCurve(float decayCurveValue)
	{
		decayCurve = decayCurveValue;
		repaint();
	}

	void valueUpdated(gin::Parameter *p) override
	{
		if (p == proc.env1Params.attack || p == proc.env2Params.attack ||
		    p == proc.env3Params.attack || p == proc.env4Params.attack)
			setAttack(p->getUserValue());
		else if (p == proc.env1Params.decay || p == proc.env2Params.decay ||
		         p == proc.env3Params.decay || p == proc.env4Params.decay)
			setDecay(p->getUserValue());
		else if (p == proc.env1Params.sustain || p == proc.env2Params.sustain ||
		         p == proc.env3Params.sustain || p == proc.env4Params.sustain)
			setSustain(p->getValue());  // getValue() -> 0..1
		else if (p == proc.env1Params.release || p == proc.env2Params.release ||
		         p == proc.env3Params.release || p == proc.env4Params.release)
			setRelease(p->getUserValue());
		else if (p == proc.env1Params.acurve || p == proc.env2Params.acurve ||
		         p == proc.env3Params.acurve || p == proc.env4Params.acurve)
			setAttackCurve(p->getUserValue());
		else if (p == proc.env1Params.drcurve || p == proc.env2Params.drcurve ||
		         p == proc.env3Params.drcurve || p == proc.env4Params.drcurve)
			setDecayCurve(p->getUserValue());
	}

	void paint(juce::Graphics &g) override
	{
		auto bounds = getLocalBounds();
		auto width =  static_cast<float>(bounds.getWidth());
		auto height = static_cast<float>(bounds.getHeight() - 5);
		juce::Path myPath;
		juce::NormalisableRange<float> attackRange{0.0, 60.0, 0.0, 0.2f};
		auto attackLength = attackRange.convertTo0to1(attack) * width / 4.f;

		float attackFirstControlX{0.f}, attackFirstControlY{0.f},
		    attackSecondControlX{0.f}, attackSecondControlY;

		float fudgeUIFactor = 0.75f;

		if (attackCurve > 0.f) {
			attackFirstControlX =
			    0.25f * attackLength -
			    attackCurve * fudgeUIFactor * 0.25f * attackLength;
			attackFirstControlY =
			    0.25f * height + attackCurve * fudgeUIFactor * 0.75f * height;
			attackSecondControlX =
			    0.75f * attackLength -
			    attackCurve * fudgeUIFactor * 0.75f * attackLength;
			attackSecondControlY =
			    0.75f * height + attackCurve * fudgeUIFactor * 0.25f * height;
		} else {
			attackFirstControlX =
			    0.25f * attackLength -
			    fudgeUIFactor * attackCurve * 0.75f * attackLength;
			attackFirstControlY =
			    0.25f * height + attackCurve * fudgeUIFactor * 0.25f * height;
			attackSecondControlX =
			    0.75f * attackLength -
			    attackCurve * fudgeUIFactor * 0.25f * attackLength;
			attackSecondControlY =
			    0.75f * height + attackCurve * fudgeUIFactor * 0.75f * height;
		}

		auto decayLength = attackRange.convertTo0to1(decay) * width / 4.f;
		float decayFirstControlX{0.f}, decayFirstControlY{0.f},
		    decaySecondControlX{0.f}, decaySecondControlY;
		float decayEndX = attackLength + decayLength;
		float decayEndY = sustain * height;

		if (decayCurve < 0.f) {
			decayFirstControlX =
			    attackLength + 0.25f * decayLength +
			    decayCurve * fudgeUIFactor * 0.25f * decayLength;
			decayFirstControlY = height - 0.25f * (height - sustain * height) +
			                     decayCurve * fudgeUIFactor * 0.75f *
			                         (height - sustain * height);
			decaySecondControlX =
			    attackLength + 0.75f * decayLength +
			    decayCurve * fudgeUIFactor * 0.75f * decayLength;
			decaySecondControlY = height - 0.75f * (height - sustain * height) +
			                      decayCurve * fudgeUIFactor * 0.25f *
			                          (height - sustain * height);

		} else {
			decayFirstControlX =
			    attackLength + 0.25f * decayLength +
			    fudgeUIFactor * decayCurve * 0.75f * decayLength;
			decayFirstControlY = height - 0.25f * (height - sustain * height) +
			                     decayCurve * fudgeUIFactor * 0.25f *
			                         (height - sustain * height);
			decaySecondControlX =
			    attackLength + 0.75f * decayLength +
			    decayCurve * fudgeUIFactor * 0.25f * decayLength;
			decaySecondControlY = height - 0.75f * (height - sustain * height) +
			                      decayCurve * fudgeUIFactor * 0.75f *
			                          (height - sustain * height);
		}

		g.fillAll(juce::Colour(33, 31, 33));  // clear the background
		g.setColour(juce::Colours::grey);     // outline color
		g.drawRect(getLocalBounds(), 1);  // draw an outline around the component
		juce::Colour c;
		g.setColour(c = findColour(gin::GinLookAndFeel::accentColourId));  // envelope color

		myPath.startNewSubPath(0.f, 1.f);  // attack segment
		myPath.cubicTo(attackFirstControlX, attackFirstControlY,
		    attackSecondControlX, attackSecondControlY, attackLength, height);

		// check for attack phase, 
		// getPointAlongPath()
		// draw that point! rinse and repeat!
		auto pathLength1 = myPath.getLength();
		g.setColour(juce::Colours::white);
		for (auto &state : states) {
			if (state.state == Envelope::State::attack ||
			    state.state == Envelope::State::ADRattack) {
				auto pointAlongPath = myPath.getPointAlongPath(pathLength1 * state.phase);
				g.fillEllipse(juce::Rectangle<float>(4.f, 4.f).withCentre(
				    juce::Point<float>(pointAlongPath.x, height - pointAlongPath.y + 5)));
			}
		}
		g.setColour(c);

		myPath.startNewSubPath(attackLength, height);  // decay segment
		myPath.cubicTo(decayFirstControlX, decayFirstControlY,
		    decaySecondControlX, decaySecondControlY, decayEndX, decayEndY);

			auto pathLength2 = myPath.getLength() - pathLength1;
		g.setColour(juce::Colours::white);
		for (auto &state : states) {
			if (state.state == Envelope::State::decay ||
			    state.state == Envelope::State::ADRdecay) {
				auto pointAlongPath = myPath.getPointAlongPath(
				    pathLength2 * 
					(1.f - state.phase)
					+ pathLength1);
				g.fillEllipse(juce::Rectangle<float>(4.f, 4.f).withCentre(
				    juce::Point<float>(pointAlongPath.x, height - pointAlongPath.y + 5)));
			}
		}
		g.setColour(c);

		myPath.startNewSubPath(decayEndX, decayEndY);  // sustain segment
		myPath.lineTo(decayEndX + width / 4.f, decayEndY);



		myPath.startNewSubPath(
		    decayEndX + width / 4.f, decayEndY);  // release segment

		float sustainEndX = decayEndX + width / 4.f;
		float sustainHeight = sustain * height;

		float releaseLength = attackRange.convertTo0to1(release) * width / 4.f;
		float releaseFirstControlX{0.f}, releaseFirstControlY{0.f},
		    releaseSecondControlX{0.f}, releaseSecondControlY{0.f};
		float releaseEndX = decayEndX + .25f * width + releaseLength;
		float releaseEndY = 1.f; // ......
		if (decayCurve < 0.f) {
			releaseFirstControlX =
			    sustainEndX + 0.25f * releaseLength +
			    decayCurve * fudgeUIFactor * 0.25f * releaseLength;
			releaseFirstControlY =
			    0.75f * sustainHeight +
			    decayCurve * fudgeUIFactor * 0.75f * sustainHeight;
			releaseSecondControlX =
			    sustainEndX + 0.75f * releaseLength +
			    decayCurve * fudgeUIFactor * 0.75f * releaseLength;
			releaseSecondControlY =
			    0.25f * sustainHeight +
			    decayCurve * fudgeUIFactor * 0.25f * sustainHeight;
		} else {
			releaseFirstControlX =
			    sustainEndX + 0.25f * releaseLength +
			    fudgeUIFactor * decayCurve * 0.75f * releaseLength;
			releaseFirstControlY =
			    0.75f * sustainHeight +
			    decayCurve * fudgeUIFactor * 0.25f * sustainHeight;
			releaseSecondControlX =
			    sustainEndX + 0.75f * releaseLength +
			    decayCurve * fudgeUIFactor * 0.25f * releaseLength;
			releaseSecondControlY =
			    0.25f * sustainHeight +
			    decayCurve * fudgeUIFactor * 0.75f * sustainHeight;
		}

		myPath.cubicTo(releaseFirstControlX, releaseFirstControlY,
		    releaseSecondControlX, releaseSecondControlY, releaseEndX,
		    releaseEndY);

		auto pathLength3 = myPath.getLength() - pathLength1 - pathLength2 - width / 4.f;
		g.setColour(juce::Colours::white);
		for (auto &state : states) {
			if (state.state == Envelope::State::release ||
			    state.state == Envelope::State::ADRrelease) {
				auto pointAlongPath = myPath.getPointAlongPath(
				    pathLength3 * (1 - state.phase) + pathLength1 + pathLength2
					+ width / 4.f);
				g.fillEllipse(juce::Rectangle<float>(4.f, 4.f).withCentre(
				    juce::Point<float>(pointAlongPath.x, height - pointAlongPath.y + 5)));
			}
		}
		g.setColour(c);

		myPath.applyTransform(juce::AffineTransform::verticalFlip(height)
		        .juce::AffineTransform::translated(0.0f, 5.0f));
		g.strokePath(myPath, juce::PathStrokeType(1.0f));

		g.fillEllipse(juce::Rectangle<float>(6.f, 6.f).withCentre(
		    juce::Point<float>(attackLength, 4.0)));
		g.fillEllipse(juce::Rectangle<float>(6.f, 6.f).withCentre(
		    juce::Point<float>(decayEndX, height - decayEndY + 5.f)));
		g.fillEllipse(
		    juce::Rectangle<float>(6.f, 6.f).withCentre(juce::Point<float>(
		        decayEndX + width / 4.f, height - decayEndY + 5.f)));

		g.setColour(juce::Colours::white);
		for (auto &state : states) {
			if (state.state == Envelope::State::sustain) {
				g.fillEllipse(juce::Rectangle<float>(4.f, 4.f).withCentre(
				    juce::Point<float>(decayEndX + width / 4.f, height - decayEndY + 5.f)));
				g.fillEllipse(juce::Rectangle<float>(4.f, 4.f).withCentre(
				    juce::Point<float>(decayEndX, height - decayEndY + 5.f)));
			}
		}
		
	}

	void timerCallback() override
	{
		if (phaseCallback) {
			states = phaseCallback();
		}
		repaint();
	}

	std::function<std::vector<Envelope::EnvelopeState>()> phaseCallback;
	std::vector<Envelope::EnvelopeState> states;
private:
	const APAudioProcessor &proc;
	int envelopeNumber;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeComponent)
};
