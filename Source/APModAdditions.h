#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <numbers>
#include "BinaryData.h"
#include "Fonts.h"

using std::numbers::pi;

//==============================================================================
/** A button for the modulation destination
*/
class APModulationDepthSlider : public juce::Slider
{
public:
    APModulationDepthSlider() : juce::Slider(RotaryHorizontalVerticalDrag, NoTextBox)
    {

    }

    ~APModulationDepthSlider() override
    {
    }

    juce::String getTextFromValue(double value) override
    {
        if (onTextFromValue)
            return onTextFromValue (value);
        return juce::String(value, 2);
    }

    std::function<void()> onClick;
    std::function<juce::String(double)> onTextFromValue;

private:
    void paint(juce::Graphics& g) override
    {
        auto c = juce::Colours::white.withAlpha(0.4f);
        g.setColour(c);

        auto rc = getLocalBounds().toFloat().reduced(1.5f);
        g.fillEllipse(rc);

        if (auto v = float(getValue()); v > 0.0f || v < 0.0f)
        {
            g.setColour(findColour(gin::PluginLookAndFeel::accentColourId, true).withAlpha(0.9f));

            juce::Path p;
            p.addPieSegment(rc, 0.0f, juce::MathConstants<float>::pi * 2 * v, 0.0f);

            g.fillPath(p);
        }
    }

    void mouseUp(const juce::MouseEvent& ev) override
    {
        if (ev.mouseWasClicked() && ev.mods.isPopupMenu() && onClick)
            onClick();
    }
};


//==============================================================================
/** A list box of all assigned
*/
class APModMatrixBox : public juce::ListBox,
    private juce::ListBoxModel,
    private gin::ModMatrix::Listener
{
public:
    APModMatrixBox(gin::Processor& p, gin::ModMatrix& m, int dw = 50)
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
    }

