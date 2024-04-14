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

    juce::Font regularFont{ juce::Typeface::createSystemTypefaceFor(BinaryData::latoregular_otf, BinaryData::latoregular_otfSize) };
};
