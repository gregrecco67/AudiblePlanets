#include "Editor.h"

Editor::Editor (APAudioProcessor& proc_)
    : proc ( proc_ )
{
    addAndMakeVisible(osc1);
	addAndMakeVisible(lfo1);
	addAndMakeVisible(env1);
	addAndMakeVisible(filter);
	addAndMakeVisible(mod);
	addAndMakeVisible(mtx);
	addAndMakeVisible(global);

	setupCallbacks();
}

void Editor::setupCallbacks()
{
}

void Editor::resized()
{
#if JUCE_DEBUG
    auto f = juce::File (__FILE__).getChildFile ("../../Resources/layout.json");

    layout.setLayout ("layout.json", f);
#else
    layout.setLayout ("layout.json");
#endif
}