private:
    void refresh()
    {
        assignments.clear();

        auto& params = proc.getPluginParameters();
        for (gin::Parameter* p : params)
            for (auto s : modMatrix.getModSources(p))
                assignments.add({ s, p });

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
        Row(APModMatrixBox& o)
            : owner(o)
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
            depth.setDoubleClickReturnValue(true, 0.0, juce::ModifierKeys::ctrlModifier);
            depth.onValueChange = [this]
                {
                    auto& a = owner.assignments.getReference(row);
                    auto parameter = a.dst;
                    auto dstId = gin::ModDstId(parameter->getModIndex());
                    auto range = parameter->getUserRange();
                    owner.modMatrix.setModDepth(a.src, dstId, float(depth.getValue()));
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
                src.setText(owner.modMatrix.getModSrcName(a.src), juce::dontSendNotification);
                dst.setText(a.dst->getName(100), juce::dontSendNotification);

                auto ev = owner.modMatrix.getModEnable(a.src, gin::ModDstId(a.dst->getModIndex()));
                enableButton.setToggleState(ev, juce::dontSendNotification);

                auto b = owner.modMatrix.getModBipolarMapping(a.src, gin::ModDstId(a.dst->getModIndex()));
                biPolarButton.setToggleState(b, juce::dontSendNotification);

                depth.setValue(owner.modMatrix.getModDepth(a.src, gin::ModDstId(a.dst->getModIndex())), juce::dontSendNotification);
                curveButton.setCurve(owner.modMatrix.getModFunction(a.src, gin::ModDstId(a.dst->getModIndex())));
            }
            else
            {
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

        APModMatrixBox& owner;
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
        APModDepthLookAndFeel depthLookAndFeel;

        juce::Label src;
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
};

class APKnobLNF : public juce::LookAndFeel_V4 {
public:
    APKnobLNF() {
        setColour (juce::Slider::thumbColourId, juce::Colour(0xffCC8866));
        setColour (juce::Slider::rotarySliderFillColourId, juce::Colour(0xffCC8866));
        setColour (juce::Slider::trackColourId, juce::Colour(0xff797C84));
    }
    
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                           const float rotaryStartAngleIn, const float rotaryEndAngle, juce::Slider& slider)
    {
        float rotaryStartAngle = rotaryStartAngleIn;
        const float radius = juce::jmin (width / 2, height / 2) - 2.0f;
        const float centreX = x + width * 0.5f;
        const float centreY = y + height * 0.5f;
        const float rx = centreX - radius;
        const float ry = centreY - radius;
        const float rw = radius * 2.0f;
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        const bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

        const float thickness = (radius - 2) / radius;

        g.setColour (slider.findColour (juce::Slider::trackColourId).withMultipliedAlpha (slider.isEnabled() ? 1.0f : 0.5f));

        // Draw knob
        {
            const auto rcO = juce::Rectangle<float> (rx, ry, rw, rw).withSizeKeepingCentre (radius, radius);
            const auto rcI = juce::Rectangle<float> (rx, ry, rw, rw).withSizeKeepingCentre (radius * 0.17f, radius * 0.17f);
            const auto c = 2.0f * pi * radius;
            const auto gap = (rcI.getWidth () / c) * 2.0f * pi;

            juce::Path knob;
            knob.addArc (rcO.getX(), rcO.getY(), rcO.getWidth(), rcO.getHeight(), angle + gap, angle - gap + pi * 2, true );
            knob.addArc (rcI.getX(), rcI.getY(), rcI.getWidth(), rcI.getHeight(), angle - pi / 2, angle + pi / 2 - pi * 2, false );
            knob.closeSubPath();
            g.fillPath (knob);
        }

        {
            juce::Path filledArc;
            filledArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, rotaryEndAngle, thickness);
            g.fillPath (filledArc);
        }

        if (slider.isEnabled())
            g.setColour (slider.findColour (juce::Slider::rotarySliderFillColourId).withAlpha (isMouseOver ? 0.95f : 0.85f));

        auto fillStartAngle = rotaryStartAngle;
        if (slider.getProperties().contains ("fromCentre"))
            fillStartAngle = (rotaryStartAngle + rotaryEndAngle) / 2;

        {
            juce::Path filledArc;
            filledArc.addPieSegment (rx, ry, rw, rw, fillStartAngle, angle, thickness);
            g.fillPath (filledArc);
        }

        if (slider.getProperties().contains ("modDepth"))
        {
            auto depth = (float)slider.getProperties()["modDepth"];
            bool bipolar = (bool)slider.getProperties()["modBipolar"];

            g.setColour (juce::Colour(0xffFFFFFF).withAlpha (0.9f));

            juce::Path filledArc;
            if (bipolar)
            {
                auto a = juce::jlimit (rotaryStartAngle, rotaryEndAngle, angle - depth * (rotaryEndAngle - rotaryStartAngle));
                auto b = juce::jlimit (rotaryStartAngle, rotaryEndAngle, angle + depth * (rotaryEndAngle - rotaryStartAngle));
                filledArc.addPieSegment (rx, ry, rw, rw, std::min (a, b), std::max (a, b), thickness);
            }
            else
            {
                auto modPos = juce::jlimit (rotaryStartAngle, rotaryEndAngle, angle + depth * (rotaryEndAngle - rotaryStartAngle));
                filledArc.addPieSegment (rx, ry, rw, rw, angle, modPos, thickness);
            }

            g.fillPath (filledArc);
        }

        if (slider.getProperties().contains ("modValues") && slider.isEnabled())
        {
            g.setColour (juce::Colour(0xffFFFFFF).withAlpha (0.9f));

            auto varArray = slider.getProperties()["modValues"];
            if (varArray.isArray())
            {
                for (auto value : *varArray.getArray())
                {
                    float modAngle = float (value) * (rotaryEndAngle - rotaryStartAngle) + rotaryStartAngle;

                    float modX = centreX + std::sin (modAngle) * radius;
                    float modY = centreY - std::cos (modAngle) * radius;

                    g.fillEllipse (modX - 2, modY - 2, 4.0f, 4.0f);
                }
            }
        }
    }
};

