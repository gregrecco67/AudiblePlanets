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

#include "MoonKnob.h"
//==============================================================================
MoonKnob::MoonKnob(gin::Parameter* p, bool fromCentre)
	: ParamComponent(p),
	value(parameter),
	knob(parameter)
{
	addAndMakeVisible(name);
	addAndMakeVisible(value);
	addAndMakeVisible(knob);
	addChildComponent(modDepthSlider);

	modDepthSlider.setRange(-1.0, 1.0, 0.01);
	modDepthSlider.setPopupDisplayEnabled(true, true, findParentComponentOfClass<juce::AudioProcessorEditor>());
	modDepthSlider.setDoubleClickReturnValue(true, 0.0);

	knob.setTitle(parameter->getName(100));
	knob.setDoubleClickReturnValue(true, parameter->getUserDefaultValue());
	knob.setSkewFactor(parameter->getSkew(), parameter->isSkewSymmetric());
	if (fromCentre)
		knob.getProperties().set("fromCentre", true);

	knob.setName(parameter->getShortName());

	name.setText(parameter->getShortName(), juce::dontSendNotification);
	name.setJustificationType(juce::Justification::centred);

#if JUCE_IOS
	knob.setMouseDragSensitivity(500);
#endif

	value.setTitle(parameter->getName(100));
	value.setJustificationType(juce::Justification::centred);

	value.setVisible(false);

	addMouseListener(this, true);

	if (parameter->getModIndex() >= 0)
	{
		auto& mm = *parameter->getModMatrix();
		mm.addListener(this);
	}

	modTimer.onTimer = [this]()
		{
			auto& mm = *parameter->getModMatrix();
			auto curModValues = liveValuesCallback ? liveValuesCallback() : mm.getLiveValues(parameter);
			if (curModValues != modValues)
			{
				modValues = curModValues;

				juce::Array<juce::var> vals;
				for (auto v : modValues)
					vals.add(v);

				knob.getProperties().set("modValues", vals);

				repaint();
			}
		};
	shiftTimer.onTimer = [this]()
		{
			bool shift = juce::ModifierKeys::getCurrentModifiersRealtime().isShiftDown();
			knob.setInterceptsMouseClicks(!learning || shift, !learning || shift);
		};

	if (auto mm = parameter->getModMatrix()) {
		if (auto depths = mm->getModDepths(gin::ModDstId(parameter->getModIndex())); depths.size() > 0) {
			currentModSrc = depths[0].first;
		}
	}


	modDepthSlider.onClick = [this] { showModMenu(); };
	modDepthSlider.setMouseDragSensitivity(500);
	modDepthSlider.onValueChange = [this]
		{
			if (auto mm = parameter->getModMatrix())
			{
				auto dst = gin::ModDstId(parameter->getModIndex());

				if (auto depths = mm->getModDepths(dst); depths.size() > 0)
					mm->setModDepth(currentModSrc, dst, float(modDepthSlider.getValue()));
			}
		};

	modDepthSlider.onTextFromValue = [this](double v)
		{
			if (auto mm = parameter->getModMatrix())
			{
				auto dst = gin::ModDstId(parameter->getModIndex());

				if (auto depths = mm->getModDepths(dst); depths.size() > 0)
				{
					auto pname = mm->getModSrcName(currentModSrc);
					return pname + ": " + juce::String(v);
				}
			}
			return juce::String();
		};

	modMatrixChanged();
}

MoonKnob::~MoonKnob()
{
	if (parameter->getModIndex() >= 0)
	{
		auto& mm = *parameter->getModMatrix();
		mm.removeListener(this);
	}
	removeMouseListener(this);
    stopTimer();
}

