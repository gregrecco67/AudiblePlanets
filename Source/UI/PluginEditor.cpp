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

#include "PluginEditor.h"
#include "PluginProcessor.h"
#ifndef VERSION_STRING
#define VERSION_STRING "???"
#endif

//==============================================================================
APAudioProcessorEditor::APAudioProcessorEditor(APAudioProcessor &p)
    : ProcessorEditor(p), proc(p)
{
	setName("Editor");
	// tabbed.setName("tabbed");
	addAndMakeVisible(tabbed);
	tabbed.addTab("1. Main", APColors::tabBkgd, &tab1, false, 0);
	tabbed.addTab("2. Mods", APColors::tabBkgd, &tab2, false, 1);
	tabbed.addTab("3. Effects", APColors::tabBkgd, &tab3, false, 2);

	tab1.addAndMakeVisible(editor);
	tab2.addAndMakeVisible(modEditor);
	tab3.addAndMakeVisible(fxEditor);

	usage.panic.onClick = [this] { proc.presetLoaded = true; };
	addAndMakeVisible(usage);
	addAndMakeVisible(scaleName);
	addAndMakeVisible(learningLabel);
	addChildComponent(levelMeter);

	scaleName.setFont(juce::FontOptions(12.0f));
	scaleName.setColour(juce::Label::textColourId, juce::Colour(0xffE6E6E9));
	scaleName.setColour(
	    juce::Label::backgroundColourId, juce::Colours::transparentBlack);
	scaleName.setJustificationType(juce::Justification::centredLeft);
	learningLabel.setJustificationType(juce::Justification::centred);
	setSize(1181, 725);
	startTimerHz(4);
	addKeyListener(this);
	this->setWantsKeyboardFocus(true);
	titleBar.setShowBrowser(true);
}

bool APAudioProcessorEditor::keyPressed(
    const juce::KeyPress &key, Component * /*originatingComponent*/)
{
	if (key.isKeyCode(49) ||
	    key.isKeyCode(juce::KeyPress::numberPad1)) {  // 1-5 for tab select
		levelMeter.setVisible(false);
		tabbed.setCurrentTabIndex(0);
		return true;
	}
	if (key.isKeyCode(50) || key.isKeyCode(juce::KeyPress::numberPad2)) {
		levelMeter.setVisible(true);
		tabbed.setCurrentTabIndex(1);
		return true;
	}
	if (key.isKeyCode(51) || key.isKeyCode(juce::KeyPress::numberPad3)) {
		levelMeter.setVisible(true);
		tabbed.setCurrentTabIndex(2);
		return true;
	}
	if (key.isKeyCode(juce::KeyPress::escapeKey) || key.isKeyCode(76)) {  // "L" for learning
		proc.modMatrix.disableLearn();
		return !key.isKeyCode(76);  // let the "L" through, since it's often a note
	}
	return false;
}

void APAudioProcessorEditor::timerCallback()
{
	if (!isVisible())
		return;
	if (MTS_HasMaster(proc.client)) {
		scaleName.setText(proc.scaleName, juce::dontSendNotification);
		scaleName.setColour(juce::Label::backgroundColourId,
		    juce::Colour(0xff16171A).brighter(0.3f));
	} else {
		scaleName.setText("", juce::dontSendNotification);
		scaleName.setColour(
		    juce::Label::backgroundColourId, juce::Colours::transparentBlack);
	}
	if (proc.learning.isEmpty()) {
		learningLabel.setText("", juce::dontSendNotification);
		learningLabel.setColour(
		    juce::Label::backgroundColourId, juce::Colours::transparentBlack);
	} else {
		learningLabel.setText(proc.learning, juce::dontSendNotification);
		learningLabel.setColour(juce::Label::backgroundColourId,
		    juce::Colour(0xff16171A).brighter(0.3f));
	}
	if (tabbed.getCurrentTabIndex() == 0) {
		levelMeter.setVisible(false);
	} else {
		levelMeter.setVisible(true);
	}
}

void APAudioProcessorEditor::showAboutInfo()
{
	juce::String msg;

	msg += "Audible Planets v";
	msg.append(VERSION_STRING, 25);
	msg += " (" __DATE__ ")\n\n";

	msg += "Greg Recco\n\n";

	msg += "Copyright ";
	msg += juce::String(&__DATE__[7]);

	auto w = std::make_shared<gin::PluginAlertWindow>(
	    "---- Info ----", msg, juce::AlertWindow::NoIcon, this);
	w->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
	w->setLookAndFeel(slProc.lf.get());

	w->runAsync(*this, [w](int) { w->setVisible(false); });
}

APAudioProcessorEditor::~APAudioProcessorEditor()
{
	setLookAndFeel(nullptr);
	stopTimer();
}

//==============================================================================
void APAudioProcessorEditor::paint(juce::Graphics &g)
{
	ProcessorEditor::paint(g);
}

void APAudioProcessorEditor::resized()
{
	auto rc = getLocalBounds().reduced(1);
	if (rc.getWidth() > 1186 || rc.getHeight() > 725) {
		return;
	}
	ProcessorEditor::resized();
	rc.removeFromTop(40);
	tabbed.setBounds(rc);
	auto editorArea = tabbed.getLocalBounds();
	editorArea.removeFromBottom(tabbed.getTabBarDepth());
	editor.setBounds(editorArea);
	patchBrowser.setBounds(rc);
	usage.setBounds(45, 12, 95, 20);
	scaleName.setBounds(165, 12, 200, 16);
	learningLabel.setBounds(834, 12, 184, 16); 
	fxEditor.setBounds(editorArea);
	modEditor.setBounds(editorArea);
	levelMeter.setBounds(1050, 12, 90, 22);
}

void APAudioProcessorEditor::addMenuItems(juce::PopupMenu &m)
{
	m.addSeparator();
	m.addItem("MPE", true, proc.globalParams.mpe->getUserValueBool(), [this] {
		proc.globalParams.mpe->setUserValue(
		    proc.globalParams.mpe->getUserValueBool() ? 0.0f : 1.0f);
	});

	auto setSize = [this](const float scale) {
		if (auto p = findParentComponentOfClass<gin::ScaledPluginEditor>())
			p->setScale(scale);
	};

	juce::PopupMenu um;
	um.addItem("50%", [setSize] { setSize(0.50f); });
	um.addItem("75%", [setSize] { setSize(0.75f); });
	um.addItem("100%", [setSize] { setSize(1.00f); });
	um.addItem("125%", [setSize] { setSize(1.25f); });
	um.addItem("150%", [setSize] { setSize(1.50f); });
	um.addItem("175%", [setSize] { setSize(1.75f); });
	um.addItem("200%", [setSize] { setSize(2.00f); });

	m.addSubMenu("UI Size", um);
}
