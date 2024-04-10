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
#include "PluginProcessor.h"
#include "APModAdditions.h"

class APMacroParamSliderLNF : public gin::CopperLookAndFeel
{
public:
	APMacroParamSliderLNF()
	{
		setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
	}

	void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
		float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
		const juce::Slider::SliderStyle /*style*/, juce::Slider& slider) override
	{
		//const bool isMouseOver = slider.isMouseOverOrDragging();
		auto rc = juce::Rectangle<int>(x, y, width, height);
		rc = rc.withSizeKeepingCentre(width, height);

		// track
		g.setColour(slider.findColour(juce::Slider::trackColourId).withAlpha(0.1f));
		g.fillRect(rc);

		// thumb
		if (slider.isEnabled()) {
			g.setColour(slider.findColour(juce::Slider::thumbColourId).withAlpha(0.85f));
		}
		else {
			g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));
		}
		float t = rc.getY();
		float h = rc.getHeight();
		auto c = rc.getCentreX();
		g.fillRect(juce::Rectangle<float>(0, t, sliderPos, h));

	}
};


class MacrosModMatrixBox : public juce::ListBox,
	private juce::ListBoxModel,
	private gin::ModMatrix::Listener,
	public juce::Timer
{
public:
	MacrosModMatrixBox(gin::Processor& p, gin::ModMatrix& m, gin::ModSrcId macroSrc_, juce::String name_, int dw = 50)
		: proc(p), modMatrix(m), depthWidth(dw), macroSrc(macroSrc_)
	{
		setName(name_);
		setModel(this);
		setRowHeight(18);
		refresh();

		modMatrix.addListener(this);
		startTimerHz(15);
	}

	~MacrosModMatrixBox() override
	{
		modMatrix.removeListener(this);
	}

	void timerCallback() override
	{
		refresh();
	}

private:
	void refresh()
	{
		assignments.clear();

		for (auto param : modMatrix.getModDepths(macroSrc))
			assignments.add(Assignment(macroSrc, modMatrix.getParameter(param.first)));

		updateContent();
		repaint();
	}

	int getNumRows() override
	{
		return assignments.size();
	}

	void paintListBoxItem(int, juce::Graphics&, int, int, bool) override {}

	juce::Component* refreshComponentForRow(int row, bool, juce::Component* c) override
	{
		auto rowComponent = dynamic_cast<Row*>(c);
		if (rowComponent == nullptr)
			rowComponent = new Row(*this);

		rowComponent->update(row);
		return rowComponent;
	}

	void modMatrixChanged() override
	{
		refresh();
	}

	class Row : public juce::Component,
		private juce::Slider::Listener
	{
	public:
		Row(MacrosModMatrixBox& o)
			: owner(o)
		{

			addAndMakeVisible(enableButton);
			addAndMakeVisible(deleteButton);
			addAndMakeVisible(curveButton);
			addAndMakeVisible(biPolarButton);
			addAndMakeVisible(depth);
			addAndMakeVisible(dst);
			addAndMakeVisible(paramSlider);

			paramSlider.setRange(0.0, 1.0);

			depth.setLookAndFeel(&depthLookAndFeel);
			paramSlider.setLookAndFeel(&macroLNF);

			depth.setRange(-1.0, 1.0);
			depth.addListener(this);
			depth.setSliderSnapsToMousePosition(false);
			depth.setMouseDragSensitivity(750);
			depth.setPopupDisplayEnabled(false, false, nullptr);
			// ctrl-click to reset, cmd-click or alt-click for fine adjustment
			depth.setDoubleClickReturnValue(true, 0.0, juce::ModifierKeys::ctrlModifier);
			depth.onValueChange = [this]
				{
					auto& a = owner.assignments.getReference(row);
					auto parameter = a.dst;
					auto dstId = gin::ModDstId(parameter->getModIndex());
					auto range = parameter->getUserRange();
					owner.modMatrix.setModDepth(a.src, dstId, float(depth.getValue()));
				};
			paramSlider.onValueChange = [this]
				{
					auto& a = owner.assignments.getReference(row);
					auto parameter = a.dst;
					auto dstId = gin::ModDstId(parameter->getModIndex());
					auto range = parameter->getUserRange();
					auto userValue = range.convertFrom0to1(float(paramSlider.getValue()));
					parameter->setUserValue(userValue);
					auto legalValue = range.snapToLegalValue(userValue);
					paramSlider.setValue(range.convertTo0to1(legalValue));
				};
			paramSlider.onTextFromValue = [this](double value)
				{
					auto& a = owner.assignments.getReference(row);
					auto parameter = a.dst;
					return parameter->userValueToText(parameter->getUserValue());
				};
			enableButton.onClick = [this]
				{
					if (row >= 0 && row < owner.assignments.size())
					{
						auto& a = owner.assignments.getReference(row);

						auto ev = owner.modMatrix.getModEnable(a.src, gin::ModDstId(a.dst->getModIndex()));
						owner.modMatrix.setModEnable(a.src, gin::ModDstId(a.dst->getModIndex()), !ev);
						enableButton.setToggleState(!ev, juce::dontSendNotification);
						if (ev)
							depth.setEnabled(false);
						else
							depth.setEnabled(true);
					}
				};

			biPolarButton.onClick = [this]
				{
					if (row >= 0 && row < owner.assignments.size())
					{
						auto& a = owner.assignments.getReference(row);

						auto ev = owner.modMatrix.getModBipolarMapping(a.src, gin::ModDstId(a.dst->getModIndex()));
						owner.modMatrix.setModBipolarMapping(a.src, gin::ModDstId(a.dst->getModIndex()), !ev);
						biPolarButton.setToggleState(!ev, juce::dontSendNotification);
					}
				};

			deleteButton.onClick = [this]
				{
					if (row >= 0 && row < owner.assignments.size())
					{
						auto& a = owner.assignments.getReference(row);
						owner.modMatrix.clearModDepth(a.src, gin::ModDstId(a.dst->getModIndex()));
					}
				};

			curveButton.onClick = [this]
				{
					if (row >= 0 && row < owner.assignments.size())
					{
						auto& a = owner.assignments.getReference(row);
						auto f = owner.modMatrix.getModFunction(a.src, gin::ModDstId(a.dst->getModIndex()));

						auto set = [this](gin::ModMatrix::Function func)
							{
								auto& aa = owner.assignments.getReference(row);
								owner.modMatrix.setModFunction(aa.src, gin::ModDstId(aa.dst->getModIndex()), func);
							};

						juce::PopupMenu m;

						m.addItem("Linear", true, f == gin::ModMatrix::Function::linear, [set] { set(gin::ModMatrix::Function::linear); });
						m.addItem("Quadratic In", true, f == gin::ModMatrix::Function::quadraticIn, [set] { set(gin::ModMatrix::Function::quadraticIn); });
						m.addItem("Quadratic In/Out", true, f == gin::ModMatrix::Function::quadraticInOut, [set] { set(gin::ModMatrix::Function::quadraticInOut); });
						m.addItem("Quadratic Out", true, f == gin::ModMatrix::Function::quadraticOut, [set] { set(gin::ModMatrix::Function::quadraticOut); });
						m.addItem("Sine In", true, f == gin::ModMatrix::Function::sineIn, [set] { set(gin::ModMatrix::Function::sineIn); });
						m.addItem("Sine In Out", true, f == gin::ModMatrix::Function::sineInOut, [set] { set(gin::ModMatrix::Function::sineInOut); });
						m.addItem("Sine Out", true, f == gin::ModMatrix::Function::sineOut, [set] { set(gin::ModMatrix::Function::sineOut); });
						m.addItem("Exponential In", true, f == gin::ModMatrix::Function::exponentialIn, [set] { set(gin::ModMatrix::Function::exponentialIn); });
						m.addItem("Exponential In/Out", true, f == gin::ModMatrix::Function::exponentialInOut, [set] { set(gin::ModMatrix::Function::exponentialInOut); });
						m.addItem("Exponential Out", true, f == gin::ModMatrix::Function::exponentialOut, [set] { set(gin::ModMatrix::Function::exponentialOut); });
						m.addSeparator();
						m.addItem("Inv Linear", true, f == gin::ModMatrix::Function::invLinear, [set] { set(gin::ModMatrix::Function::invLinear); });
						m.addItem("Inv Quadratic In", true, f == gin::ModMatrix::Function::invQuadraticIn, [set] { set(gin::ModMatrix::Function::invQuadraticIn); });
						m.addItem("Inv Quadratic In/Out", true, f == gin::ModMatrix::Function::invQuadraticInOut, [set] { set(gin::ModMatrix::Function::invQuadraticInOut); });
						m.addItem("Inv Quadratic Out", true, f == gin::ModMatrix::Function::invQuadraticOut, [set] { set(gin::ModMatrix::Function::invQuadraticOut); });
						m.addItem("Inv Sine In", true, f == gin::ModMatrix::Function::invSineIn, [set] { set(gin::ModMatrix::Function::invSineIn); });
						m.addItem("Inv Sine In/Out", true, f == gin::ModMatrix::Function::invSineInOut, [set] { set(gin::ModMatrix::Function::invSineInOut); });
						m.addItem("Inv Sine Out", true, f == gin::ModMatrix::Function::invSineOut, [set] { set(gin::ModMatrix::Function::invSineOut); });
						m.addItem("Inv Exponential In", true, f == gin::ModMatrix::Function::invExponentialIn, [set] { set(gin::ModMatrix::Function::invExponentialIn); });
						m.addItem("Inv Exponential In/Out", true, f == gin::ModMatrix::Function::invExponentialInOut, [set] { set(gin::ModMatrix::Function::invExponentialInOut); });
						m.addItem("Inv Exponential Out", true, f == gin::ModMatrix::Function::invExponentialOut, [set] { set(gin::ModMatrix::Function::invExponentialOut); });

						m.setLookAndFeel(&popupLNF);
						m.showMenuAsync({});
					}
				};
		}

		~Row() override
		{
			depth.setLookAndFeel(nullptr);
		}

		void sliderValueChanged(juce::Slider*) override
		{
			if (row >= 0 && row < owner.assignments.size())
			{
				auto& a = owner.assignments.getReference(row);
				owner.modMatrix.setModDepth(a.src, gin::ModDstId(a.dst->getModIndex()), (float)depth.getValue());
			}
		}

		void update(int idx)
		{
			row = idx;

			if (idx >= 0 && idx < owner.assignments.size())
			{
				auto& a = owner.assignments.getReference(idx);
				dst.setText(a.dst->getName(100), juce::dontSendNotification);

				
				auto ev = owner.modMatrix.getModEnable(a.src, gin::ModDstId(a.dst->getModIndex()));
				enableButton.setToggleState(ev, juce::dontSendNotification);

				auto b = owner.modMatrix.getModBipolarMapping(a.src, gin::ModDstId(a.dst->getModIndex()));
				biPolarButton.setToggleState(b, juce::dontSendNotification);

				depth.setValue(owner.modMatrix.getModDepth(a.src, gin::ModDstId(a.dst->getModIndex())), juce::dontSendNotification);
				float paramUserValue = a.dst->getUserValue();
				float paramValue = a.dst->getUserRange().convertTo0to1(paramUserValue);
				paramSlider.setValue(paramValue);
				curveButton.setCurve(owner.modMatrix.getModFunction(a.src, gin::ModDstId(a.dst->getModIndex())));
			}
			else
			{
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
			dst.setBounds(rc.removeFromLeft(w));
			paramSlider.setBounds(rc);
		}

		class PopupLNF : public juce::LookAndFeel_V4
		{
		public:
			PopupLNF()
			{
				setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff16171A));
				setColour(juce::PopupMenu::textColourId, juce::Colour(0xffE6E6E9));
				setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
				setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colours::darkgrey);
			}
		};

		PopupLNF popupLNF;

		MacrosModMatrixBox& owner;
		int row = 0;

		class APDepthSlider : public juce::Slider
		{
		public:
			APDepthSlider() : juce::Slider() //, juce::Slider::NoTextBox)
			{
				setSliderStyle(juce::Slider::SliderStyle::LinearBar);
				setColour(juce::Slider::textBoxTextColourId, juce::Colours::transparentBlack);
			}

			void mouseEnter(const juce::MouseEvent& ev) override
			{
				juce::Slider::mouseEnter(ev);
				setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
				setColour(juce::Slider::thumbColourId, juce::Colours::darkgrey);
			}

			void mouseExit(const juce::MouseEvent& ev) override
			{
				juce::Slider::mouseExit(ev);
				setColour(juce::Slider::textBoxTextColourId, juce::Colours::transparentBlack);
				setColour(juce::Slider::thumbColourId, juce::Colour(0xffCC8866));
			}

			juce::String getTextFromValue(double value) override
			{
				if(onTextFromValue)
					return onTextFromValue(value);
				return juce::String(value, 3);
			}

			std::function<juce::String(double)> onTextFromValue;
		};

		class APModDepthLookAndFeel : public gin::CopperLookAndFeel
		{
		public:
			APModDepthLookAndFeel()
			{
				setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
			}

			void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
				float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
				const juce::Slider::SliderStyle /*style*/, juce::Slider& slider) override
			{
				//const bool isMouseOver = slider.isMouseOverOrDragging();
				auto rc = juce::Rectangle<int>(x, y, width, height);
				rc = rc.withSizeKeepingCentre(width, height);

				// track
				g.setColour(slider.findColour(juce::Slider::trackColourId).withAlpha(0.1f));
				g.fillRect(rc);

				// thumb
				if (slider.isEnabled()) {
					g.setColour(slider.findColour(juce::Slider::thumbColourId).withAlpha(0.85f));
				}
				else {
					g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));
				}
				float t = rc.getY();
				float h = rc.getHeight();
				auto c = rc.getCentreX();
				if (sliderPos < c)
					g.fillRect(juce::Rectangle<float>(sliderPos, t, c - sliderPos, h));
				else
					g.fillRect(juce::Rectangle<float>(float(c), t, sliderPos - c, h));

			}
		};

		APDepthSlider depth;
		APDepthSlider paramSlider;
		APModDepthLookAndFeel depthLookAndFeel;
		APMacroParamSliderLNF macroLNF;

		//juce::Label src;
		juce::Label dst;

		gin::ModCurveButton curveButton;
		gin::SVGButton biPolarButton{ "bi", gin::Assets::bipolar };

		gin::SVGButton enableButton{ "enable", gin::Assets::power, 1 };
		gin::SVGButton deleteButton{ "delete", gin::Assets::del };
	};

	struct Assignment
	{
		gin::ModSrcId src = {};
		gin::Parameter* dst = nullptr;
	};

	gin::Processor& proc;
	gin::ModMatrix& modMatrix;
	juce::Array<Assignment> assignments;
	int depthWidth = 50;
	gin::ModSrcId macroSrc;
};



