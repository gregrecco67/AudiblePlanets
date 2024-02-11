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
#include <juce_gui_basics/juce_gui_basics.h>
#include "BinaryData.h"

class MoonSlider : public gin::PluginSlider
	//public juce::DragAndDropTarget,
	//private juce::Timer,
	//private gin::ModMatrix::Listener
{
public:
	MoonSlider(gin::Parameter* parameter_);

	juce::String getUid() { return parameter->getUid(); }

	void setTooltip(const juce::String& newTooltip) override
	{
		juce::SettableTooltipClient::setTooltip(newTooltip);

		for (auto c : getChildren())
			if (auto stc = dynamic_cast<juce::SettableTooltipClient*>(c))
				stc->setTooltip(newTooltip);
	}

protected:
	gin::Parameter* parameter = nullptr;

private:
	void paint(juce::Graphics& g) override
	{
		if (filmStrip.isValid()) {
			int sliderVal = std::clamp((int)((getValue() - getMinimum()) / (getMaximum() - getMinimum()) * (numFrames_ - 1)), 0, 125);
			g.drawImage(filmStrip, 0, 0, getWidth(), getHeight(), sliderVal * frameWidth, 0, 128, 128);
		}
	}
	juce::Image filmStrip;
	const int numFrames_{ 127 };

	int frameWidth{ 128 };

	juce::Label name;
	gin::Readout value;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MoonSlider)
};
