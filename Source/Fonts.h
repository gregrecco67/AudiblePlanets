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

#include "BinaryData.h"
#include "JuceHeader.h"
#include <numbers>

using std::numbers::pi;

class APLNF : public gin::CopperLookAndFeel
{
public:
	APLNF() {
		setColour(juce::Slider::thumbColourId, juce::Colour(0xffCC8866));
		setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffCC8866));
		setColour(juce::Slider::trackColourId, juce::Colour(0xff797C84));
		setColour(juce::BubbleComponent::backgroundColourId, juce::Colour(0xff16171A));
		setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff16171A));
		setColour(juce::PopupMenu::textColourId, juce::Colour(0xffE6E6E9));
		setColour(juce::PopupMenu::headerTextColourId, juce::Colour(0xff9B9EA5));
		setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xffCC8866));
		setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
	}

	//juce::Typeface::Ptr getTypefaceForFont(const juce::Font& font) override
	//{
	//	return juce::Typeface::createSystemTypefaceFor(BinaryData::latoregular_otf, BinaryData::latoregular_otfSize);
	//}

	TypefaceMetricsKind getDefaultMetricsKind() const { return TypefaceMetricsKind::legacy; }
	
	juce::Font getLabelFont(juce::Label& label) override
	{
        return label.getFont().withHeight(label.getHeight() * 0.7f);
 	}

//	juce::Font getTextButtonFont(juce::TextButton& button, int height) override
//	{
//		return regularFont.withHeight(12);
//	}
// //
//	juce::Font getSliderPopupFont(Slider&) override
//	{
//		return regularFont.withHeight(12);
//	}
//
//	juce::Font getComboBoxFont(ComboBox&) override
//	{
//		return regularFont.withHeight(12);
//	}
//
//	juce::Font getPopupMenuFont() override
//	{
//		return regularFont.withHeight(12);
//	}

	void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
		const float rotaryStartAngleIn, const float rotaryEndAngle, juce::Slider& slider)
	{
		float rotaryStartAngle = rotaryStartAngleIn;
		const float radius = juce::jmin(width / 2, height / 2) - 2.0f;
		const float centreX = x + width * 0.5f;
		const float centreY = y + height * 0.5f;
		const float rx = centreX - radius;
		const float ry = centreY - radius;
		const float rw = radius * 2.0f;
		const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
		const bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

		const float thickness = (radius - 2) / radius;

		g.setColour(slider.findColour(juce::Slider::trackColourId).withMultipliedAlpha(slider.isEnabled() ? 1.0f : 0.5f));

		// Draw knob
		{
			const auto rcO = juce::Rectangle<float>(rx, ry, rw, rw).withSizeKeepingCentre(radius, radius);
			const auto rcI = juce::Rectangle<float>(rx, ry, rw, rw).withSizeKeepingCentre(radius * 0.17f, radius * 0.17f);
			const auto c = 2.0f * pi * radius;
			const auto gap = (rcI.getWidth() / c) * 2.0f * pi;

			juce::Path knob;
			knob.addArc(rcO.getX(), rcO.getY(), rcO.getWidth(), rcO.getHeight(), angle + gap, angle - gap + pi * 2, true);
			knob.addArc(rcI.getX(), rcI.getY(), rcI.getWidth(), rcI.getHeight(), angle - pi / 2, angle + pi / 2 - pi * 2, false);
			knob.closeSubPath();
			g.fillPath(knob);
		}

		{
			juce::Path filledArc;
			filledArc.addPieSegment(rx, ry, rw, rw, rotaryStartAngle, rotaryEndAngle, thickness);
			g.fillPath(filledArc);
		}

		if (slider.isEnabled())
			g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId).withAlpha(isMouseOver ? 0.95f : 0.85f));

		auto fillStartAngle = rotaryStartAngle;
		if (slider.getProperties().contains("fromCentre"))
			fillStartAngle = (rotaryStartAngle + rotaryEndAngle) / 2;

		{
			juce::Path filledArc;
			filledArc.addPieSegment(rx, ry, rw, rw, fillStartAngle, angle, thickness);
			g.fillPath(filledArc);
		}

		if (slider.getProperties().contains("modDepth"))
		{
			auto depth = (float)slider.getProperties()["modDepth"];
			bool bipolar = (bool)slider.getProperties()["modBipolar"];

			g.setColour(juce::Colour(0xffFFFFFF).withAlpha(0.9f));

			juce::Path filledArc;
			if (bipolar)
			{
				auto a = juce::jlimit(rotaryStartAngle, rotaryEndAngle, angle - depth * (rotaryEndAngle - rotaryStartAngle));
				auto b = juce::jlimit(rotaryStartAngle, rotaryEndAngle, angle + depth * (rotaryEndAngle - rotaryStartAngle));
				filledArc.addPieSegment(rx, ry, rw, rw, std::min(a, b), std::max(a, b), thickness);
			}
			else
			{
				auto modPos = juce::jlimit(rotaryStartAngle, rotaryEndAngle, angle + depth * (rotaryEndAngle - rotaryStartAngle));
				filledArc.addPieSegment(rx, ry, rw, rw, angle, modPos, thickness);
			}

			g.fillPath(filledArc);
		}

		if (slider.getProperties().contains("modValues") && slider.isEnabled())
		{
			g.setColour(juce::Colour(0xffFFFFFF).withAlpha(0.9f));

			auto varArray = slider.getProperties()["modValues"];
			if (varArray.isArray())
			{
				for (auto value : *varArray.getArray())
				{
					float modAngle = float(value) * (rotaryEndAngle - rotaryStartAngle) + rotaryStartAngle;

					float modX = centreX + std::sin(modAngle) * radius;
					float modY = centreY - std::cos(modAngle) * radius;

					g.fillEllipse(modX - 2, modY - 2, 4.0f, 4.0f);
				}
			}
		}
	}
};
