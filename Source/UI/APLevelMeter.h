/*
==============================================================================

This file is part of the GIN library.
Copyright (c) 2019 - Roland Rabien.

==============================================================================
*/
#pragma once

#include "juce_gui_basics/juce_gui_basics.h"
#include "gin_plugin/gin_plugin.h"

#pragma once

/** Draws a simple level meter
 */
class APLevelMeter final : public juce::Component, juce::Timer {
public:
	explicit APLevelMeter(
	    const gin::LevelTracker &, juce::NormalisableRange<float> r = {-60, 0},
	    bool vertical = false);
	~APLevelMeter() override;

	enum ColourIds {
		lineColourId = 0x1291e10,
		backgroundColourId = 0x1291e11,
		meterColourId = 0x1291e12,
		clipColourId = 0x1291e13
	};
	void paint(juce::Graphics &g) override;

private:
	//==============================================================================
	void timerCallback() override;

	bool vertical = false;
	const gin::LevelTracker &tracker;
	juce::NormalisableRange<float> range{-60, 6};

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(APLevelMeter)
};
