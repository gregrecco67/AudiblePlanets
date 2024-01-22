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
	addAndMakeVisible(global);
	addAndMakeVisible(timbre);
    addAndMakeVisible(orbitViz);
    addAndMakeVisible(algoSelector);
    

	setupCallbacks();
    startTimerHz(frameRate);
}

void Editor::setupCallbacks()
{
}

void Editor::timerCallback() {
    auto speed = 0.2f;
    auto defRatio = proc.osc1Params.coarse->getValue() + proc.osc1Params.fine->getValue();
    auto defPhaseIncrement = defRatio * phaseIncrement;
    vizDefPhase += defPhaseIncrement * speed;
    if (vizDefPhase > juce::MathConstants<float>::twoPi)
        vizDefPhase -= juce::MathConstants<float>::twoPi;

    auto epi1Ratio = proc.osc2Params.coarse->getValue() + proc.osc2Params.fine->getValue();
    auto vizEpi1PhaseIncrement = epi1Ratio * phaseIncrement;
    vizEpi1Phase += vizEpi1PhaseIncrement * speed;
    if (vizEpi1Phase > juce::MathConstants<float>::twoPi)
        vizEpi1Phase -= juce::MathConstants<float>::twoPi;

    auto epi2Ratio = proc.osc3Params.coarse->getValue()  + proc.osc3Params.fine->getValue();
    auto vizEpi2PhaseIncrement = epi2Ratio * phaseIncrement;
    vizEpi2Phase += vizEpi2PhaseIncrement * speed;
    if (vizEpi2Phase > juce::MathConstants<float>::twoPi)
        vizEpi2Phase -= juce::MathConstants<float>::twoPi;

    auto epi3Ratio = proc.osc4Params.coarse->getValue()  + proc.osc4Params.fine->getValue();
    auto vizEpi3PhaseIncrement = epi3Ratio * phaseIncrement;
    vizEpi3Phase += vizEpi3PhaseIncrement * speed;
    if (vizEpi3Phase > juce::MathConstants<float>::twoPi)
        vizEpi3Phase -= juce::MathConstants<float>::twoPi;

    orbitViz.setEquant(proc.timbreParams.equant->getValue());
    orbitViz.setDefRad(proc.osc1Params.radius->getValue());
    orbitViz.setEpi1Rad(proc.osc2Params.radius->getValue());
    orbitViz.setEpi2Rad(proc.osc3Params.radius->getValue());
    orbitViz.setEpi3Rad(proc.osc4Params.radius->getValue());
    orbitViz.setAlgo(proc.timbreParams.algo->getUserValue());
    orbitViz.setDefPhase(vizDefPhase);
    orbitViz.setEpi1Phase(vizEpi1Phase);
    orbitViz.setEpi2Phase(vizEpi2Phase);
    orbitViz.setEpi3Phase(vizEpi3Phase);
    orbitViz.repaint();
}

void Editor::resized()
{
    auto area = getLocalBounds();
    auto width = area.getWidth();
    auto height = area.getHeight();
    orbitViz.setBounds(area.getRight() - height * 0.5, height * 0.5, height * 0.5, height * 0.5);
    
#if JUCE_DEBUG
    auto f = juce::File (__FILE__).getChildFile ("../../assets/layout.json");

    layout.setLayout ("layout.json", f);
#else
    layout.setLayout ("layout.json");
#endif
}
