#include "Editor.h"

Editor::Editor (APAudioProcessor& proc_)
    : proc ( proc_ ), algoSelector (proc.timbreParams.algo)
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
    addAndMakeVisible(orbit);
	
	//orbit.addAndMakeVisible(algoSelector);
	setupCallbacks();
    startTimerHz(frameRate);
}

void Editor::setupCallbacks()
{
}

void Editor::timerCallback() {
    auto speed = proc.orbitParams.speed->getUserValue();
    auto defRatio = proc.osc1Params.coarse->getUserValue() + proc.osc1Params.fine->getUserValue();
    auto defPhaseIncrement = defRatio * phaseIncrement;
    vizDefPhase += defPhaseIncrement * speed;
    if (vizDefPhase > juce::MathConstants<float>::twoPi)
        vizDefPhase -= juce::MathConstants<float>::twoPi;

    auto epi1Ratio = proc.osc2Params.coarse->getUserValue() + proc.osc2Params.fine->getUserValue();
    auto vizEpi1PhaseIncrement = epi1Ratio * phaseIncrement;
    vizEpi1Phase += vizEpi1PhaseIncrement * speed;
    if (vizEpi1Phase > juce::MathConstants<float>::twoPi)
        vizEpi1Phase -= juce::MathConstants<float>::twoPi;

    auto epi2Ratio = proc.osc3Params.coarse->getUserValue()  + proc.osc3Params.fine->getUserValue();
    auto vizEpi2PhaseIncrement = epi2Ratio * phaseIncrement;
    vizEpi2Phase += vizEpi2PhaseIncrement * speed;
    if (vizEpi2Phase > juce::MathConstants<float>::twoPi)
        vizEpi2Phase -= juce::MathConstants<float>::twoPi;

    auto epi3Ratio = proc.osc4Params.coarse->getUserValue()  + proc.osc4Params.fine->getUserValue();
    auto vizEpi3PhaseIncrement = epi3Ratio * phaseIncrement;
    vizEpi3Phase += vizEpi3PhaseIncrement * speed;
    if (vizEpi3Phase > juce::MathConstants<float>::twoPi)
        vizEpi3Phase -= juce::MathConstants<float>::twoPi;

    orbitViz.setEquant(proc.timbreParams.equant->getUserValue());
    orbitViz.setDefRad(proc.osc1Params.radius->getValue());
    orbitViz.setEpi1Rad(proc.osc2Params.radius->getValue());
    orbitViz.setEpi2Rad(proc.osc3Params.radius->getValue());
    orbitViz.setEpi3Rad(proc.osc4Params.radius->getValue());
    orbitViz.setAlgo(proc.timbreParams.algo->getUserValue());
    orbitViz.setDefPhase(vizDefPhase);
    orbitViz.setEpi1Phase(vizEpi1Phase);
    orbitViz.setEpi2Phase(vizEpi2Phase);
    orbitViz.setEpi3Phase(vizEpi3Phase);
	orbitViz.setScale(proc.orbitParams.scale->getUserValue());
    orbitViz.repaint();
}

void Editor::resized()
{
    auto area = getLocalBounds();
    auto width = area.getWidth();
    auto height = area.getHeight();
    orbitViz.setBounds(area.getRight() - (394), height * 0.5, 394, height * 0.5);
    
#if JUCE_DEBUG
    auto f = juce::File (__FILE__).getChildFile ("../../assets/layout.json");

    layout.setLayout ("layout.json", f);
#else
    layout.setLayout ("layout.json");
#endif
}
