#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "RandPanels.h"
#include <random>


//==============================================================================
class RandEditor : public juce::Component
{
public:
	RandEditor(APAudioProcessor& proc_);
	void resized() override;
	void randomize();

private:
	APAudioProcessor& proc;
	RandMatrixBox matrix{ "matrix", proc };
	juce::TextButton randomizeButton{ "Randomize" };
	std::random_device rd;
	juce::Label hello{"hello", "hello"};
    juce::Slider randNumber, randAmount;
};
