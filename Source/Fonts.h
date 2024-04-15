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

#include "BinaryData.h"
#include "JuceHeader.h"

class APLNF : public gin::CopperLookAndFeel
{
public:
	juce::Typeface::Ptr getTypefaceForFont(const juce::Font& font) override
	{
		return juce::Typeface::createSystemTypefaceFor(BinaryData::latoregular_otf, BinaryData::latoregular_otfSize);
	}
	
	juce::Font getLabelFont(juce::Label& label) override
	{
        return regularFont.withHeight(label.getHeight() * 0.8f);
        //return regularFont.withHeight(11);
	}

	juce::Font getTextButtonFont(juce::TextButton& button, int height) override
	{
		return regularFont.withHeight(12);
	}
    
	juce::Font getSliderPopupFont(Slider&) override
	{
		return regularFont.withHeight(12);
	}

	juce::Font getComboBoxFont(ComboBox&) override
	{
		return regularFont.withHeight(12);
	}

	juce::Font getPopupMenuFont() override
	{
		return regularFont.withHeight(12);
	}

    juce::Font regularFont{ juce::Typeface::createSystemTypefaceFor(BinaryData::latoregular_otf, BinaryData::latoregular_otfSize) };
};
