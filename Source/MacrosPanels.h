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

class AuxBox : public gin::ParamBox
{
public:
	AuxBox(const juce::String& name, APAudioProcessor& proc_)
		: gin::ParamBox(name), proc(proc_)
	{
		
		setName("aux");
		addEnable(proc.auxParams.enable);
		
		addControl(wave = new gin::Select(proc.auxParams.wave), 0, 0);
		addControl(env = new gin::Select(proc.auxParams.env), 0, 0);
		addControl(new APKnob(proc.auxParams.octave), 1, 0);
		addControl(new APKnob(proc.auxParams.volume), 2, 0);
		addControl(new APKnob(proc.auxParams.detune), 3, 0);
		addControl(new APKnob(proc.auxParams.spread), 4, 0);

		addControl(prefx = new gin::Select(proc.auxParams.prefx), 0, 1);
		addControl(filtertype = new gin::Select(proc.auxParams.filtertype), 0, 1);
		addControl(new APKnob(proc.auxParams.filtercutoff), 1, 1);
		addControl(new APKnob(proc.auxParams.filterres), 2, 1);
		addControl(new APKnob(proc.auxParams.filterkeytrack), 3, 1);
		addControl(new gin::Switch(proc.auxParams.ignorepb), 4, 1);
	}

	void resized() override
	{
		ParamBox::resized();
		wave->setBounds(0, 23, 56, 35);
		env->setBounds(0, 58, 56, 35);
		prefx->setBounds(0, 93, 56, 35);
		filtertype->setBounds(0, 128, 56, 35);
	}

	APAudioProcessor& proc;
	gin::ParamComponent::Ptr wave, env, prefx, filtertype;
};

class Waveform : public juce::Component
{
public:
	Waveform(APAudioProcessor& p) : proc(p) {}
	void paint(juce::Graphics& g) override
	{
		if (proc.sampler.sound.data != nullptr) {
			g.setColour(juce::Colour(0xffCC8866).darker(0.5f));
			int loopstart = proc.samplerParams.loopstart->getUserValue() * getWidth();
			int loopend = proc.samplerParams.loopend->getUserValue() * getWidth();
			g.fillRect(loopstart, 0, loopend - loopstart, getHeight());
			g.setColour(juce::Colours::grey);
			g.drawLine(loopstart, 0, loopstart, getHeight(), 1);
			g.drawLine(loopend, 0, loopend, getHeight(), 1);
		}
		g.setColour(juce::Colours::darkgrey);
		g.drawRect(getLocalBounds(), 1);
		auto& audio = proc.sampler.sound.data;
		auto stride = proc.sampler.sound.length * (proc.samplerParams.end->getValue() - proc.samplerParams.start->getValue()) / getWidth();
		auto name = proc.sampler.sound.name;
		if (stride < 1) { return; }
		auto buffer = audio->getReadPointer(0);
		if (shouldRedraw) {
			audioPoints.clear();
			smoothed.clear();
			auto length = proc.sampler.sound.length;
			int startSample = int(proc.samplerParams.start->getUserValue() * length);
			int endSample = int(proc.samplerParams.end->getUserValue() * length);
			for (int sample = startSample; sample < endSample; sample += stride)
			{
				audioPoints.push_back(buffer[sample]);
			}
			smoothed.push_back(audioPoints[0]);
			for (int i = 1; i < audioPoints.size() - 1; i++)
			{
				smoothed.push_back(audioPoints[i - 1] * 0.333f + audioPoints[i] * 0.333f + audioPoints[i+1] * 0.333f);
			}
			smoothed.push_back(audioPoints[audioPoints.size() - 1]);
			shouldRedraw = false;
		}
		juce::Path p;
		p.clear();
		p.startNewSubPath(0, getHeight() / 2);
		float max{ 0 }, min{ 0 };
		for (int sample = 0; sample < smoothed.size(); sample++)
		{
			if (smoothed[sample] > max) { max = smoothed[sample]; }
			if (smoothed[sample] < min) { min = smoothed[sample]; }
		}
		if (std::abs(min) > max) { max = std::abs(min); }
		else { min = -max; }
		min = jlimit(-1.f, -.3f, min);
		max = jlimit(.3f, 1.f, max);
		for (int sample = 0; sample < smoothed.size(); sample++) {
			p.lineTo(sample, jmap(smoothed[sample], min, max, static_cast<float>(getHeight()), 0.f));
		}

		g.setColour(juce::Colours::white);
		g.strokePath(p, juce::PathStrokeType(1.0f));
	}
	APAudioProcessor& proc;
	std::vector<float> audioPoints, smoothed;
	bool shouldRedraw{ true };
};

