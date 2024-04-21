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
#include "Panels.h"
#include "Editor.h"
#include "FXEditor.h"
#include "RandEditor.h"
#include "MsegEditor.h"
#include "MacrosEditor.h"
#include "APColors.h"

//==============================================================================
class APAudioProcessorEditor : public gin::ProcessorEditor,
	public juce::DragAndDropContainer, public juce::KeyListener, public juce::Timer
{
public:
    APAudioProcessorEditor(APAudioProcessor&);
    ~APAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    void addMenuItems(juce::PopupMenu& m) override;
    bool keyPressed(const KeyPress& key, Component* originatingComponent) override;
    using juce::Component::keyPressed; // above is overloaded
	void timerCallback() override;
    
    class TabLNF : public APLNF {
    public:
        TabLNF() = default;

        void drawTabButton (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown) override
        {
            const Rectangle<int> activeArea (button.getActiveArea());

            const TabbedButtonBar::Orientation o = button.getTabbedButtonBar().getOrientation();

            const Colour bkg (button.getTabBackgroundColour());

            if (button.getToggleState())
            {
                g.setColour (bkg);
            }
            else
            {
                Point<int> p1, p2;

                switch (o)
                {
                    case TabbedButtonBar::TabsAtBottom:   p1 = activeArea.getBottomLeft(); p2 = activeArea.getTopLeft();    break;
                    case TabbedButtonBar::TabsAtTop:      p1 = activeArea.getTopLeft();    p2 = activeArea.getBottomLeft(); break;
                    case TabbedButtonBar::TabsAtRight:    p1 = activeArea.getTopRight();   p2 = activeArea.getTopLeft();    break;
                    case TabbedButtonBar::TabsAtLeft:     p1 = activeArea.getTopLeft();    p2 = activeArea.getTopRight();   break;
                    default:                              jassertfalse; break;
                }

                g.setGradientFill (ColourGradient (bkg.brighter (0.2f), p1.toFloat(),
                                                   bkg.darker (0.1f),   p2.toFloat(), false));
            }

            g.fillRect (activeArea);

            g.setColour (button.findColour (TabbedButtonBar::tabOutlineColourId));

            Rectangle<int> r (activeArea);

            if (o != TabbedButtonBar::TabsAtBottom)   g.fillRect (r.removeFromTop (1));
            if (o != TabbedButtonBar::TabsAtTop)      g.fillRect (r.removeFromBottom (1));
            if (o != TabbedButtonBar::TabsAtRight)    g.fillRect (r.removeFromLeft (1));
            if (o != TabbedButtonBar::TabsAtLeft)     g.fillRect (r.removeFromRight (1));

            const float alpha = button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f;

            Colour col (bkg.contrasting().withMultipliedAlpha (alpha));

            if (TabbedButtonBar* bar = button.findParentComponentOfClass<TabbedButtonBar>())
            {
                TabbedButtonBar::ColourIds colID = button.isFrontTab() ? TabbedButtonBar::frontTextColourId
                                                                       : TabbedButtonBar::tabTextColourId;

                if (bar->isColourSpecified (colID))
                    col = bar->findColour (colID);
                else if (isColourSpecified (colID))
                    col = findColour (colID);
            }

            const Rectangle<float> area (button.getTextArea().toFloat());

            float length = area.getWidth();
            float depth  = area.getHeight();

            if (button.getTabbedButtonBar().isVertical())
                std::swap (length, depth);

            TextLayout textLayout;
            //createTabTextLayout (button, length, depth, col, textLayout);
            
            

            Font font = regularFont.withHeight(depth * 0.5f);
            font.setUnderline (button.hasKeyboardFocus (false));

            AttributedString s;
            s.setJustification (Justification::centred);
            s.append (button.getButtonText().trim(), font, col);

            textLayout.createLayout (s, length);

            AffineTransform t;

            switch (o)
            {
                case TabbedButtonBar::TabsAtLeft:   t = t.rotated (MathConstants<float>::pi * -0.5f).translated (area.getX(), area.getBottom()); break;
                case TabbedButtonBar::TabsAtRight:  t = t.rotated (MathConstants<float>::pi *  0.5f).translated (area.getRight(), area.getY()); break;
                case TabbedButtonBar::TabsAtTop:
                case TabbedButtonBar::TabsAtBottom: t = t.translated (area.getX(), area.getY()); break;
                default:                            jassertfalse; break;
            }

            g.addTransform (t);
            textLayout.draw (g, Rectangle<float> (length, depth));
        }
        
        juce::Font getTabButtonFont(TabBarButton&, float height) override {
            return regularFont.withHeight(height * 0.6f);
        }
        
        //juce::Font regularFont{ juce::Typeface::createSystemTypefaceFor(BinaryData::latoregular_otf, BinaryData::latoregular_otfSize) };
        
    } tabLNF;
    

private:
    APAudioProcessor& proc;

    
    gin::SynthesiserUsage usage { proc.synth };
	gin::LevelMeter meter { proc.levelTracker };
    
    juce::TabbedComponent tabbed{juce::TabbedButtonBar::TabsAtBottom };
    juce::Component tab1, tab2, tab3, tab4, tab5;

    Editor editor { proc };
	FXEditor fxEditor{ proc };
	RandEditor randEditor{ proc };
	MsegEditor msegEditor{ proc };
	MacrosEditor macrosEditor{ proc };
	APKnob levelKnob{ proc.globalParams.level }, auxKnob{ proc.auxParams.volume }, samplerKnob{ proc.samplerParams.volume };

	Label scaleName, learningLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(APAudioProcessorEditor)
};