//==============================================================================
/** Slider + editable text for showing a param
*/
class APKnob : public gin::ParamComponent,
             public juce::DragAndDropTarget,
             private juce::Timer, private gin::ModMatrix::Listener
{
public:

    
    
    APKnob (gin::Parameter* p, bool fromCentre=false)
      : gin::ParamComponent (p),
        value (parameter),
        knob (parameter, juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox)
    {
        addAndMakeVisible (name);
        addAndMakeVisible (value);
        addAndMakeVisible (knob);
        addChildComponent (modDepthSlider);
        setLookAndFeel(&knobLNF);
        
        modDepthSlider.setRange (-1.0, 1.0, 0.001);
        modDepthSlider.setPopupDisplayEnabled (true, true, findParentComponentOfClass<juce::AudioProcessorEditor>());
        modDepthSlider.setDoubleClickReturnValue (true, 0.0);

        knob.setTitle (parameter->getName (100));
        knob.setDoubleClickReturnValue (true, parameter->getUserDefaultValue());
        knob.setSkewFactor (parameter->getSkew(), parameter->isSkewSymmetric());
        if (fromCentre)
            knob.getProperties().set ("fromCentre", true);

        knob.setName (parameter->getShortName());

        name.setText (parameter->getShortName(), juce::dontSendNotification);
        name.setJustificationType (juce::Justification::centred);
       #if JUCE_IOS
        knob.setMouseDragSensitivity (500);
       #endif

        value.setTitle (parameter->getName (100));
        value.setJustificationType (juce::Justification::centred);
		value.setVisible (false);

        addMouseListener (this, true);

        if (parameter->getModIndex() >= 0)
        {
            auto& mm = *parameter->getModMatrix();
            mm.addListener (this);
        }

        modTimer.onTimer = [this] ()
        {
            auto& mm = *parameter->getModMatrix();
            if (mm.shouldShowLiveModValues())
            {
                auto curModValues = liveValuesCallback ? liveValuesCallback() : mm.getLiveValues (parameter);
                if (curModValues != modValues)
                {
                    modValues = curModValues;

                    juce::Array<juce::var> vals;
                    for (auto v : modValues)
                        vals.add (v);

                    knob.getProperties().set ("modValues", vals);

                    repaint();
                }
            }
            else if (knob.getProperties().contains ("modValues"))
            {
                knob.getProperties().remove ("modValues");
                repaint();
            }
        };
        shiftTimer.onTimer = [this] ()
        {
            bool shift = juce::ModifierKeys::getCurrentModifiersRealtime().isShiftDown();
            knob.setInterceptsMouseClicks (! learning || shift, ! learning || shift );
        };

        modDepthSlider.onClick = [this] { showModMenu(); };
        modDepthSlider.setMouseDragSensitivity (500);
        modDepthSlider.onValueChange = [this]
        {
            if (auto mm = parameter->getModMatrix())
            {
                auto dst = gin::ModDstId (parameter->getModIndex());

                if (auto depths = mm->getModDepths (dst); depths.size() > 0)
                {
                        mm->setModDepth(currentModSrc, dst, float (modDepthSlider.getValue()));
                }
            }
        };
        
        modDepthSlider.onTextFromValue = [this] (double v)
        {
            if (auto mm = parameter->getModMatrix())
            {
                auto dst = gin::ModDstId (parameter->getModIndex());

                if (auto depths = mm->getModDepths(dst); depths.size() > 0)
                {
                    auto pname      = mm->getModSrcName(currentModSrc);
                    return pname + ": " + juce::String(v);
                }
            }
            return juce::String();
        };
        
        modMatrixChanged();
    }

    ~APKnob() override
    {
        if (parameter->getModIndex() >= 0)
        {
            auto& mm = *parameter->getModMatrix();
            mm.removeListener (this);
        }
    }
    

    void setDisplayName (const juce::String& n)
    {
        name.setText (n, juce::dontSendNotification);
    }

    void setLiveValuesCallback (std::function<juce::Array<float> ()> cb)
    {
        liveValuesCallback = cb;
        modMatrixChanged();
    }

    gin::PluginSlider& getSlider()   { return knob; }
    gin::Readout& getReadout()       { return value; }

    void paint (juce::Graphics& g) override
    {
        if (dragOver)
        {
            g.setColour (findColour (gin::PluginLookAndFeel::accentColourId, true).withAlpha (0.3f));
            g.fillEllipse (knob.getBounds().toFloat());
        }
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (2);

        auto extra = r.getHeight() - r.getWidth();

        auto rc = r.removeFromBottom (extra);

        name.setBounds (rc);
        value.setBounds (rc);
        knob.setBounds (r.reduced (2));
		auto height = value.getHeight();
		value.setFont(regularFont.withHeight(height*0.8f));
		name.setFont(regularFont.withHeight(height*0.8f));


        modDepthSlider.setBounds (knob.getBounds().removeFromTop (7).removeFromRight (7).reduced (-3));
    }
    
    void parentHierarchyChanged() override
    {
        auto a = wantsAccessibleKeyboard (*this);
        name.setWantsKeyboardFocus (a);
        value.setWantsKeyboardFocus (a);
        knob.setWantsKeyboardFocus (a);

        if (wantsAccessibleKeyboard (*this))
        {
            name.setVisible (false);
            value.setVisible (true);
        }
        else
        {
            name.setVisible (true);
            value.setVisible (false);
        }
    }

    void mouseEnter (const juce::MouseEvent& /*ev*/) override
    {
        if (wantsAccessibleKeyboard (*this))
            return;

        if (! isTimerRunning() && isEnabled())
        {
            startTimer (100);
            name.setVisible (false);
            value.setVisible (true);
        }
    }
    
    void mouseDown (const juce::MouseEvent& ev) override
    {
        if (! isEnabled())
            return;

        bool shift = juce::ModifierKeys::getCurrentModifiersRealtime().isShiftDown();
        if (shift || ! learning || ! knob.getBounds().contains (ev.getMouseDownPosition()))
            return;

        auto& mm = *parameter->getModMatrix();
        auto dst = gin::ModDstId (parameter->getModIndex());
        modDepth = mm.getModDepth (mm.getLearn(), dst);

        knob.getProperties().set ("modDepth", modDepth);

        repaint();
    }

    void mouseDrag (const juce::MouseEvent& ev) override
    {
        if (! isEnabled())
            return;

        bool shift = juce::ModifierKeys::getCurrentModifiersRealtime().isShiftDown();
        if (shift || ! learning || ! knob.getBounds().contains (ev.getMouseDownPosition()))
             return;

        if (ev.getDistanceFromDragStart() >= 3)
        {
            auto pt = ev.getMouseDownPosition();
            auto delta = (ev.position.x - pt.getX()) + (pt.getY() - ev.position.y);

            float newModDepth = juce::jlimit (-1.0f, 1.0f, delta / 200.0f + modDepth);

            auto& mm = *parameter->getModMatrix();
            
            auto dst = gin::ModDstId (parameter->getModIndex());
            
            knob.getProperties().set("modDepth", newModDepth);
            if (bool bi = mm.getModBipolarMapping(mm.getLearn(), dst))
            {
                knob.getProperties().set("modBipolar", bi);
            }

            
            

            auto range = parameter->getUserRange();
            if (range.interval <= 0.0f || juce::ModifierKeys::currentModifiers.isShiftDown())
            {
                mm.setModDepth (mm.getLearn(), dst, newModDepth);
            }
            else
            {
                mm.setModDepth (mm.getLearn(), dst, newModDepth);
                modDepthSlider.setValue (newModDepth, juce::dontSendNotification);
            }

            repaint();
        }
    }

    bool isInterestedInDragSource (const SourceDetails& sd) override
    {
        if (isEnabled() && parameter && parameter->getModMatrix())
            return sd.description.toString().startsWith ("modSrc");

        return false;
    }

    void itemDragEnter (const SourceDetails& /*dragSourceDetails*/) override
    {
        dragOver = true;
        repaint();
    }
    
    void itemDragExit (const SourceDetails& /*dragSourceDetails*/) override
    {
        dragOver = false;
        repaint();
    }
    
    void itemDropped (const SourceDetails& sd) override
    {
        dragOver = false;
        repaint();

        auto& mm = *parameter->getModMatrix();

        currentModSrc = gin::ModSrcId (sd.description.toString().getTrailingIntValue());
        auto dst = gin::ModDstId (parameter->getModIndex());

        mm.setModDepth (currentModSrc, dst, 1.0f);
    }
    
    
protected:
    void timerCallback() override
    {
        auto p = getMouseXYRelative();
        if (! getLocalBounds().contains (p) &&
            ! juce::ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown() &&
            ! value.isBeingEdited())
        {
            if (wantsAccessibleKeyboard (*this))
            {
                name.setVisible (false);
                value.setVisible (true);
            }
            else
            {
                name.setVisible (true);
                value.setVisible (false);
            }

            stopTimer();
        }
    }

    void learnSourceChanged (gin::ModSrcId src) override
    {
        learning = src.isValid();
        if (learning)
            currentModSrc = src;

        bool shift = juce::ModifierKeys::getCurrentModifiersRealtime().isShiftDown();
        knob.setInterceptsMouseClicks (! learning || shift, ! learning || shift );

        auto& mm = *parameter->getModMatrix();
        modDepth = mm.getModDepth (mm.getLearn(), gin::ModDstId (parameter->getModIndex()));

        if (learning)
        {
            knob.getProperties().set ("modDepth", modDepth);
            knob.getProperties().set ("modBipolar", mm.getModBipolarMapping (mm.getLearn(), gin::ModDstId (parameter->getModIndex())));

            shiftTimer.startTimerHz (100);
        }
        else
        {
            knob.getProperties().remove ("modDepth");
            knob.getProperties().remove ("modBipolar");

            shiftTimer.stopTimer();
        }

        repaint();
    }
    
    
    void modMatrixChanged() override
    {
        if (auto mm = parameter->getModMatrix())
        {
            auto dst = gin::ModDstId (parameter->getModIndex());

            if (mm->isModulated (dst) || liveValuesCallback)
            {
                modTimer.startTimerHz (30);

                auto vis = mm->isModulated (dst);
                if (vis != modDepthSlider.isVisible())
                {
                    modDepthSlider.setVisible (vis);
                    resized();
                }

                if (auto depths = mm->getModDepths (dst); depths.size() > 0) {
                    for (auto depth : depths) {
                        if (depth.first == currentModSrc)
                            modDepthSlider.setValue (depth.second, juce::dontSendNotification);
                    }
                }
                else
                    modDepthSlider.setValue (0.0f, juce::dontSendNotification);
            }
            else
            {
                modTimer.stopTimer();
                knob.getProperties().remove ("modValues");

                if (modDepthSlider.isVisible())
                {
                    modDepthSlider.setVisible (false);
                    resized();
                }
            }

            if (learning && ! isMouseButtonDown (true))
            {
                modDepth = mm->getModDepth (mm->getLearn(), dst);
                knob.getProperties().set ("modDepth", modDepth);
                knob.getProperties().set ("modBipolar", mm->getModBipolarMapping (mm->getLearn(), gin::ModDstId (parameter->getModIndex())));
                repaint();
            }
        }
    }

    void showModMenu()
    {
        juce::PopupMenu m;
        m.setLookAndFeel (&getLookAndFeel());

        auto& mm = *parameter->getModMatrix();
        for (auto src : mm.getModSources (parameter))
        {
            bool current{false};
            if (src == currentModSrc) { current = true; }
            m.addItem ("Remove: " + mm.getModSrcName (src), true, current, [this, src]
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
        
        for (auto src : mm.getModSources (parameter))
        {
            bool editing = (src == currentModSrc) ? true : false;
            m.addItem ("Edit: " + mm.getModSrcName(src) + ": " + String(mm.getModDepth(src, gin::ModDstId(parameter->getModIndex())), 3), !editing, editing, [this, src]
            {
                currentModSrc = src;
                modMatrixChanged();
            });
        }
        
        

        m.showMenuAsync ({});
    }

    juce::Label name;
    gin::Readout value;
    gin::PluginSlider knob;
    bool learning = false;
    float modDepth = 0.0f;
    bool dragOver = false;
    gin::ModSrcId currentModSrc{-1};

    APKnobLNF knobLNF;
    gin::CoalescedTimer modTimer;
    gin::CoalescedTimer shiftTimer;
    juce::Array<float> modValues;
    std::function<juce::Array<float> ()> liveValuesCallback;
    APModulationDepthSlider modDepthSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (APKnob)
};

