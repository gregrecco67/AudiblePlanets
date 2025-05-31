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

#include <numbers>
#include "BinaryData.h"
#include "DSP/PluginProcessor.h"
#include "Fonts.h"

using std::numbers::pi;

//==============================================================================
/** A button for the modulation destination
 */
class APModulationDepthSlider : public juce::Slider {
public:
	APModulationDepthSlider()
	    : juce::Slider(RotaryHorizontalVerticalDrag, NoTextBox)
	{
	}

	~APModulationDepthSlider() override = default;

	juce::String getTextFromValue(double value) override
	{
		if (onTextFromValue)
			return onTextFromValue(value);
		return {value, 2};
	}

	std::function<void()> onClick;
	std::function<juce::String(double)> onTextFromValue;

private:
	void paint(juce::Graphics &g) override
	{
		auto c = juce::Colour(0xff444444);
		g.setColour(c);

		auto rc = getLocalBounds().toFloat().reduced(1.5f);
		g.fillEllipse(rc);

		if (auto v = float(getValue()); v > 0.0f || v < 0.0f) {
			g.setColour(findColour(gin::PluginLookAndFeel::accentColourId)
			        .withAlpha(0.9f));

			juce::Path p;
			p.addPieSegment(
			    rc, 0.0f, juce::MathConstants<float>::pi * 2 * v, 0.0f);

			g.fillPath(p);
		}
	}

	void mouseUp(const juce::MouseEvent &ev) override
	{
		if (ev.mouseWasClicked() && ev.mods.isPopupMenu() && onClick)
			onClick();
	}
};

class MainVolSlider : public juce::Slider {
public:
	MainVolSlider() : juce::Slider()  //, juce::Slider::NoTextBox)
	{
		setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
		setColour(
		    juce::Slider::textBoxTextColourId, juce::Colours::transparentBlack);
	}

	void mouseEnter(const juce::MouseEvent &ev) override
	{
		juce::Slider::mouseEnter(ev);
		setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
		setColour(juce::Slider::thumbColourId, juce::Colours::darkgrey);
	}

	void mouseExit(const juce::MouseEvent &ev) override
	{
		juce::Slider::mouseExit(ev);
		setColour(
		    juce::Slider::textBoxTextColourId, juce::Colours::transparentBlack);
		setColour(juce::Slider::thumbColourId, juce::Colour(0xffCC8866));
	}

	juce::String getTextFromValue(double value) override
	{
		return {value, 3};
	}

	std::function<juce::String(double)> onTextFromValue;
};

//==============================================================================
/** A list box of all assigned
 */
