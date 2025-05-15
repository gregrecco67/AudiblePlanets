/*
==============================================================================

This file is part of the GIN library.
Copyright (c) 2019 - Roland Rabien.

==============================================================================
*/

#include "gin_plugin/gin_plugin.h"
#include "juce_gui_basics/juce_gui_basics.h"

#pragma once

/** Draws a simple level meter
 */
class APLevelMeter : public juce::Component, public juce::Timer {
public:
	APLevelMeter(
	    const gin::LevelTracker &, juce::NormalisableRange<float> r = {-60, 0});
	~APLevelMeter() override;

	enum ColourIds {
		lineColourId = 0x1291e10,
		backgroundColourId = 0x1291e11,
		meterColourId = 0x1291e12,
		clipColourId = 0x1291e13
	};

private:
	//==============================================================================
	void paint(juce::Graphics &g) override;
	void timerCallback() override;

	const gin::LevelTracker &tracker;
	juce::NormalisableRange<float> range{-60, 6};

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(APLevelMeter)
};
