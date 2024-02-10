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

	modDepthSlider.onClick = [this] { showModMenu(); };
	modDepthSlider.setMouseDragSensitivity(500);
	modDepthSlider.onValueChange = [this]
		{
			if (auto mm = parameter->getModMatrix())
			{
				auto dst = gin::ModDstId(parameter->getModIndex());

				if (auto depths = mm->getModDepths(dst); depths.size() > 0)
					mm->setModDepth(depths[0].first, dst, float(modDepthSlider.getValue()));
			}
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
}

void MoonKnob::showModMenu()
{
	juce::PopupMenu m;
	m.setLookAndFeel(&getLookAndFeel());

	auto& mm = *parameter->getModMatrix();
	for (auto src : mm.getModSources(parameter))
	{
		m.addItem("Remove " + mm.getModSrcName(src), [this, src]
			{
				parameter->getModMatrix()->clearModDepth(src, gin::ModDstId(parameter->getModIndex()));
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

		if (mm->isModulated(dst) || liveValuesCallback)
		{
			modTimer.startTimerHz(30);
			modDepthSlider.setVisible(mm->isModulated(dst));

			if (auto depths = mm->getModDepths(dst); depths.size() > 0)
				modDepthSlider.setValue(depths[0].second, juce::dontSendNotification);
			else
				modDepthSlider.setValue(0.0f, juce::dontSendNotification);
		}
		else
		{
			modTimer.stopTimer();
			knob.getProperties().remove("modValues");
			modDepthSlider.setVisible(false);
		}

		if (learning && !isMouseButtonDown(true))
		{
			modDepth = mm->getModDepth(mm->getLearn(), dst);
			knob.getProperties().set("modDepth", modDepth);
			repaint();
		}
	}
}

void MoonKnob::mouseDown(const juce::MouseEvent& e)
{
	if (!isEnabled())
		return;

	bool shift = juce::ModifierKeys::getCurrentModifiersRealtime().isShiftDown();
	if (shift || !learning || !knob.getBounds().contains(e.getMouseDownPosition()))
		return;

	auto& mm = *parameter->getModMatrix();
	auto dst = gin::ModDstId(parameter->getModIndex());
	modDepth = mm.getModDepth(mm.getLearn(), dst);

	knob.getProperties().set("modDepth", modDepth);

	repaint();
}

void MoonKnob::mouseDrag(const juce::MouseEvent& e)
{
	if (!isEnabled())
		return;

	bool shift = juce::ModifierKeys::getCurrentModifiersRealtime().isShiftDown();
	if (shift || !learning || !knob.getBounds().contains(e.getMouseDownPosition()))
		return;

	if (e.getDistanceFromDragStart() >= 3)
	{
		auto pt = e.getMouseDownPosition();
		auto delta = (e.position.x - pt.getX()) + (pt.getY() - e.position.y);

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
	repaint();

	auto& mm = *parameter->getModMatrix();

	auto src = gin::ModSrcId(sd.description.toString().getTrailingIntValue());
	auto dst = gin::ModDstId(parameter->getModIndex());

	mm.setModDepth(src, dst, 1.0f);
}
