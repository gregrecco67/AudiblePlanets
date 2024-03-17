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

 //==============================================================================
class DevEditor : public juce::Component, public gin::Parameter::ParameterListener
{
public:
	DevEditor(APAudioProcessor& proc_);

	void resized() override;
	void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override;
	void valueUpdated(gin::Parameter* param) override;

private:
	APAudioProcessor& proc;
	gin::MSEG::Data msegData;
	gin::MSEGComponent msegComponent;
	juce::Label msegLabel{"MSEG1", "MSEG1"};
	gin::Layout layout{ *this };
    gin::Knob rate{proc.mseg1Params.rate}, offset{proc.mseg1Params.offset},
        depth{proc.mseg1Params.depth};
    gin::Select beat{proc.mseg1Params.beat};
    gin::Switch sync{proc.mseg1Params.sync}, loop{proc.mseg1Params.loop};
};