class ParameterSelector : public juce::Label
{
public:
	ParameterSelector(APAudioProcessor& p_, gin::ModSrcId src) : proc(p_), macroSrc(src) {
		setEditable(false, true, false);
		setJustificationType(Justification::left);
		setText("Add", dontSendNotification);

		// build hierarchical popup menu of parameters
		PopupMenu oscsMenu, osc1Menu, osc2Menu, osc3Menu, osc4Menu;
		PopupMenu lfosMenu, lfo1Menu, lfo2Menu, lfo3Menu, lfo4Menu;
		PopupMenu envsMenu, env1Menu, env2Menu, env3Menu, env4Menu;
		PopupMenu filterMenu;
		PopupMenu msegsMenu, mseg1Menu, mseg2Menu, mseg3Menu, mseg4Menu;
		PopupMenu timbreMenu; // equant, pitch, blend, demodmix, demodvol, algo
		PopupMenu globalMenu; // level, squash
		PopupMenu fxMenu;
		PopupMenu waveshaperMenu;
		PopupMenu compressorMenu;
		PopupMenu delayMenu;
		PopupMenu reverbMenu;
		PopupMenu mbeqMenu;
		PopupMenu chorusMenu;
		PopupMenu ringmodMenu;

		osc1Menu.addItem("OSC1 Coarse", [this]() { setDest(proc.osc1Params.coarse); });
		osc1Menu.addItem("OSC1 Fine", [this]() { setDest(proc.osc1Params.fine); });
		osc1Menu.addItem("OSC1 Volume", [this]() { setDest(proc.osc1Params.volume); });
		osc1Menu.addItem("OSC1 Tones", [this]() { setDest(proc.osc1Params.tones); });
		osc1Menu.addItem("OSC1 Phase", [this]() { setDest(proc.osc1Params.phase); });
		osc1Menu.addItem("OSC1 Detune", [this]() { setDest(proc.osc1Params.detune); });
		osc1Menu.addItem("OSC1 Spread", [this]() { setDest(proc.osc1Params.spread); });
		osc1Menu.addItem("OSC1 Pan", [this]() { setDest(proc.osc1Params.pan); });

		osc2Menu.addItem("OSC2 Coarse", [this]() { setDest(proc.osc2Params.coarse); });
		osc2Menu.addItem("OSC2 Fine", [this]() { setDest(proc.osc2Params.fine); });
		osc2Menu.addItem("OSC2 Volume", [this]() { setDest(proc.osc2Params.volume); });
		osc2Menu.addItem("OSC2 Tones", [this]() { setDest(proc.osc2Params.tones); });
		osc2Menu.addItem("OSC2 Phase", [this]() { setDest(proc.osc2Params.phase); });
		osc2Menu.addItem("OSC2 Detune", [this]() { setDest(proc.osc2Params.detune); });
		osc2Menu.addItem("OSC2 Spread", [this]() { setDest(proc.osc2Params.spread); });
		osc2Menu.addItem("OSC2 Pan", [this]() { setDest(proc.osc2Params.pan); });

		osc3Menu.addItem("OSC3 Coarse", [this]() { setDest(proc.osc3Params.coarse); });
		osc3Menu.addItem("OSC3 Fine", [this]() { setDest(proc.osc3Params.fine); });
		osc3Menu.addItem("OSC3 Volume", [this]() { setDest(proc.osc3Params.volume); });
		osc3Menu.addItem("OSC3 Tones", [this]() { setDest(proc.osc3Params.tones); });
		osc3Menu.addItem("OSC3 Phase", [this]() { setDest(proc.osc3Params.phase); });
		osc3Menu.addItem("OSC3 Detune", [this]() { setDest(proc.osc3Params.detune); });
		osc3Menu.addItem("OSC3 Spread", [this]() { setDest(proc.osc3Params.spread); });
		osc3Menu.addItem("OSC3 Pan", [this]() { setDest(proc.osc3Params.pan); });

		osc4Menu.addItem("OSC4 Coarse", [this]() { setDest(proc.osc4Params.coarse); });
		osc4Menu.addItem("OSC4 Fine", [this]() { setDest(proc.osc4Params.fine); });
		osc4Menu.addItem("OSC4 Volume", [this]() { setDest(proc.osc4Params.volume); });
		osc4Menu.addItem("OSC4 Tones", [this]() { setDest(proc.osc4Params.tones); });
		osc4Menu.addItem("OSC4 Phase", [this]() { setDest(proc.osc4Params.phase); });
		osc4Menu.addItem("OSC4 Detune", [this]() { setDest(proc.osc4Params.detune); });
		osc4Menu.addItem("OSC4 Spread", [this]() { setDest(proc.osc4Params.spread); });
		osc4Menu.addItem("OSC4 Pan", [this]() { setDest(proc.osc4Params.pan); });

		oscsMenu.addSubMenu("OSC1", osc1Menu);
		oscsMenu.addSubMenu("OSC2", osc2Menu);
		oscsMenu.addSubMenu("OSC3", osc3Menu);
		oscsMenu.addSubMenu("OSC4", osc4Menu);

		lfo1Menu.addItem("LFO1 Rate", [this]() { setDest(proc.lfo1Params.rate); });
		lfo1Menu.addItem("LFO1 Beats", [this]() { setDest(proc.lfo1Params.beat); });
		lfo1Menu.addItem("LFO1 Depth", [this]() { setDest(proc.lfo1Params.depth); });
		lfo1Menu.addItem("LFO1 Offset", [this]() { setDest(proc.lfo1Params.offset); });

		lfo2Menu.addItem("LFO2 Rate", [this]() { setDest(proc.lfo2Params.rate); });
		lfo2Menu.addItem("LFO2 Beats", [this]() { setDest(proc.lfo2Params.beat); });
		lfo2Menu.addItem("LFO2 Depth", [this]() { setDest(proc.lfo2Params.depth); });
		lfo2Menu.addItem("LFO2 Offset", [this]() { setDest(proc.lfo2Params.offset); });

		lfo3Menu.addItem("LFO3 Rate", [this]() { setDest(proc.lfo3Params.rate); });
		lfo3Menu.addItem("LFO3 Beats", [this]() { setDest(proc.lfo3Params.beat); });
		lfo3Menu.addItem("LFO3 Depth", [this]() { setDest(proc.lfo3Params.depth); });
		lfo3Menu.addItem("LFO3 Offset", [this]() { setDest(proc.lfo3Params.offset); });

		lfo4Menu.addItem("LFO4 Rate", [this]() { setDest(proc.lfo4Params.rate); });
		lfo4Menu.addItem("LFO4 Beats", [this]() { setDest(proc.lfo4Params.beat); });
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
		env1Menu.addItem("ENV1 Beats", [this]() { setDest(proc.env1Params.duration); });
		env1Menu.addItem("ENV1 Atk Curve", [this]() { setDest(proc.env1Params.acurve); });
		env1Menu.addItem("ENV1 D/R Curve", [this]() { setDest(proc.env1Params.drcurve); });

		env2Menu.addItem("ENV2 Attack", [this]() { setDest(proc.env2Params.attack); });
		env2Menu.addItem("ENV2 Decay", [this]() { setDest(proc.env2Params.decay); });
		env2Menu.addItem("ENV2 Sustain", [this]() { setDest(proc.env2Params.sustain); });
		env2Menu.addItem("ENV2 Release", [this]() { setDest(proc.env2Params.release); });
		env2Menu.addItem("ENV2 Time", [this]() { setDest(proc.env2Params.time); });
		env2Menu.addItem("ENV2 Beats", [this]() { setDest(proc.env2Params.duration); });
		env2Menu.addItem("ENV2 Atk Curve", [this]() { setDest(proc.env2Params.acurve); });
		env2Menu.addItem("ENV2 D/R Curve", [this]() { setDest(proc.env2Params.drcurve); });

		env3Menu.addItem("ENV3 Attack", [this]() { setDest(proc.env3Params.attack); });
		env3Menu.addItem("ENV3 Decay", [this]() { setDest(proc.env3Params.decay); });
		env3Menu.addItem("ENV3 Sustain", [this]() { setDest(proc.env3Params.sustain); });
		env3Menu.addItem("ENV3 Release", [this]() { setDest(proc.env3Params.release); });
		env3Menu.addItem("ENV3 Time", [this]() { setDest(proc.env3Params.time); });
		env3Menu.addItem("ENV3 Beats", [this]() { setDest(proc.env3Params.duration); });
		env3Menu.addItem("ENV3 Atk Curve", [this]() { setDest(proc.env3Params.acurve); });
		env3Menu.addItem("ENV3 D/R Curve", [this]() { setDest(proc.env3Params.drcurve); });

		env4Menu.addItem("ENV4 Attack", [this]() { setDest(proc.env4Params.attack); });
		env4Menu.addItem("ENV4 Decay", [this]() { setDest(proc.env4Params.decay); });
		env4Menu.addItem("ENV4 Sustain", [this]() { setDest(proc.env4Params.sustain); });
		env4Menu.addItem("ENV4 Release", [this]() { setDest(proc.env4Params.release); });
		env4Menu.addItem("ENV4 Time", [this]() { setDest(proc.env4Params.time); });
		env4Menu.addItem("ENV4 Beats", [this]() { setDest(proc.env4Params.duration); });
		env4Menu.addItem("ENV4 Atk Curve", [this]() { setDest(proc.env4Params.acurve); });
		env4Menu.addItem("ENV4 D/R Curve", [this]() { setDest(proc.env4Params.drcurve); });

		envsMenu.addSubMenu("ENV1", env1Menu);
		envsMenu.addSubMenu("ENV2", env2Menu);
		envsMenu.addSubMenu("ENV3", env3Menu);
		envsMenu.addSubMenu("ENV4", env4Menu);


		m.addSubMenu("Oscillators", oscsMenu);
		m.addSubMenu("Envelopes", envsMenu);
		m.addSubMenu("LFOs", lfosMenu);

	}