class APModMatrixBox : public juce::ListBox,
                       private juce::ListBoxModel,
                       private gin::ModMatrix::Listener {
public:
	// APModMatrixBox(APAudioProcessor& p, gin::ModMatrix& m, int dw = 50)
	APModMatrixBox(gin::Processor &p, gin::ModMatrix &m, int dw = 50)
	    : proc(p), modMatrix(m), depthWidth(dw)
	{
		setName("matrix");
		setModel(this);
		setRowHeight(18);
		refresh();

		modMatrix.addListener(this);
	}

	~APModMatrixBox() override
	{
		modMatrix.removeListener(this);
		setModel(nullptr);
	}

	void refresh()
	{
		assignments.clear();

		auto &params = proc.getPluginParameters();
		for (gin::Parameter *p : params)
			for (const auto& s : modMatrix.getModSources(p))
				assignments.add({s, p});

		updateContent();
		repaint();
	}

	int getNumRows() override { return assignments.size(); }

	void paintListBoxItem(int, juce::Graphics &, int, int, bool) override {}

	juce::Component *refreshComponentForRow(
	    int row, bool, juce::Component *c) override
	{
		auto rowComponent = dynamic_cast<Row *>(c);
		if (rowComponent == nullptr)
			rowComponent = new Row(*this);

		rowComponent->update(row);
		return rowComponent;
	}

	void modMatrixChanged() override { refresh(); }

	class Row : public juce::Component, private juce::Slider::Listener {
	public:
		explicit Row(APModMatrixBox &o) : owner(o)
		{
			addAndMakeVisible(enableButton);
			addAndMakeVisible(deleteButton);
			addAndMakeVisible(curveButton);
			addAndMakeVisible(biPolarButton);
			addAndMakeVisible(depth);
			addAndMakeVisible(src);
			addAndMakeVisible(dst);

			depth.setLookAndFeel(&depthLookAndFeel);

			depth.setRange(-1.0, 1.0);
			depth.addListener(this);
			depth.setSliderSnapsToMousePosition(false);
			depth.setMouseDragSensitivity(750);
			depth.setPopupDisplayEnabled(false, false, nullptr);
			// ctrl-click to reset, cmd-click or alt-click for fine adjustment
			depth.setDoubleClickReturnValue(
			    true, 0.0, juce::ModifierKeys::ctrlModifier);
			depth.onValueChange = [this] {
				auto &a = owner.assignments.getReference(row);
				auto parameter = a.dst;
				auto dstId = gin::ModDstId(parameter->getModIndex());
				auto range = parameter->getUserRange();
				owner.modMatrix.setModDepth(
				    a.src, dstId, float(depth.getValue()));
			};

			enableButton.onClick = [this] {
				if (row >= 0 && row < owner.assignments.size()) {
					auto &a = owner.assignments.getReference(row);

					auto ev = owner.modMatrix.getModEnable(
					    a.src, gin::ModDstId(a.dst->getModIndex()));
					owner.modMatrix.setModEnable(
					    a.src, gin::ModDstId(a.dst->getModIndex()), !ev);
					enableButton.setToggleState(
					    !ev, juce::dontSendNotification);
					if (ev)
						depth.setEnabled(false);
					else
						depth.setEnabled(true);
				}
			};

			biPolarButton.onClick = [this] {
				if (row >= 0 && row < owner.assignments.size()) {
					auto &a = owner.assignments.getReference(row);

					auto ev = owner.modMatrix.getModBipolarMapping(
					    a.src, gin::ModDstId(a.dst->getModIndex()));
					owner.modMatrix.setModBipolarMapping(
					    a.src, gin::ModDstId(a.dst->getModIndex()), !ev);
					biPolarButton.setToggleState(
					    ev, juce::dontSendNotification);
				}
			};

			deleteButton.onClick = [this] {
				if (row >= 0 && row < owner.assignments.size()) {
					auto &a = owner.assignments.getReference(row);
					owner.modMatrix.clearModDepth(
					    a.src, gin::ModDstId(a.dst->getModIndex()));
				}
			};

			curveButton.onClick = [this] {
				if (row >= 0 && row < owner.assignments.size()) {
					auto &a = owner.assignments.getReference(row);
					auto f = owner.modMatrix.getModFunction(
					    a.src, gin::ModDstId(a.dst->getModIndex()));

					auto set = [this](gin::ModMatrix::Function func) {
						auto &aa = owner.assignments.getReference(row);
						owner.modMatrix.setModFunction(
						    aa.src, gin::ModDstId(aa.dst->getModIndex()), func);
					};

					juce::PopupMenu m;

					m.addItem("Linear", true,
					    f == gin::ModMatrix::Function::linear,
					    [set] { set(gin::ModMatrix::Function::linear); });
					m.addItem("Quadratic In", true,
					    f == gin::ModMatrix::Function::quadraticIn,
					    [set] { set(gin::ModMatrix::Function::quadraticIn); });
					m.addItem("Quadratic In/Out", true,
					    f == gin::ModMatrix::Function::quadraticInOut, [set] {
						    set(gin::ModMatrix::Function::quadraticInOut);
					    });
					m.addItem("Quadratic Out", true,
					    f == gin::ModMatrix::Function::quadraticOut,
					    [set] { set(gin::ModMatrix::Function::quadraticOut); });
					m.addItem("Sine In", true,
					    f == gin::ModMatrix::Function::sineIn,
					    [set] { set(gin::ModMatrix::Function::sineIn); });
					m.addItem("Sine In Out", true,
					    f == gin::ModMatrix::Function::sineInOut,
					    [set] { set(gin::ModMatrix::Function::sineInOut); });
					m.addItem("Sine Out", true,
					    f == gin::ModMatrix::Function::sineOut,
					    [set] { set(gin::ModMatrix::Function::sineOut); });
					m.addItem("Exponential In", true,
					    f == gin::ModMatrix::Function::exponentialIn, [set] {
						    set(gin::ModMatrix::Function::exponentialIn);
					    });
					m.addItem("Exponential In/Out", true,
					    f == gin::ModMatrix::Function::exponentialInOut, [set] {
						    set(gin::ModMatrix::Function::exponentialInOut);
					    });
					m.addItem("Exponential Out", true,
					    f == gin::ModMatrix::Function::exponentialOut, [set] {
						    set(gin::ModMatrix::Function::exponentialOut);
					    });
					m.addSeparator();
					m.addItem("Inv Linear", true,
					    f == gin::ModMatrix::Function::invLinear,
					    [set] { set(gin::ModMatrix::Function::invLinear); });
					m.addItem("Inv Quadratic In", true,
					    f == gin::ModMatrix::Function::invQuadraticIn, [set] {
						    set(gin::ModMatrix::Function::invQuadraticIn);
					    });
					m.addItem("Inv Quadratic In/Out", true,
					    f == gin::ModMatrix::Function::invQuadraticInOut,
					    [set] {
						    set(gin::ModMatrix::Function::invQuadraticInOut);
					    });
					m.addItem("Inv Quadratic Out", true,
					    f == gin::ModMatrix::Function::invQuadraticOut, [set] {
						    set(gin::ModMatrix::Function::invQuadraticOut);
					    });
					m.addItem("Inv Sine In", true,
					    f == gin::ModMatrix::Function::invSineIn,
					    [set] { set(gin::ModMatrix::Function::invSineIn); });
					m.addItem("Inv Sine In/Out", true,
					    f == gin::ModMatrix::Function::invSineInOut,
					    [set] { set(gin::ModMatrix::Function::invSineInOut); });
					m.addItem("Inv Sine Out", true,
					    f == gin::ModMatrix::Function::invSineOut,
					    [set] { set(gin::ModMatrix::Function::invSineOut); });
					m.addItem("Inv Exponential In", true,
					    f == gin::ModMatrix::Function::invExponentialIn, [set] {
						    set(gin::ModMatrix::Function::invExponentialIn);
					    });
					m.addItem("Inv Exponential In/Out", true,
					    f == gin::ModMatrix::Function::invExponentialInOut,
					    [set] {
						    set(gin::ModMatrix::Function::invExponentialInOut);
					    });
					m.addItem("Inv Exponential Out", true,
					    f == gin::ModMatrix::Function::invExponentialOut,
					    [set] {
						    set(gin::ModMatrix::Function::invExponentialOut);
					    });

					m.setLookAndFeel(&popupLNF);
					m.showMenuAsync({});
				}
			};
		}

		~Row() override
		{
			depth.setLookAndFeel(nullptr);
			depth.removeListener(this);
		}

		void sliderValueChanged(juce::Slider *) override
		{
			if (row >= 0 && row < owner.assignments.size()) {
				auto &a = owner.assignments.getReference(row);
				owner.modMatrix.setModDepth(a.src,
				    gin::ModDstId(a.dst->getModIndex()),
				    (float)depth.getValue());
			}
		}

		void update(int idx)
		{
			row = idx;

			if (idx >= 0 && idx < owner.assignments.size()) {
				auto &a = owner.assignments.getReference(idx);
				src.setText(owner.modMatrix.getModSrcName(a.src),
				    juce::dontSendNotification);
				dst.setText(a.dst->getName(100), juce::dontSendNotification);

				auto ev = owner.modMatrix.getModEnable(
				    a.src, gin::ModDstId(a.dst->getModIndex()));
				enableButton.setToggleState(ev, juce::dontSendNotification);

				auto b = owner.modMatrix.getModBipolarMapping(
				    a.src, gin::ModDstId(a.dst->getModIndex()));
				biPolarButton.setToggleState(!b, juce::dontSendNotification);

				depth.setValue(owner.modMatrix.getModDepth(
				                   a.src, gin::ModDstId(a.dst->getModIndex())),
				    juce::dontSendNotification);
				curveButton.setCurve(owner.modMatrix.getModFunction(
				    a.src, gin::ModDstId(a.dst->getModIndex())));
			} else {
				src.setText("", juce::dontSendNotification);
				dst.setText("", juce::dontSendNotification);
				curveButton.setCurve(gin::ModMatrix::Function::linear);
			}
		}

		void resized() override
		{
			auto rc = getLocalBounds().reduced(2);

			int h = rc.getHeight();

			enableButton.setBounds(rc.removeFromLeft(h));
			rc.removeFromLeft(4);
			deleteButton.setBounds(rc.removeFromRight(h));
			rc.removeFromLeft(2);
			depth.setBounds(rc.removeFromLeft(owner.depthWidth));
			rc.removeFromLeft(4);
			biPolarButton.setBounds(rc.removeFromLeft(h));
			rc.removeFromLeft(2);
			curveButton.setBounds(rc.removeFromLeft(h));

			int w = rc.getWidth() / 2;
			src.setBounds(rc.removeFromLeft(w));
			dst.setBounds(rc.removeFromLeft(w));
		}

		class PopupLNF : public juce::LookAndFeel_V4 {
		public:
			PopupLNF()
			{
				setColour(juce::PopupMenu::backgroundColourId,
				    juce::Colour(0xff16171A));
				setColour(juce::PopupMenu::textColourId, 
					juce::Colour(0xff66AACC));
				setColour(juce::PopupMenu::headerTextColourId,
				    juce::Colour(0xff9B9EA5));
				setColour(juce::PopupMenu::highlightedTextColourId,
				    juce::Colours::white);
				setColour(juce::PopupMenu::highlightedBackgroundColourId,
				    juce::Colours::darkgrey);
			}
		};

		PopupLNF popupLNF;

		APModMatrixBox &owner;
		int row = 0;

		class APDepthSlider : public juce::Slider {
		public:
			APDepthSlider() : juce::Slider()  //, juce::Slider::NoTextBox)
			{
				setSliderStyle(juce::Slider::SliderStyle::LinearBar);
				setColour(juce::Slider::textBoxTextColourId,
				    juce::Colours::transparentBlack);
			}

			void mouseEnter(const juce::MouseEvent &ev) override
			{
				juce::Slider::mouseEnter(ev);
				setColour(
				    juce::Slider::textBoxTextColourId, juce::Colours::white);
				setColour(juce::Slider::thumbColourId, juce::Colours::darkgrey);
			}

			void mouseExit(const juce::MouseEvent &ev) override
			{
				juce::Slider::mouseExit(ev);
				setColour(juce::Slider::textBoxTextColourId,
				    juce::Colours::transparentBlack);
				setColour(juce::Slider::thumbColourId,
				    findColour(gin::CopperLookAndFeel::accentColourId));
			}

			juce::String getTextFromValue(double value) override
			{
				return {value, 3};
			}

			std::function<juce::String(double)> onTextFromValue;
		};

		class APModDepthLookAndFeel : public gin::CopperLookAndFeel {
		public:
			APModDepthLookAndFeel()
			{
				setColour(juce::Slider::textBoxOutlineColourId,
				    juce::Colours::transparentBlack);
				setColour(juce::PopupMenu::backgroundColourId,
				    juce::Colour(0xff16171A));
				setColour(
				    juce::PopupMenu::textColourId, juce::Colour(0xffE6E6E9));
				setColour(juce::PopupMenu::headerTextColourId,
				    juce::Colour(0xff9B9EA5));
				setColour(juce::PopupMenu::highlightedBackgroundColourId,
				    juce::Colour(0xffCC8866));
				setColour(juce::PopupMenu::highlightedTextColourId,
				    juce::Colours::white);
			}

			void drawLinearSlider(juce::Graphics &g,
			    int x,
			    int y,
			    int width,
			    int height,
			    float sliderPos,
			    float /*minSliderPos*/,
			    float /*maxSliderPos*/,
			    const juce::Slider::SliderStyle /*style*/,
			    juce::Slider &slider) override
			{
				// const bool isMouseOver = slider.isMouseOverOrDragging();
				auto rc = juce::Rectangle<int>(x, y, width, height);
				rc = rc.withSizeKeepingCentre(width, height);

				// track
				g.setColour(slider.findColour(juce::Slider::trackColourId)
				        .withAlpha(0.1f));
				g.fillRect(rc);

				// thumb
				if (slider.isEnabled()) {
					g.setColour(slider.findColour(juce::Slider::thumbColourId)
					        .withAlpha(0.85f));
				} else {
					g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));
				}
				auto t = static_cast<float>(rc.getY());
				auto h = static_cast<float>(rc.getHeight());
				auto c =  static_cast<float>(rc.getCentreX());
				if (sliderPos < c)
					g.fillRect(
					    juce::Rectangle<float>(sliderPos, t, c - sliderPos, h));
				else
					g.fillRect(
					    juce::Rectangle<float>(float(c), t, sliderPos - c, h));
			}
		};

		APDepthSlider depth;
		APModDepthLookAndFeel depthLookAndFeel;

		juce::Label src;
		juce::Label dst;

		gin::ModCurveButton curveButton;
		gin::SVGButton biPolarButton{"bi", gin::Assets::bipolar};

		gin::SVGButton enableButton{"enable", gin::Assets::power, 1};
		gin::SVGButton deleteButton{"delete", gin::Assets::del};
	};

	struct Assignment {
		gin::ModSrcId src = {};
		gin::Parameter *dst = nullptr;
	};

	gin::Processor &proc;
	gin::ModMatrix &modMatrix;
	juce::Array<Assignment> assignments;
	int depthWidth = 50;
};

