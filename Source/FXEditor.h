#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "BinaryData.h"

//==============================================================================
class FXEditor : public juce::Component, public juce::Timer
{
public:
	FXEditor(APAudioProcessor& proc_);

	void resized() override;

private:
	APAudioProcessor& proc;

	FXSelectBox fx1{ "fx1", proc, proc.fx1Params };

	//DelayBox delay { proc };

	gin::Layout layout{ *this };
};
