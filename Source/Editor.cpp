#include "Editor.h"

Editor::Editor (APAudioProcessor& proc_)
    : proc ( proc_ )
{
    addAndMakeVisible(osc1);
	addAndMakeVisible(osc2);
	addAndMakeVisible(osc3);
	addAndMakeVisible(osc4);
	addAndMakeVisible(lfo1);
	addAndMakeVisible(lfo2);
	addAndMakeVisible(lfo3);
	addAndMakeVisible(lfo4);
	addAndMakeVisible(env1);
	addAndMakeVisible(env2);
	addAndMakeVisible(env3);
	addAndMakeVisible(env4);
	addAndMakeVisible(filter);
	addAndMakeVisible(mod);
	addAndMakeVisible(mtx);
	addAndMakeVisible(global);
	addAndMakeVisible(timbre);

	setupCallbacks();
}

void Editor::setupCallbacks()
{
}

void Editor::resized()
{
#if JUCE_DEBUG
    auto f = juce::File (__FILE__).getChildFile ("../../assets/layout.json");

    layout.setLayout ("layout.json", f);
#else
    layout.setLayout ("layout.json");
#endif
}
