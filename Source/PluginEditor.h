/*
 * Audible Planets - an expressive, quasi-Ptolemaic semi-modular synthesizer
 *
 * Copyright 2024, Greg Recco
 *
 * Audible Planets is released under the GNU General Public Licence v3
 * or later (GPL-3.0-or-later). The license is found in the "LICENSE"
 * file in the root of this repository, or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source for Audible Planets is available at
 * https://github.com/gregrecco67/AudiblePlanets
 */

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Panels.h"
#include "Editor.h"
#include "FXEditor.h"
#include "RandEditor.h"
#include "APColors.h"

//==============================================================================
class APAudioProcessorEditor : public gin::ProcessorEditor,
    public juce::DragAndDropContainer, public juce::KeyListener
{
public:
    APAudioProcessorEditor(APAudioProcessor&);
    ~APAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    void addMenuItems(juce::PopupMenu& m) override;
    bool keyPressed(const KeyPress& key, Component* originatingComponent) override;

private:
    APAudioProcessor& proc;

    
    gin::SynthesiserUsage usage { proc };
	gin::LevelMeter meter { proc.levelTracker };
    
    juce::TabbedComponent tabbed{juce::TabbedButtonBar::TabsAtBottom };
    juce::Component tab1, tab2, tab3;

    Editor editor { proc };
	FXEditor fxEditor{ proc };
	RandEditor randEditor{ proc };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(APAudioProcessorEditor)
};