void MoonKnob::showModMenu()
{
	juce::PopupMenu m;
	m.setLookAndFeel(&getLookAndFeel());

	auto& mm = *parameter->getModMatrix();
	for (auto src : mm.getModSources(parameter))
	{
		bool current{ false };
		if (currentModSrc == gin::ModSrcId{ -1 }) { currentModSrc = src; }
		if (src == currentModSrc) { current = true; }
		m.addItem("Remove: " + mm.getModSrcName(src), true, current, [this, src]
			{
				auto dst = gin::ModDstId(parameter->getModIndex());
				parameter->getModMatrix()->clearModDepth(src, dst);
				if (auto depths = parameter->getModMatrix()->getModDepths(dst); depths.size() > 0) {
					currentModSrc = depths[0].first;
				}
				else {
					currentModSrc = gin::ModSrcId(-1);
				}
				modMatrixChanged();
			});
	}

	m.addSeparator();

	for (auto src : mm.getModSources(parameter))
	{
		if (currentModSrc == gin::ModSrcId{ -1 }) { currentModSrc = src; }
		bool editing = (src == currentModSrc) ? true : false;
		m.addItem("Edit: " + mm.getModSrcName(src) + ": " + String(mm.getModDepth(src, gin::ModDstId(parameter->getModIndex())), 3), !editing, editing, [this, src]
			{
				currentModSrc = src;
				modMatrixChanged();
			});
	}

	m.showMenuAsync({});
}

void MoonKnob::paint(juce::Graphics& g)
{
	if (dragOver)
	{
		g.setColour(findColour(gin::PluginLookAndFeel::accentColourId, true).withAlpha(0.3f));
		g.fillEllipse(knob.getBounds().toFloat());
	}
}

void MoonKnob::resized()
{
	auto r = getLocalBounds().reduced(1);

	auto extra = r.getHeight() - r.getWidth();

	auto rc = r.removeFromBottom(extra);

	name.setBounds(rc);
	value.setBounds(rc);
	knob.setBounds(r);
	modDepthSlider.setBounds(knob.getBounds().removeFromTop(13).removeFromRight(13));
}

void MoonKnob::mouseEnter(const juce::MouseEvent&)
{
	if (wantsAccessibleKeyboard(*this))
		return;

	if (!isTimerRunning() && isEnabled())
	{
		startTimer(100);
		name.setVisible(false);
		value.setVisible(true);
	}
}

void MoonKnob::timerCallback()
{
	if (!isVisible()) return;
	auto p = getMouseXYRelative();
	if (!getLocalBounds().contains(p) &&
		!juce::ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown() &&
		!value.isBeingEdited())
	{
		if (wantsAccessibleKeyboard(*this))
		{
			name.setVisible(false);
			value.setVisible(true);
		}
		else
		{
			name.setVisible(true);
			value.setVisible(false);
		}

		stopTimer();
	}
}

void MoonKnob::parentHierarchyChanged()
{
	auto a = wantsAccessibleKeyboard(*this);
	name.setWantsKeyboardFocus(a);
	value.setWantsKeyboardFocus(a);
	knob.setWantsKeyboardFocus(a);

	if (wantsAccessibleKeyboard(*this))
	{
		name.setVisible(false);
		value.setVisible(true);
	}
	else
	{
		name.setVisible(true);
		value.setVisible(false);
	}
}

void MoonKnob::learnSourceChanged(gin::ModSrcId src)
{
	learning = src.isValid();

	bool shift = juce::ModifierKeys::getCurrentModifiersRealtime().isShiftDown();
	knob.setInterceptsMouseClicks(!learning || shift, !learning || shift);

	auto& mm = *parameter->getModMatrix();
	modDepth = mm.getModDepth(mm.getLearn(), gin::ModDstId(parameter->getModIndex()));

	if (learning)
	{
		knob.getProperties().set("modDepth", modDepth);
		knob.getProperties().set("modBipolar", mm.getModSrcBipolar(mm.getLearn()));

		shiftTimer.startTimerHz(100);
	}
	else
	{
		knob.getProperties().remove("modDepth");
		knob.getProperties().remove("modBipolar");

		shiftTimer.stopTimer();
	}

	repaint();
}

