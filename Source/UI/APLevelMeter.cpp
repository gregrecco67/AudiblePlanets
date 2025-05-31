#include "APLevelMeter.h"

APLevelMeter::APLevelMeter(
    const gin::LevelTracker &l, juce::NormalisableRange<float> r, bool vertical_)
    : vertical(vertical_), tracker(l), range(r)
{
    startTimerHz(30);
	setColour(lineColourId, juce::Colours::white);
	setColour(meterColourId, juce::Colours::white.withAlpha(0.5f));
	setColour(backgroundColourId, juce::Colours::white.withAlpha(0.1f));
	setColour(clipColourId, juce::Colours::red);
}

APLevelMeter::~APLevelMeter() { stopTimer(); }

//==============================================================================

void APLevelMeter::paint(juce::Graphics &g)
{
	g.setColour(findColour(lineColourId));
	g.drawRect(getLocalBounds());
	const auto level = juce::jlimit(range.start, range.end, tracker.getLevel());
	auto rc = getLocalBounds().toFloat();
	g.setColour(findColour(meterColourId));
	if (vertical)
	{	g.fillRect(rc.removeFromBottom(range.convertTo0to1(level) * rc.getHeight())); }
	else 
	{	g.fillRect(rc.removeFromLeft(range.convertTo0to1(level) * rc.getWidth())); }
	g.setColour(findColour(backgroundColourId));
	g.fillRect(rc);
	if (tracker.getClip()) {
		g.setColour(findColour(clipColourId));
		g.fillRect(
		    getLocalBounds().toFloat().withTrimmedLeft(rc.getWidth() * 0.95f));
	}
}

void APLevelMeter::timerCallback() { repaint(); }
