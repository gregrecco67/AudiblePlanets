#pragma once
#include <juce_graphics/juce_graphics.h>
#include <JuceHeader.h>

namespace APColors
{
	const juce::Colour blue = juce::Colour(0x4D, 0x9D, 0xE0);
	const juce::Colour red = juce::Colour(0xE1, 0x55, 0x54);
	const juce::Colour yellow = juce::Colour(0xE1, 0xBC, 0x29);
	const juce::Colour green = juce::Colour(0x3B, 0xB2, 0x73);
	const juce::Colour purple = juce::Colour(0x77, 0x68, 0xAE);
	const juce::Colour blueMuted = blue.darker(0.5f);
	const juce::Colour redMuted = red.darker(0.5f);
	const juce::Colour yellowMuted = yellow.darker(0.5f);
	const juce::Colour greenMuted = green.darker(0.5f);
	const juce::Colour purpleMuted = purple.darker(0.5f);
	const juce::Colour blueDark = blue.darker(1.45f);
	const juce::Colour redDark = red.darker(1.45f);
	const juce::Colour yellowDark = yellow.darker(1.45f);
	const juce::Colour greenDark = green.darker(1.45f);
	const juce::Colour purpleDark = purple.darker(1.45f);
	const juce::Colour blueLight = blue.brighter(1.45f);
	const juce::Colour redLight = red.brighter(1.45f);
	const juce::Colour yellowLight = yellow.brighter(1.45f);
	const juce::Colour greenLight = green.brighter(1.45f);
	const juce::Colour purpleLight = purple.brighter(1.45f);

	const juce::Colour tabBkgd = juce::Colour(31, 30, 31).brighter(0.05f);
}

