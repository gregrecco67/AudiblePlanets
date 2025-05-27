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
#include "BinaryData.h"

#include <numbers>

using std::numbers::pi;

class APLNF : public gin::CopperLookAndFeel {
public:
	APLNF();
	juce::Font getLabelFont(juce::Label &label) override;
	void drawRotarySlider(juce::Graphics &g,
	    int x,
	    int y,
	    int width,
	    int height,
	    float sliderPos,
	    const float rotaryStartAngleIn,
	    const float rotaryEndAngle,
	    juce::Slider &slider) override;
	void drawLinearSlider(juce::Graphics &g,
	    int x,
	    int y,
	    int width,
	    int height,
	    float sliderPos,
	    float minSliderPos,
	    float maxSliderPos,
	    juce::Slider::SliderStyle style,
	    juce::Slider &slider) override;

	juce::Font getPopupMenuFont() override;

	juce::Font getSliderPopupFont(juce::Slider &slider) override;
};
