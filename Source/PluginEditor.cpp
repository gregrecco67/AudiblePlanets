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

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
APAudioProcessorEditor::APAudioProcessorEditor(APAudioProcessor& p)
    : ProcessorEditor(p), proc(p)
{
	addAndMakeVisible(meter);
    addAndMakeVisible(tabbed);
    tabbed.addTab("1. Main", APColors::tabBkgd, &tab1, false, 0);
    tabbed.addTab("2. Effects", APColors::tabBkgd, &tab2, false, 1);
    tabbed.addTab("3. Randomization",   APColors::tabBkgd, &tab3, false, 2);
	tabbed.addTab("4. MSEG", APColors::tabBkgd, &tab4, false, 3);
	tabbed.addTab("5. Macros/Aux", APColors::tabBkgd, &tab5, false, 4);

    tabbed.setLookAndFeel(&tabLNF);
    
    tab1.addAndMakeVisible(editor);
	tab2.addAndMakeVisible(fxEditor);
	tab3.addAndMakeVisible(randEditor);
	tab4.addAndMakeVisible(msegEditor);
	tab5.addAndMakeVisible(macrosEditor);
    
    usage.panic.onClick = [this] { proc.presetLoaded = true; };
    addAndMakeVisible(usage);
	addAndMakeVisible(scaleName);
	addAndMakeVisible(learningLabel);
	addAndMakeVisible(auxKnob);
	auxKnob.setDisplayName("Aux");
	addAndMakeVisible(samplerKnob);
	samplerKnob.setDisplayName("Sampler");
	addAndMakeVisible(levelKnob);



	scaleName.setFont(juce::Font(12.0f, juce::Font::plain));
	scaleName.setColour(juce::Label::textColourId, juce::Colour(0xffE6E6E9));
	scaleName.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
	scaleName.setJustificationType(juce::Justification::centred);
	learningLabel.setJustificationType(juce::Justification::centred);
    setSize(1186,725);
    startTimerHz(3);
    addKeyListener(this);
    this->setWantsKeyboardFocus(true);
}

bool APAudioProcessorEditor::keyPressed(const KeyPress& key, Component* /*originatingComponent*/) {
        if (key.isKeyCode(49) || key.isKeyCode(juce::KeyPress::numberPad1)) { // 1-5 for tab select
            tabbed.setCurrentTabIndex(0);
            return true;
        }
        if (key.isKeyCode(50) || key.isKeyCode(juce::KeyPress::numberPad2)) {
            tabbed.setCurrentTabIndex(1);
            return true;
        }
        if (key.isKeyCode(51) || key.isKeyCode(juce::KeyPress::numberPad3)) {
            tabbed.setCurrentTabIndex(2);
            return true;
        }
		if (key.isKeyCode(52) || key.isKeyCode(juce::KeyPress::numberPad4)) {
			tabbed.setCurrentTabIndex(3);
			return true;
		}
		if (key.isKeyCode(53) || key.isKeyCode(juce::KeyPress::numberPad5)) {
			tabbed.setCurrentTabIndex(4);
			return true;
		}
		if (key.isKeyCode(juce::KeyPress::escapeKey) || key.isKeyCode(76)) { // "L" for learning
			proc.modMatrix.disableLearn();
			return !key.isKeyCode(76); // let the "L" through, since it's often a note
		}
        return false;
}

void APAudioProcessorEditor::timerCallback()
{
	if (MTS_HasMaster(proc.client))
	{
		scaleName.setText(proc.scaleName, juce::dontSendNotification);
		scaleName.setColour(juce::Label::backgroundColourId, juce::Colour(0xff16171A).brighter(0.3f));
	}
	else
	{
		scaleName.setText("", juce::dontSendNotification);
		scaleName.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
	}
	learningLabel.setColour(juce::Label::backgroundColourId, (!proc.learningLabel.isEmpty()) ? juce::Colour(0xff16171A).brighter(0.3f) : juce::Colours::transparentBlack);
	learningLabel.setText(proc.learningLabel, juce::dontSendNotification);
}

APAudioProcessorEditor::~APAudioProcessorEditor()
{
	tabbed.setLookAndFeel(nullptr);
    stopTimer();
}

//==============================================================================
void APAudioProcessorEditor::paint(juce::Graphics& g)
{
    ProcessorEditor::paint(g);

    titleBar.setShowBrowser(true);

    g.fillAll(findColour(gin::PluginLookAndFeel::blackColourId));
}

void APAudioProcessorEditor::resized()
{
    ProcessorEditor::resized();

    auto rc = getLocalBounds().reduced(1); 
    rc.removeFromTop(40);
    tabbed.setBounds(rc);
    auto editorArea = tabbed.getLocalBounds();
    editorArea.removeFromBottom(tabbed.getTabBarDepth());
    editor.setBounds(editorArea);
    patchBrowser.setBounds(rc);
	//	meter.setBounds(1130, 5, 15, 30);
	auxKnob.setBounds(1031, 0, 32, 40);
	samplerKnob.setBounds(1063, 0, 32, 40);
	levelKnob.setBounds(1095, 0, 32, 40);

	meter.setBounds(1130, 5, 15, 30);
	usage.setBounds(45, 12, 80, 16);
	scaleName.setBounds(165, 12, 200, 16);
	learningLabel.setBounds(834, 12, 184, 16);

	fxEditor.setBounds(editorArea);
	randEditor.setBounds(editorArea);
	msegEditor.setBounds(editorArea);
	macrosEditor.setBounds(editorArea);
}

void APAudioProcessorEditor::addMenuItems(juce::PopupMenu& m)
{
    m.addSeparator();
    m.addItem("MPE", true, proc.globalParams.mpe->getUserValueBool(), [this]
    {
        proc.globalParams.mpe->setUserValue(proc.globalParams.mpe->getUserValueBool() ? 0.0f : 1.0f);
    });

    auto setSize = [this](float scale)
    {
        if (auto p = findParentComponentOfClass<gin::ScaledPluginEditor>())
            p->setScale(scale);
    };

    juce::PopupMenu um;
    um.addItem("50%",  [setSize] { setSize(0.50f); });
    um.addItem("75%",  [setSize] { setSize(0.75f); });
    um.addItem("100%", [setSize] { setSize(1.00f); });
    um.addItem("150%", [setSize] { setSize(1.50f); });
    um.addItem("200%", [setSize] { setSize(2.00f); });

    m.addSubMenu("UI Size", um);
}