	void setDest(gin::Parameter::Ptr p) {
		proc.modMatrix.setModDepth(macroSrc, gin::ModDstId(p->getModIndex()), 0.0f);
	}

	void mouseDown(const MouseEvent& e) override {
		m.show();
	}

	PopupMenu m;
	APAudioProcessor& proc;
	gin::ModSrcId macroSrc;
	
};


class MacrosMatrixBox : public gin::ParamBox
{
public:
	MacrosMatrixBox(const juce::String& name, APAudioProcessor& proc_, gin::ModSrcId macroSrc_, gin::Parameter::Ptr macroDst_)
		: gin::ParamBox(name), proc(proc_), macroSrc(macroSrc_)
	{
		setName("mtx");
		addControl(knob = new APKnob(macroDst_), 0, 0, 1.5, 1.5);
		addAndMakeVisible(paramSelector);
		addControl(new MacrosModMatrixBox(proc, proc.modMatrix, macroSrc, name, 70), 2, 0, 6, 4);
	}

	void resized() override {
		ParamBox::resized();
		knob->setBounds(0, 23, 84, 105);
		paramSelector.setBounds(5, 0, 55, 23);
	}

	gin::ParamComponent::Ptr knob;
	APAudioProcessor& proc;
	gin::ModSrcId macroSrc;
	ParameterSelector paramSelector{ proc, macroSrc };
};




// TODO: 
// Row should show live values?
