#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "BinaryData.h"
#include "ModPanels.h"

//==============================================================================
class ModEditor : public juce::Component, public gin::Parameter::ParameterListener
{
public:
    ModEditor(APAudioProcessor& proc_);
    void resized() override;
    void valueUpdated(gin::Parameter* param) override;
    void removeDuplicates(int effect, int componentToSkip);

private:
    APAudioProcessor& proc;
    //MatrixBox matrix{"matrix", proc};
};

