#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Panels.h"
#include "Editor.h"
#include "APColors.h"

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

    
    gin::SynthesiserUsage usage { wtProc };
	gin::LevelMeter meter { wtProc.levelTracker };
    
    juce::TabbedComponent tabbed{juce::TabbedButtonBar::TabsAtBottom };
    juce::Component tab1, tab2, tab3;

    Editor editor { wtProc };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (APAudioProcessorEditor)
};