class SamplerBox : public gin::ParamBox
{
public:
	SamplerBox(const juce::String& name, APAudioProcessor& proc_)
		: gin::ParamBox(name), proc(proc_), waveform(proc_) {
		addEnable(proc.samplerParams.enable);
		addControl(new APKnob(proc.samplerParams.volume), 0, 0);
		addControl(new gin::Select(proc.samplerParams.loop), 1, 0);
		addControl(new APKnob(proc.samplerParams.key), 2, 0);
		addControl(new APKnob(proc.samplerParams.start), 3, 0);
		addControl(new APKnob(proc.samplerParams.end), 4, 0);
		addControl(new APKnob(proc.samplerParams.loopstart), 0, 1);
		addControl(new APKnob(proc.samplerParams.loopend), 1, 1);
		addAndMakeVisible(waveform);
		addAndMakeVisible(loadButton);
		loadButton.onClick = [this] { chooseFile(); };
	}

	void chooseFile() {
		chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
			[this](const juce::FileChooser& fc)
			{
				auto file = fc.getResult();
				if (!file.existsAsFile()) { return; }
				proc.loadSample(file.getFullPathName());
				waveform.shouldRedraw = true;
				waveform.repaint();
			});
	};

	void resized() override {
		ParamBox::resized();
		waveform.setBounds(0, 140 + 23, getWidth(), 93);
		loadButton.setBounds(getWidth() - 55, 0, 55, 23);
	}

	Waveform waveform;
	APAudioProcessor& proc;
	TextButton loadButton{ "Load" };
	std::unique_ptr<juce::FileChooser> chooser = std::make_unique<juce::FileChooser>("Select file",
		juce::File{}, "*.wav,*.aif,*.mp3,*.aif,*.ogg,*.flac");
};


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
		g.fillRect(juce::Rectangle<float>(0, t, sliderPos, h));

	}
};

class MIDILearnButton : public juce::Label
{
public:
	MIDILearnButton(APAudioProcessor& p) : proc(p) {
		setEditable(false, false, false);
		setJustificationType(Justification::left);
		setText("Learn", dontSendNotification);
		setLookAndFeel(&midilearnLNF);
	}

	~MIDILearnButton() override
	{
		setLookAndFeel(nullptr);
	}

	void mouseDown(const juce::MouseEvent& /*ev*/) override
	{
        if (thisMacroNumber == 1) {
            if (proc.macroParams.macro1cc->getUserValue() > 0.f) { return; }
        }
        if (thisMacroNumber == 2) {
            if (proc.macroParams.macro2cc->getUserValue() > 0.f) { return; }
        }
        if (thisMacroNumber == 3) {
            if (proc.macroParams.macro3cc->getUserValue() > 0.f) { return; }
        }
        if (thisMacroNumber == 4) {
            if (proc.macroParams.macro4cc->getUserValue() > 0.f) { return; }
        }
		learning = !learning;
		if (learning) {
			proc.macroParams.learning->setUserValue(static_cast<float>(thisMacroNumber));
			setText("Learning", dontSendNotification);
		}
		else {
			proc.macroParams.learning->setValue(0.0f);
			setText("Learn", dontSendNotification);
		}
	}

	void setMacroNumber(int n)
	{
		thisMacroNumber = n;
	}
	
	void setCCString(const juce::String& s)
	{
		currentAssignment = s;
		setText(s, dontSendNotification);
	}

    void setLearning(bool shouldLearn) {
        learning = shouldLearn;
    }
    

	class MIDILearnLNF : public APLNF
	{
	public:
		MIDILearnLNF()
		{
		}

		void drawLabel(juce::Graphics& g, juce::Label& label) override
		{
			auto rc = label.getLocalBounds();
			g.setColour(juce::Colours::white);
			g.setFont(regularFont.withHeight(20.0f));
			g.drawText(label.getText(), rc, juce::Justification::centred);
		}
        
        juce::Font regularFont{ juce::Typeface::createSystemTypefaceFor(BinaryData::latoregular_otf, BinaryData::latoregular_otfSize) };
	} midilearnLNF;
    
	juce::String currentAssignment;
	APAudioProcessor& proc;
	bool learning{ false };
	int mididCC{ -1 }, thisMacroNumber{ 0 };
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

