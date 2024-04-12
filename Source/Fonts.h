#pragma once

#include "BinaryData.h"
#include "JuceHeader.h"

//static const juce::Font regularFont{ juce::Typeface::createSystemTypefaceFor(BinaryData::latoregular_otf, BinaryData::latoregular_otfSize) };
//static const juce::Font boldFont{ juce::Typeface::createSystemTypefaceFor(BinaryData::latoblack_otf, BinaryData::latoblack_otfSize) };
//static const juce::Font lightFont{ juce::Typeface::createSystemTypefaceFor(BinaryData::latolight_otf, BinaryData::latolight_otfSize) };

class APLNF : public gin::CopperLookAndFeel
{
	juce::Typeface::Ptr getTypefaceForFont(const juce::Font& font) override
	{
		return juce::Typeface::createSystemTypefaceFor(BinaryData::latoregular_otf, BinaryData::latoregular_otfSize);
	}
	
	juce::Font getLabelFont(juce::Label& label) override
	{
		return regularFont.withHeight(11);
	}

	juce::Font getTextButtonFont(juce::TextButton& button, int height) override
	{
		return regularFont.withHeight(12);
	}
    
    juce::Font regularFont{ juce::Typeface::createSystemTypefaceFor(BinaryData::latoregular_otf, BinaryData::latoregular_otfSize) };
};
