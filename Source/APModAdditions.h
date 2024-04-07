#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

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
