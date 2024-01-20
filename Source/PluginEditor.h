#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Panels.h"
#include "Editor.h"

//==============================================================================
class APAudioProcessorEditor : public gin::ProcessorEditor,
                                  public juce::DragAndDropContainer
{
public:
    APAudioProcessorEditor (APAudioProcessor&);
    ~APAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void addMenuItems (juce::PopupMenu& m) override;

private:
    APAudioProcessor& wtProc;

    gin::TriggeredScope scope { wtProc.scopeFifo };
    gin::SynthesiserUsage usage { wtProc };
    
    Editor editor { wtProc };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (APAudioProcessorEditor)
};