void MoonKnob::modMatrixChanged()
{
	if (auto mm = parameter->getModMatrix())
	{

		auto dst = gin::ModDstId(parameter->getModIndex());
		for (auto src : mm->getModSources(parameter))
		{
			bool current{ false };
			if (currentModSrc == gin::ModSrcId{ -1 }) { currentModSrc = src; }
		}

		if (mm->isModulated(dst) || liveValuesCallback)
		{
			modTimer.startTimerHz(30);

			auto vis = mm->isModulated(dst);
			if (vis != modDepthSlider.isVisible())
			{
				modDepthSlider.setVisible(vis);
				resized();
			}

			if (auto depths = mm->getModDepths(dst); depths.size() > 0) {
				for (auto depth : depths) {
					if (depth.first == currentModSrc)
						modDepthSlider.setValue(depth.second, juce::dontSendNotification);
				}
			}
			else
				modDepthSlider.setValue(0.0f, juce::dontSendNotification);
		}
		else
		{
			modTimer.stopTimer();
			knob.getProperties().remove("modValues");

			if (modDepthSlider.isVisible())
			{
				modDepthSlider.setVisible(false);
				resized();
			}
		}

		if (learning && !isMouseButtonDown(true))
		{
			modDepth = mm->getModDepth(mm->getLearn(), dst);
			knob.getProperties().set("modDepth", modDepth);
			knob.getProperties().set("modBipolar", mm->getModBipolarMapping(mm->getLearn(), gin::ModDstId(parameter->getModIndex())));
			repaint();
		}
	}
}

void MoonKnob::mouseDown(const juce::MouseEvent& ev)
{
	if (!isEnabled())
		return;

	bool shift = juce::ModifierKeys::getCurrentModifiersRealtime().isShiftDown();
	if (shift || !learning || !knob.getBounds().contains(ev.getMouseDownPosition()))
		return;

	auto& mm = *parameter->getModMatrix();
	auto dst = gin::ModDstId(parameter->getModIndex());
	modDepth = mm.getModDepth(mm.getLearn(), dst);

	knob.getProperties().set("modDepth", modDepth);

	repaint();
}

void MoonKnob::mouseDrag(const juce::MouseEvent& ev)
{
	if (!isEnabled())
		return;

	bool shift = juce::ModifierKeys::getCurrentModifiersRealtime().isShiftDown();
	if (shift || !learning || !knob.getBounds().contains(ev.getMouseDownPosition()))
		return;

	if (ev.getDistanceFromDragStart() >= 3)
	{
		auto pt = ev.getMouseDownPosition();
		auto delta = (ev.position.x - pt.getX()) + (pt.getY() - ev.position.y);

		float newModDepth = juce::jlimit(-1.0f, 1.0f, delta / 200.0f + modDepth);

		knob.getProperties().set("modDepth", newModDepth);

		auto& mm = *parameter->getModMatrix();
		auto dst = gin::ModDstId(parameter->getModIndex());
		mm.setModDepth(mm.getLearn(), dst, newModDepth);

		repaint();
	}
}

bool MoonKnob::isInterestedInDragSource(const SourceDetails& sd)
{
	if (isEnabled() && parameter && parameter->getModMatrix())
		return sd.description.toString().startsWith("modSrc");

	return false;
}

void MoonKnob::itemDragEnter(const SourceDetails&)
{
	dragOver = true;
	repaint();
}

void MoonKnob::itemDragExit(const SourceDetails&)
{
	dragOver = false;
	repaint();
}

void MoonKnob::itemDropped(const SourceDetails& sd)
{
	dragOver = false;

	auto& mm = *parameter->getModMatrix();

	currentModSrc = gin::ModSrcId(sd.description.toString().getTrailingIntValue());
	auto dst = gin::ModDstId(parameter->getModIndex());

	mm.setModDepth(currentModSrc, dst, 1.0f);
	repaint();
}