        for (auto param : modMatrix.getModDepths(macroSrc)) {
            Assignment assignment;
            assignment.src = macroSrc;
            assignment.dst = modMatrix.getParameter(param.first);
            assignments.add(Assignment(assignment));
        }
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
					if (owner.assignments.isEmpty()) { return; }
					auto& a = owner.assignments.getReference(row);
					auto parameter = a.dst;
//					if (!owner.modMatrix.isModulated(gin::ModDstId(parameter->getModIndex()))) { return; }
					auto range = parameter->getUserRange();
					auto userValue = range.convertFrom0to1(float(paramSlider.getValue()));
					parameter->setUserValue(userValue);
					auto legalValue = range.snapToLegalValue(userValue);
					paramSlider.setValue(range.convertTo0to1(legalValue));
				};

			paramSlider.onTextFromValue = [this](double /*value*/)
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

			void mouseDoubleClick(const juce::MouseEvent& ev) override
			{
				if (onDoubleClick)
					return onDoubleClick();
				juce::Slider::mouseDoubleClick(ev);
			}

			std::function<juce::String(double)> onTextFromValue;
			std::function<void()> onDoubleClick;
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






class MacrosMatrixBox : public gin::ParamBox
{
public:
	MacrosMatrixBox(
		const juce::String& name, 
		APAudioProcessor& proc_, 
		gin::ModSrcId macroSrc_, 
		gin::Parameter::Ptr macroDst_, 
		gin::Parameter::Ptr macroCC_, 
		int macroNumber)
		: gin::ParamBox(name), proc(proc_), macroSrc(macroSrc_), macroCC(macroCC_)
	{
		setName("mtx");
		addControl(knob = new APKnob(macroDst_), 0, 0, 1.5, 1.5);
		addAndMakeVisible(paramSelector);
		addAndMakeVisible(midiLearnButton);
        //midiLearnButton.setFont(20.f);
		addControl(new MacrosModMatrixBox(proc, proc.modMatrix, macroSrc, name, 70), 2, 0, 6, 4);
		midiLearnButton.setMacroNumber(macroNumber);
		macroCC->addListener(this);
        clearButton.onClick = [this]() { cancelAssignment(); };
        addChildComponent(clearButton);
        valueUpdated(macroCC);
		addAndMakeVisible(clearAllButton);
		clearAllButton.onClick = [this]() { clearAll(); };
	}

	void clearAll() {
		auto& params = proc.getPluginParameters();
		for (auto* param : params) {
			if (param->getModIndex() == -1) continue;
			if (proc.modMatrix.isModulated(gin::ModDstId(param->getModIndex()))) {
				if (proc.modMatrix.getModSources(param).contains(macroSrc))
					proc.modMatrix.clearModDepth(macroSrc, gin::ModDstId(param->getModIndex()));
			}
		}
	}

	void valueUpdated(gin::Parameter* p) override {
		auto ccValue = macroCC->getUserValueInt();
		if (ccValue >= 0) {
			midiLearnButton.setCCString("CC " + macroCC->getUserValueText());
            clearButton.setVisible(true);
		}
        else {
            midiLearnButton.setCCString("Learn");
            midiLearnButton.setLearning(false);
            clearButton.setVisible(false);
        }
	}

	void resized() override {
		ParamBox::resized();
		knob->setBounds(0, 23, 84, 105);
		midiLearnButton.setBounds(0, 128, 84, 55);
        clearButton.setBounds(0, 183, 84, 25);
		paramSelector.setBounds(5, 0, 55, 23);
		clearAllButton.setBounds(getWidth() - 55, 0, 55, 25);
	}

    void cancelAssignment() {
        macroCC->setUserValue(-1.f);
        midiLearnButton.setCCString("Learn");
		midiLearnButton.setLearning(false);
        clearButton.setVisible(false);
    }

	void disableLearning() {
		midiLearnButton.setLearning(false);
		midiLearnButton.setCCString("Learn");
	}
    

	TextButton clearAllButton{ "Clear All", "Clear All" };
	gin::ParamComponent::Ptr knob;
	APAudioProcessor& proc;
	gin::ModSrcId macroSrc;
	gin::Parameter::Ptr macroCC;
	ParameterSelector paramSelector{ proc, macroSrc };
	MIDILearnButton midiLearnButton{ proc };
    TextButton clearButton{"Clear", "Clear"};
};

//==============================================================================
class MacrosModBox : public gin::ParamBox
{
public:
	MacrosModBox(const juce::String& name, APAudioProcessor& proc_)
		: gin::ParamBox(name), proc(proc_)
	{
		setName("msegmod");
		setTitle("mod sources");
		addControl(srclist = new gin::ModSrcListBox(proc.modMatrix), 0, 0, 5, 3);
		srclist->setRowHeight(20);
	}
	gin::ModSrcListBox* srclist;
	APAudioProcessor& proc;
};