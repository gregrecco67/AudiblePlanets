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

#include <gin_plugin/gin_plugin.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "APColors.h"
#include "APLevelMeter.h"
#include "DSP/PluginProcessor.h"
#include "Editor.h"
#include "FXEditor.h"
#include "ModEditor.h"

//==============================================================================
class APAudioProcessorEditor final : public gin::ProcessorEditor,
                                     public juce::DragAndDropContainer,
                                     public juce::KeyListener,
                                     public juce::Timer {
public:
	explicit APAudioProcessorEditor(APAudioProcessor &);
	~APAudioProcessorEditor() override;

	//==============================================================================
	void paint(juce::Graphics &) override;
	void resized() override;
	void addMenuItems(juce::PopupMenu &m) override;
	bool keyPressed(
	    const juce::KeyPress &key, Component *originatingComponent) override;
	using juce::Component::keyPressed;  // above is overloaded
	void timerCallback() override;
	void showAboutInfo() override;

private:
	APAudioProcessor &proc;

	gin::SynthesiserUsage usage{proc.synth};

	juce::TabbedComponent tabbed{juce::TabbedButtonBar::TabsAtBottom};
	juce::Component tab1, tab2, tab3;

	Editor editor{proc};
	FXEditor fxEditor{proc};
	ModEditor modEditor{proc};
	APLevelMeter levelMeter{proc.levelTracker};

	juce::Label scaleName, learningLabel;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(APAudioProcessorEditor)
};
