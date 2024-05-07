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
#include "MoonSlider.h"
#include "Fonts.h"
#include "APModAdditions.h"
//==============================================================================
/** Slider + editable text for showing a param
*/
class MoonKnob : public gin::ParamComponent,
	public juce::DragAndDropTarget,
	private juce::Timer,
	private gin::ModMatrix::Listener
{
public:
	MoonKnob(gin::Parameter* parameter, bool fromCentre = false);
	~MoonKnob() override;

	void setLiveValuesCallback(std::function<juce::Array<float>()> cb)
	{
		liveValuesCallback = cb;
		modMatrixChanged();
	}

	MoonSlider& getSlider() { return knob; }

private:
	void paint(juce::Graphics& g) override;
	void resized() override;
	void mouseEnter(const juce::MouseEvent& e) override;
	void timerCallback() override;
	void learnSourceChanged(gin::ModSrcId) override;
	void modMatrixChanged() override;
	void parentHierarchyChanged() override;

	void mouseDown(const juce::MouseEvent& e) override;
	void mouseDrag(const juce::MouseEvent& e) override;

	bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;
	void itemDragEnter(const SourceDetails& dragSourceDetails) override;
	void itemDragExit(const SourceDetails& dragSourceDetails) override;
	void itemDropped(const SourceDetails& dragSourceDetails) override;

	void showModMenu();

	juce::Label name;
	gin::Readout value;
	MoonSlider knob;
	bool learning = false;
	float modDepth = 0.0f;
	bool dragOver = false;
	gin::ModSrcId currentModSrc{ -1 };

	gin::CoalescedTimer modTimer;
	gin::CoalescedTimer shiftTimer;
	juce::Array<float> modValues;
	std::function<juce::Array<float>()> liveValuesCallback;
	APModulationDepthSlider modDepthSlider;
    //juce::Font regularFont{ juce::Typeface::createSystemTypefaceFor(BinaryData::latoregular_otf, BinaryData::latoregular_otfSize) };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MoonKnob)
};