//==============================================================================
/** Slider + editable text for showing a param
 */
class APKnob : public gin::ParamComponent,
               public juce::DragAndDropTarget,
               private juce::Timer,
               private gin::ModMatrix::Listener {
public:
	explicit APKnob(gin::Parameter *p, bool fromCentre = false)
	    : gin::ParamComponent(p), value(parameter),
	      knob(parameter,
	          juce::Slider::RotaryHorizontalVerticalDrag,
	          juce::Slider::NoTextBox)
	{
		addAndMakeVisible(name);
		addAndMakeVisible(value);
		addAndMakeVisible(knob);
		addChildComponent(modDepthSlider);

		modDepthSlider.setRange(-1.0, 1.0, 0.001);
		modDepthSlider.setPopupDisplayEnabled(true, true,
		    findParentComponentOfClass<juce::AudioProcessorEditor>());
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

		if (parameter->getModIndex() >= 0) {
			auto &mm = *parameter->getModMatrix();
			mm.addListener(this);
		}

		modTimer.onTimer = [this]() {
			auto &mm = *parameter->getModMatrix();
			if (mm.shouldShowLiveModValues()) {
				auto curModValues = liveValuesCallback
				                        ? liveValuesCallback()
				                        : mm.getLiveValues(parameter);
				if (curModValues != modValues) {
					modValues = curModValues;

					juce::Array<juce::var> vals;
					for (auto v : modValues)
						vals.add(v);

					knob.getProperties().set("modValues", vals);

					repaint();
				}
			} else if (knob.getProperties().contains("modValues")) {
				knob.getProperties().remove("modValues");
				repaint();
			}
		};
		shiftTimer.onTimer = [this]() {
			bool shift =
			    juce::ModifierKeys::getCurrentModifiersRealtime().isShiftDown();
			knob.setInterceptsMouseClicks(
			    !learning || shift, !learning || shift);
		};

		if (auto mm = parameter->getModMatrix()) {
			if (auto depths = mm->getModDepths(gin::ModDstId(parameter->getModIndex())); !depths.empty())
				{ currentModSrc = depths[0].first; }
		}
		modDepthSlider.onClick = [this] { showModMenu(); };
		modDepthSlider.setMouseDragSensitivity(500);
		modDepthSlider.onValueChange = [this] {
			if (auto mm = parameter->getModMatrix()) {
				auto dst = gin::ModDstId(parameter->getModIndex());
				if (auto depths = mm->getModDepths(dst); !depths.empty())
					{ mm->setModDepth(currentModSrc, dst, float(modDepthSlider.getValue())); }
			}
		};

		modDepthSlider.onTextFromValue = [this](double v) {
			if (auto mm = parameter->getModMatrix()) {
				auto dst = gin::ModDstId(parameter->getModIndex());

				if (auto depths = mm->getModDepths(dst); !depths.empty()) {
					auto pname = mm->getModSrcName(currentModSrc);
					return pname + ": " + juce::String(v);
				}
			}
			return juce::String();
		};

		modMatrixChanged();
	}

	~APKnob() override
	{
		if (parameter->getModIndex() >= 0) {
			auto &mm = *parameter->getModMatrix();
			mm.removeListener(this);
		}
		setLookAndFeel(nullptr);
	}

	void setDisplayName(const juce::String &n)
	{
		name.setText(n, juce::dontSendNotification);
	}

	void setLiveValuesCallback(std::function<juce::Array<float>()> cb)
	{
		liveValuesCallback = cb;
		modMatrixChanged();
	}

	gin::PluginSlider &getSlider() { return knob; }
	gin::Readout &getReadout() { return value; }

	void paint(juce::Graphics &g) override
	{
		if (dragOver) {
			g.setColour(findColour(gin::PluginLookAndFeel::accentColourId, true)
			        .withAlpha(0.3f));
			g.fillEllipse(knob.getBounds().toFloat());
		}
	}

	void resized() override
	{
		auto r = getLocalBounds().reduced(2);

		auto extra = r.getHeight() - r.getWidth();

		auto rc = r.removeFromBottom(extra);

		name.setBounds(rc);
		value.setBounds(rc);
		knob.setBounds(r.reduced(2));

		modDepthSlider.setBounds(
		    knob.getBounds().removeFromTop(7).removeFromRight(7).reduced(-3));
	}

	void parentHierarchyChanged() override
	{
		auto a = wantsAccessibleKeyboard(*this);
		name.setWantsKeyboardFocus(a);
		value.setWantsKeyboardFocus(a);
		knob.setWantsKeyboardFocus(a);

		if (wantsAccessibleKeyboard(*this)) {
			name.setVisible(false);
			value.setVisible(true);
		} else {
			name.setVisible(true);
			value.setVisible(false);
		}
	}

	void mouseEnter(const juce::MouseEvent & /*ev*/) override
	{
		if (wantsAccessibleKeyboard(*this))
			return;

		if (!isTimerRunning() && isEnabled()) {
			startTimer(100);
			name.setVisible(false);
			value.setVisible(true);
		}
	}

	void mouseDown(const juce::MouseEvent &ev) override
	{
		if (!isEnabled())
			return;

		bool shift =
		    juce::ModifierKeys::getCurrentModifiersRealtime().isShiftDown();
		if (shift || !learning ||
		    !knob.getBounds().contains(ev.getMouseDownPosition()))
			return;

		auto &mm = *parameter->getModMatrix();
		auto dst = gin::ModDstId(parameter->getModIndex());
		modDepth = mm.getModDepth(mm.getLearn(), dst);

		knob.getProperties().set("modDepth", modDepth);

		repaint();
	}

	void mouseDrag(const juce::MouseEvent &ev) override
	{
		if (!isEnabled())
			return;

		bool shift =
		    juce::ModifierKeys::getCurrentModifiersRealtime().isShiftDown();
		if (shift || !learning ||
		    !knob.getBounds().contains(ev.getMouseDownPosition()))
			return;

		if (ev.getDistanceFromDragStart() >= 3) {
			auto pt = ev.getMouseDownPosition();
			auto delta =
			    (ev.position.x - pt.getX()) + (pt.getY() - ev.position.y);

			float newModDepth =
			    juce::jlimit(-1.0f, 1.0f, delta / 200.0f + modDepth);

			auto &mm = *parameter->getModMatrix();

			auto dst = gin::ModDstId(parameter->getModIndex());

			knob.getProperties().set("modDepth", newModDepth);
			if (bool bi = mm.getModBipolarMapping(mm.getLearn(), dst)) {
				knob.getProperties().set("modBipolar", bi);
			}

			auto range = parameter->getUserRange();
			if (range.interval <= 0.0f ||
			    juce::ModifierKeys::currentModifiers.isShiftDown()) {
				mm.setModDepth(mm.getLearn(), dst, newModDepth);
			} else {
				mm.setModDepth(mm.getLearn(), dst, newModDepth);
				modDepthSlider.setValue(
				    newModDepth, juce::dontSendNotification);
			}

			repaint();
		}
	}

	bool isInterestedInDragSource(const SourceDetails &sd) override
	{
		if (isEnabled() && parameter && parameter->getModMatrix())
			return sd.description.toString().startsWith("modSrc");

		return false;
	}

	void itemDragEnter(const SourceDetails & /*dragSourceDetails*/) override
	{
		dragOver = true;
		repaint();
	}

	void itemDragExit(const SourceDetails & /*dragSourceDetails*/) override
	{
		dragOver = false;
		repaint();
	}

	void itemDropped(const SourceDetails &sd) override
	{
		dragOver = false;

		auto &mm = *parameter->getModMatrix();

		currentModSrc =
		    gin::ModSrcId(sd.description.toString().getTrailingIntValue());
		auto dst = gin::ModDstId(parameter->getModIndex());

		mm.setModDepth(currentModSrc, dst, 1.0f);
		repaint();
	}

	void timerCallback() override
	{
		if (!isVisible())
			return;
		auto p = getMouseXYRelative();
		if (!getLocalBounds().contains(p) &&
		    !juce::ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown() &&
		    !value.isBeingEdited()) {
			if (wantsAccessibleKeyboard(*this)) {
				name.setVisible(false);
				value.setVisible(true);
			} else {
				name.setVisible(true);
				value.setVisible(false);
			}

			stopTimer();
		}
	}

	void learnSourceChanged(gin::ModSrcId src) override
	{
		learning = src.isValid();

		bool shift =
		    juce::ModifierKeys::getCurrentModifiersRealtime().isShiftDown();
		knob.setInterceptsMouseClicks(!learning || shift, !learning || shift);

		auto &mm = *parameter->getModMatrix();
		modDepth = mm.getModDepth(
		    mm.getLearn(), gin::ModDstId(parameter->getModIndex()));

		if (learning) {
			knob.getProperties().set("modDepth", modDepth);
			knob.getProperties().set(
			    "modBipolar", mm.getModBipolarMapping(mm.getLearn(),
			                      gin::ModDstId(parameter->getModIndex())));

			shiftTimer.startTimerHz(100);
		} else {
			knob.getProperties().remove("modDepth");
			knob.getProperties().remove("modBipolar");

			shiftTimer.stopTimer();
		}

		repaint();
	}

	void modMatrixChanged() override
	{
		if (auto mm = parameter->getModMatrix()) {
			auto dst = gin::ModDstId(parameter->getModIndex());
			auto sources = mm->getModSources(parameter);
			if (!sources.contains(currentModSrc)) {
				currentModSrc = gin::ModSrcId{-1};
			}
			for (const auto& src : sources) {
				if (currentModSrc == gin::ModSrcId{-1}) {
					currentModSrc = src;
				}
			}

			if (mm->isModulated(dst) || liveValuesCallback) {
				modTimer.startTimerHz(30);

				auto vis = mm->isModulated(dst);
				if (vis != modDepthSlider.isVisible()) {
					modDepthSlider.setVisible(vis);
					resized();
				}

				if (auto depths = mm->getModDepths(dst); !depths.empty()) {
					for (const auto& depth : depths) {
						if (depth.first == currentModSrc)
							modDepthSlider.setValue(
							    depth.second, juce::dontSendNotification);
					}
				} else
					modDepthSlider.setValue(0.0f, juce::dontSendNotification);
			} else {
				modTimer.stopTimer();
				knob.getProperties().remove("modValues");

				if (modDepthSlider.isVisible()) {
					modDepthSlider.setVisible(false);
					resized();
				}
			}

			if (learning && !isMouseButtonDown(true)) {
				modDepth = mm->getModDepth(mm->getLearn(), dst);
				knob.getProperties().set("modDepth", modDepth);
				knob.getProperties().set(
				    "modBipolar", mm->getModBipolarMapping(mm->getLearn(),
				                      gin::ModDstId(parameter->getModIndex())));
				repaint();
			}
		}
	}

	void showModMenu()
	{
		juce::PopupMenu m;
		m.setLookAndFeel(&getLookAndFeel());

		auto &mm = *parameter->getModMatrix();
		for (const auto& src : mm.getModSources(parameter)) {
			bool current{false};
			if (currentModSrc == gin::ModSrcId{-1}) {
				currentModSrc = src;
			}
			if (src == currentModSrc) {
				current = true;
			}
			m.addItem(
			    "Remove: " + mm.getModSrcName(src), true, current, [this, src] {
				    auto dst = gin::ModDstId(parameter->getModIndex());
				    parameter->getModMatrix()->clearModDepth(src, dst);
				    if (auto depths = parameter->getModMatrix()->getModDepths(dst); !depths.empty()) {
					    currentModSrc = depths[0].first;
				    } else {
					    currentModSrc = gin::ModSrcId(-1);
				    }
				    modMatrixChanged();
			    });
		}

		m.addSeparator();

		for (const auto& src : mm.getModSources(parameter)) {
			if (currentModSrc == gin::ModSrcId{-1}) {
				currentModSrc = src;
			}
			bool editing = (src == currentModSrc) ? true : false;
			m.addItem(
			    "Edit: " + mm.getModSrcName(src) + ": " +
			        juce::String(mm.getModDepth(src,
			                         gin::ModDstId(parameter->getModIndex())),
			            3),
			    !editing, editing, [this, src] {
				    currentModSrc = src;
				    modMatrixChanged();
			    });
		}

		m.showMenuAsync({});
	}

	juce::Label name;
	gin::Readout value;
	gin::PluginSlider knob;
	bool learning = false;
	float modDepth = 0.0f;
	bool dragOver = false;
	gin::ModSrcId currentModSrc{-1};

	gin::CoalescedTimer modTimer;
	gin::CoalescedTimer shiftTimer;
	juce::Array<float> modValues;
	std::function<juce::Array<float>()> liveValuesCallback;
	APModulationDepthSlider modDepthSlider;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(APKnob)
};

using PopupMenu = juce::PopupMenu;
class ParameterSelector : public juce::Label
{
public:
	ParameterSelector(APAudioProcessor& p_, gin::ModSrcId src) : proc(p_), macroSrc(src) {
		setEditable(false, true, false);
		setJustificationType(juce::Justification::left);
		setText("Add", juce::dontSendNotification);
		PopupMenu oscsMenu, osc1Menu, osc2Menu, osc3Menu, osc4Menu;
		PopupMenu lfosMenu, lfo1Menu, lfo2Menu, lfo3Menu, lfo4Menu;
		PopupMenu envsMenu, env1Menu, env2Menu, env3Menu, env4Menu;
		PopupMenu filterMenu;
		PopupMenu msegsMenu, mseg1Menu, mseg2Menu, mseg3Menu, mseg4Menu;
		PopupMenu timbreMenu; // equant, pitch, blend, demodmix, demodvol, algo
		PopupMenu globalMenu; // level, squash
		PopupMenu fxLanesMenu;
		PopupMenu fxModulesMenu;
		PopupMenu waveshaperMenu;
		PopupMenu compressorMenu;
		PopupMenu delayMenu;
		PopupMenu reverbMenu;
		PopupMenu mbfilterMenu;
		PopupMenu chorusMenu;
		PopupMenu ringmodMenu;
		PopupMenu ladderMenu;

		ladderMenu.addItem("Cutoff", [this]() { setDest(proc.ladderParams.cutoff); });
		ladderMenu.addItem("Resonance", [this]() { setDest(proc.ladderParams.reso); });
		ladderMenu.addItem("Drive", [this]() { setDest(proc.ladderParams.drive); });
		ladderMenu.addItem("Level", [this]() { setDest(proc.ladderParams.gain); });

		osc1Menu.addItem("OSC1 Coarse", [this]() { setDest(proc.osc1Params.coarse); });
		osc1Menu.addItem("OSC1 Fine", [this]() { setDest(proc.osc1Params.fine); });
		osc1Menu.addItem("OSC1 Volume", [this]() { setDest(proc.osc1Params.volume); });
		osc1Menu.addItem("OSC1 Phase", [this]() { setDest(proc.osc1Params.phase); });

		osc2Menu.addItem("OSC2 Coarse", [this]() { setDest(proc.osc2Params.coarse); });
		osc2Menu.addItem("OSC2 Fine", [this]() { setDest(proc.osc2Params.fine); });
		osc2Menu.addItem("OSC2 Volume", [this]() { setDest(proc.osc2Params.volume); });
		osc2Menu.addItem("OSC2 Phase", [this]() { setDest(proc.osc2Params.phase); });

		osc3Menu.addItem("OSC3 Coarse", [this]() { setDest(proc.osc3Params.coarse); });
		osc3Menu.addItem("OSC3 Fine", [this]() { setDest(proc.osc3Params.fine); });
		osc3Menu.addItem("OSC3 Volume", [this]() { setDest(proc.osc3Params.volume); });
		osc3Menu.addItem("OSC3 Phase", [this]() { setDest(proc.osc3Params.phase); });

		osc4Menu.addItem("OSC4 Coarse", [this]() { setDest(proc.osc4Params.coarse); });
		osc4Menu.addItem("OSC4 Fine", [this]() { setDest(proc.osc4Params.fine); });
		osc4Menu.addItem("OSC4 Volume", [this]() { setDest(proc.osc4Params.volume); });
		osc4Menu.addItem("OSC4 Phase", [this]() { setDest(proc.osc4Params.phase); });

		oscsMenu.addSubMenu("OSC1", osc1Menu);
		oscsMenu.addSubMenu("OSC2", osc2Menu);
		oscsMenu.addSubMenu("OSC3", osc3Menu);
		oscsMenu.addSubMenu("OSC4", osc4Menu);

		lfo1Menu.addItem("LFO1 Rate", [this]() { setDest(proc.lfo1Params.rate); });
		lfo1Menu.addItem("LFO1 Depth", [this]() { setDest(proc.lfo1Params.depth); });
		lfo1Menu.addItem("LFO1 Offset", [this]() { setDest(proc.lfo1Params.offset); });

		lfo2Menu.addItem("LFO2 Rate", [this]() { setDest(proc.lfo2Params.rate); });
		lfo2Menu.addItem("LFO2 Depth", [this]() { setDest(proc.lfo2Params.depth); });
		lfo2Menu.addItem("LFO2 Offset", [this]() { setDest(proc.lfo2Params.offset); });

		lfo3Menu.addItem("LFO3 Rate", [this]() { setDest(proc.lfo3Params.rate); });
		lfo3Menu.addItem("LFO3 Depth", [this]() { setDest(proc.lfo3Params.depth); });
		lfo3Menu.addItem("LFO3 Offset", [this]() { setDest(proc.lfo3Params.offset); });

		lfo4Menu.addItem("LFO4 Rate", [this]() { setDest(proc.lfo4Params.rate); });
		lfo4Menu.addItem("LFO4 Depth", [this]() { setDest(proc.lfo4Params.depth); });
		lfo4Menu.addItem("LFO4 Offset", [this]() { setDest(proc.lfo4Params.offset); });

		lfosMenu.addSubMenu("LFO1", lfo1Menu);
		lfosMenu.addSubMenu("LFO2", lfo2Menu);
		lfosMenu.addSubMenu("LFO3", lfo3Menu);
		lfosMenu.addSubMenu("LFO4", lfo4Menu);

		env1Menu.addItem("ENV1 Attack", [this]() { setDest(proc.env1Params.attack); });
		env1Menu.addItem("ENV1 Decay", [this]() { setDest(proc.env1Params.decay); });
		env1Menu.addItem("ENV1 Sustain", [this]() { setDest(proc.env1Params.sustain); });
		env1Menu.addItem("ENV1 Release", [this]() { setDest(proc.env1Params.release); });
		env1Menu.addItem("ENV1 Time", [this]() { setDest(proc.env1Params.time); });
		env1Menu.addItem("ENV1 Atk Curve", [this]() { setDest(proc.env1Params.acurve); });
		env1Menu.addItem("ENV1 D/R Curve", [this]() { setDest(proc.env1Params.drcurve); });

		env2Menu.addItem("ENV2 Attack", [this]() { setDest(proc.env2Params.attack); });
		env2Menu.addItem("ENV2 Decay", [this]() { setDest(proc.env2Params.decay); });
		env2Menu.addItem("ENV2 Sustain", [this]() { setDest(proc.env2Params.sustain); });
		env2Menu.addItem("ENV2 Release", [this]() { setDest(proc.env2Params.release); });
		env2Menu.addItem("ENV2 Time", [this]() { setDest(proc.env2Params.time); });
		env2Menu.addItem("ENV2 Atk Curve", [this]() { setDest(proc.env2Params.acurve); });
		env2Menu.addItem("ENV2 D/R Curve", [this]() { setDest(proc.env2Params.drcurve); });

		env3Menu.addItem("ENV3 Attack", [this]() { setDest(proc.env3Params.attack); });
		env3Menu.addItem("ENV3 Decay", [this]() { setDest(proc.env3Params.decay); });
		env3Menu.addItem("ENV3 Sustain", [this]() { setDest(proc.env3Params.sustain); });
		env3Menu.addItem("ENV3 Release", [this]() { setDest(proc.env3Params.release); });
		env3Menu.addItem("ENV3 Time", [this]() { setDest(proc.env3Params.time); });
		env3Menu.addItem("ENV3 Atk Curve", [this]() { setDest(proc.env3Params.acurve); });
		env3Menu.addItem("ENV3 D/R Curve", [this]() { setDest(proc.env3Params.drcurve); });

		env4Menu.addItem("ENV4 Attack", [this]() { setDest(proc.env4Params.attack); });
		env4Menu.addItem("ENV4 Decay", [this]() { setDest(proc.env4Params.decay); });
		env4Menu.addItem("ENV4 Sustain", [this]() { setDest(proc.env4Params.sustain); });
		env4Menu.addItem("ENV4 Release", [this]() { setDest(proc.env4Params.release); });
		env4Menu.addItem("ENV4 Time", [this]() { setDest(proc.env4Params.time); });
		env4Menu.addItem("ENV4 Atk Curve", [this]() { setDest(proc.env4Params.acurve); });
		env4Menu.addItem("ENV4 D/R Curve", [this]() { setDest(proc.env4Params.drcurve); });

		envsMenu.addSubMenu("ENV1", env1Menu);
		envsMenu.addSubMenu("ENV2", env2Menu);
		envsMenu.addSubMenu("ENV3", env3Menu);
		envsMenu.addSubMenu("ENV4", env4Menu);
		
		// gin::Parameter::Ptr rate, depth, offset, phase, 

		mseg1Menu.addItem("MSEG1 Rate", [this]() { setDest(proc.mseg1Params.rate); });
		mseg1Menu.addItem("MSEG1 Depth", [this]() { setDest(proc.mseg1Params.depth); });
		mseg1Menu.addItem("MSEG1 Offset", [this]() { setDest(proc.mseg1Params.offset); });

		mseg2Menu.addItem("MSEG2 Rate", [this]() { setDest(proc.mseg2Params.rate); });
		mseg2Menu.addItem("MSEG2 Depth", [this]() { setDest(proc.mseg2Params.depth); });
		mseg2Menu.addItem("MSEG2 Offset", [this]() { setDest(proc.mseg2Params.offset); });

		mseg3Menu.addItem("MSEG3 Rate", [this]() { setDest(proc.mseg3Params.rate); });
		mseg3Menu.addItem("MSEG3 Depth", [this]() { setDest(proc.mseg3Params.depth); });
		mseg3Menu.addItem("MSEG3 Offset", [this]() { setDest(proc.mseg3Params.offset); });

		mseg4Menu.addItem("MSEG4 Rate", [this]() { setDest(proc.mseg4Params.rate); });
		mseg4Menu.addItem("MSEG4 Depth", [this]() { setDest(proc.mseg4Params.depth); });
		mseg4Menu.addItem("MSEG4 Offset", [this]() { setDest(proc.mseg4Params.offset); });

		msegsMenu.addSubMenu("MSEG1", mseg1Menu);
		msegsMenu.addSubMenu("MSEG2", mseg2Menu);
		msegsMenu.addSubMenu("MSEG3", mseg3Menu);
		msegsMenu.addSubMenu("MSEG4", mseg4Menu);

		timbreMenu.addItem("Equant", [this]() { setDest(proc.timbreParams.equant); });
		timbreMenu.addItem("Pitch", [this]() { setDest(proc.timbreParams.pitch); });
		timbreMenu.addItem("Algorithm", [this]() { setDest(proc.timbreParams.algo); });
		timbreMenu.addItem("Demod Mix", [this]() { setDest(proc.timbreParams.demodmix); });
		timbreMenu.addItem("Demod Vol", [this]() { setDest(proc.timbreParams.demodvol); });

		filterMenu.addItem("Cutoff", [this]() { setDest(proc.filterParams.frequency); });
		filterMenu.addItem("Resonance", [this]() { setDest(proc.filterParams.resonance); });

		fxLanesMenu.addItem("FX Lane A Gain", [this]() { setDest(proc.fxOrderParams.laneAGain); });
		fxLanesMenu.addItem("FX Lane A Cutoff", [this]() { setDest(proc.fxOrderParams.laneAFreq); });
		fxLanesMenu.addItem("FX Lane A Resonance", [this]() { setDest(proc.fxOrderParams.laneARes); });
		fxLanesMenu.addItem("FX Lane A Pan", [this]() { setDest(proc.fxOrderParams.laneAPan); });
		fxLanesMenu.addItem("FX Lane B Gain", [this]() { setDest(proc.fxOrderParams.laneBGain); });
		fxLanesMenu.addItem("FX Lane B Cutoff", [this]() { setDest(proc.fxOrderParams.laneBFreq); });
		fxLanesMenu.addItem("FX Lane B Resonance", [this]() { setDest(proc.fxOrderParams.laneBRes); });
		fxLanesMenu.addItem("FX Lane B Pan", [this]() { setDest(proc.fxOrderParams.laneBPan); });

		waveshaperMenu.addItem("Drive", [this]() { setDest(proc.waveshaperParams.drive); });
		waveshaperMenu.addItem("Dry", [this]() { setDest(proc.waveshaperParams.dry); });
		waveshaperMenu.addItem("Wet", [this]() { setDest(proc.waveshaperParams.wet); });
		waveshaperMenu.addItem("High Shelf", [this]() { setDest(proc.waveshaperParams.highshelf); });
		waveshaperMenu.addItem("Low Pass", [this]() { setDest(proc.waveshaperParams.lp); });

		compressorMenu.addItem("Threshold", [this]() { setDest(proc.compressorParams.threshold); });
		compressorMenu.addItem("Ratio", [this]() { setDest(proc.compressorParams.ratio); });
		compressorMenu.addItem("Attack", [this]() { setDest(proc.compressorParams.attack); });
		compressorMenu.addItem("Release", [this]() { setDest(proc.compressorParams.release); });
		compressorMenu.addItem("Input", [this]() { setDest(proc.compressorParams.input); });
		compressorMenu.addItem("Output", [this]() { setDest(proc.compressorParams.output); });

		delayMenu.addItem("Time Left", [this]() { setDest(proc.stereoDelayParams.timeleft); });
		delayMenu.addItem("Time Right", [this]() { setDest(proc.stereoDelayParams.timeright); });
		delayMenu.addItem("Feedback", [this]() { setDest(proc.stereoDelayParams.feedback); });
		delayMenu.addItem("LP Cutoff", [this]() { setDest(proc.stereoDelayParams.cutoff); });
		delayMenu.addItem("Dry", [this]() { setDest(proc.stereoDelayParams.dry); });
		delayMenu.addItem("Wet", [this]() { setDest(proc.stereoDelayParams.wet); });

		chorusMenu.addItem("Rate", [this]() { setDest(proc.chorusParams.rate); });
		chorusMenu.addItem("Depth", [this]() { setDest(proc.chorusParams.depth); });
		chorusMenu.addItem("Delay", [this]() { setDest(proc.chorusParams.delay); });
		chorusMenu.addItem("Feedback", [this]() { setDest(proc.chorusParams.feedback); });
		chorusMenu.addItem("Dry", [this]() { setDest(proc.chorusParams.dry); });
		chorusMenu.addItem("Wet", [this]() { setDest(proc.chorusParams.wet); });

		reverbMenu.addItem("Room Size", [this]() { setDest(proc.reverbParams.size); });
		reverbMenu.addItem("Decay", [this]() { setDest(proc.reverbParams.decay); });
		reverbMenu.addItem("Damping", [this]() { setDest(proc.reverbParams.damping); });
		reverbMenu.addItem("Low pass", [this]() { setDest(proc.reverbParams.lowpass); });
		reverbMenu.addItem("Pre Delay", [this]() { setDest(proc.reverbParams.predelay); });
		reverbMenu.addItem("Dry", [this]() { setDest(proc.reverbParams.dry); });
		reverbMenu.addItem("Wet", [this]() { setDest(proc.reverbParams.wet); });

		mbfilterMenu.addItem("Low Freq", [this]() { setDest(proc.mbfilterParams.lowshelffreq); });
		mbfilterMenu.addItem("Low Gain", [this]() { setDest(proc.mbfilterParams.lowshelfgain); });
		mbfilterMenu.addItem("Low Q", [this]() { setDest(proc.mbfilterParams.lowshelfq); });
		mbfilterMenu.addItem("Peak Frequency", [this]() { setDest(proc.mbfilterParams.peakfreq); });
		mbfilterMenu.addItem("Peak Gain", [this]() { setDest(proc.mbfilterParams.peakgain); });
		mbfilterMenu.addItem("Peak Q", [this]() { setDest(proc.mbfilterParams.peakq); });
		mbfilterMenu.addItem("High Freq", [this]() { setDest(proc.mbfilterParams.highshelffreq); });
		mbfilterMenu.addItem("High Gain", [this]() { setDest(proc.mbfilterParams.highshelfgain); });
		mbfilterMenu.addItem("High Q", [this]() { setDest(proc.mbfilterParams.highshelfq); });

		ringmodMenu.addItem("Mod Freq 1", [this]() { setDest(proc.ringmodParams.modfreq1); });
		ringmodMenu.addItem("Shape 1", [this]() { setDest(proc.ringmodParams.shape1); });
		ringmodMenu.addItem("Mix 1", [this]() { setDest(proc.ringmodParams.mix1); });
		ringmodMenu.addItem("Mod Freq 2", [this]() { setDest(proc.ringmodParams.modfreq2); });
		ringmodMenu.addItem("Shape 2", [this]() { setDest(proc.ringmodParams.shape2); });
		ringmodMenu.addItem("Mix 2", [this]() { setDest(proc.ringmodParams.mix2); });
		ringmodMenu.addItem("Spread", [this]() { setDest(proc.ringmodParams.spread); });
		ringmodMenu.addItem("Low Cut", [this]() { setDest(proc.ringmodParams.lowcut); });
		ringmodMenu.addItem("High Cut", [this]() { setDest(proc.ringmodParams.highcut); });

		fxModulesMenu.addSubMenu("Waveshaper", waveshaperMenu);
		fxModulesMenu.addSubMenu("Compressor", compressorMenu);
		fxModulesMenu.addSubMenu("Delay", delayMenu);
		fxModulesMenu.addSubMenu("Chorus", chorusMenu);
		fxModulesMenu.addSubMenu("Reverb", reverbMenu);
		fxModulesMenu.addSubMenu("MB Filter", mbfilterMenu);
		fxModulesMenu.addSubMenu("Ring Mod", ringmodMenu);
		fxModulesMenu.addSubMenu("Ladder Filter", ladderMenu);
		fxModulesMenu.addItem("Gain", [this]() { setDest(proc.gainParams.gain); });

		m.addSubMenu("Oscillators", oscsMenu);
		m.addSubMenu("Envelopes", envsMenu);
		m.addSubMenu("LFOs", lfosMenu);
		m.addSubMenu("MSEGs", msegsMenu);
		m.addSubMenu("Timbre", timbreMenu);
		m.addSubMenu("Filter", filterMenu);
		m.addSubMenu("FX Modules", fxModulesMenu);
		m.addSubMenu("FX Lanes", fxLanesMenu);
		m.addItem("Global Level", [this]() { setDest(proc.globalParams.level); });
		m.setLookAndFeel(&popupLNF);
		setLookAndFeel(&popupLNF);
	}

	~ParameterSelector() override {
		setLookAndFeel(nullptr);
	}

	void setDest(gin::Parameter::Ptr p) {
		proc.modMatrix.setModDepth(macroSrc, gin::ModDstId(p->getModIndex()), 0.0f);
	}

	void mouseDown(const juce::MouseEvent& /*ev*/) override {
		m.showMenu(PopupMenu::Options().withMousePosition());
	}

	class PopupLNF : public juce::LookAndFeel_V4
	{
	public:
		PopupLNF()
		{
			setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff16171A));
			setColour(juce::PopupMenu::textColourId, juce::Colour(0xff66AACC));
			setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
			setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colours::darkgrey);
			setColour(juce::Label::textColourId, juce::Colour(0xff66AACC));
		}
	};

	PopupLNF popupLNF;
	PopupMenu m;
	APAudioProcessor& proc;
	gin::ModSrcId macroSrc;
};

