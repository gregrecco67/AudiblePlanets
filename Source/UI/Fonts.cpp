#include "Fonts.h"

APLNF::APLNF()
{
	setColour(juce::Slider::thumbColourId, juce::Colour(0xff888888));
	setColour(juce::Slider::rotarySliderFillColourId,
	    juce::Colour(0xffa09189).darker(0.3f));
	setColour(juce::Slider::trackColourId, juce::Colour(0xff000000));
	setColour(
	    juce::BubbleComponent::backgroundColourId, juce::Colour(0xff16171A));
	setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff16171A));
	setColour(juce::PopupMenu::textColourId, juce::Colour(0xff66AACC));
	setColour(juce::PopupMenu::headerTextColourId, juce::Colour(0xff9B9EA5));
	setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
	setColour(juce::PopupMenu::highlightedBackgroundColourId,
	    juce::Colours::darkgrey);
	setColour(juce::TooltipWindow::textColourId, findColour(accentColourId));
	// setColour(juce::TextButton::buttonOnColourId, juce::Colours::grey);
}

juce::Typeface::Ptr fontAwesomeTypeface =
    juce::Typeface::createSystemTypefaceFor(
        BinaryData::latoregular_otf, BinaryData::latoregular_otfSize);

juce::Font APLNF::getLabelFont(juce::Label &label)
{
	return juce::Font{juce::FontOptions{}.withName("Lato").withHeight(
	    label.getHeight() * 0.89f)};
}

juce::Font APLNF::getPopupMenuFont()
{
	return juce::Font{
	    juce::FontOptions{}.withName("Lato").withHeight(20.f * 0.89f)};
}

juce::Font APLNF::getSliderPopupFont(juce::Slider& /*slider*/)
{
	return juce::Font{
	    juce::FontOptions{}.withName("Lato").withHeight(20.f * 0.89f)};
}

void APLNF::drawRotarySlider(juce::Graphics &g,
    int x,
    int y,
    int width,
    int height,
    float sliderPos,
    const float rotaryStartAngleIn,
    const float rotaryEndAngle,
    juce::Slider &slider)
{
	float rotaryStartAngle = rotaryStartAngleIn;
	const float radius = juce::jmin(width * 0.5f, height * 0.5f) - 2.0f;
	const float centreX = x + width * 0.5f;
	const float centreY = y + height * 0.5f;
	const float rx = centreX - radius;
	const float ry = centreY - radius;
	const float rw = radius * 2.0f;
	const float angle =
	    rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
	const bool isMouseOver =
	    slider.isMouseOverOrDragging() && slider.isEnabled();

	const float thickness = (radius - 2) / radius;

	// Draw knob
	{
		const auto rcO =
		    juce::Rectangle<float>(rx, ry, rw, rw)
		        .withSizeKeepingCentre(radius * 1.6f, radius * 1.6f);
		const auto rcI =
		    juce::Rectangle<float>(rx, ry, rw, rw)
		        .withSizeKeepingCentre(radius * 0.08f, radius * 0.08f);
		const auto c = 2.0f * static_cast<float>(pi) * radius;
		const float gap = (rcI.getWidth() / c) * 2.0f * static_cast<float>(pi);

		float alpha = isMouseOver ? 1.0f : 0.7f;
		float dimHL = slider.isEnabled() ? 0.7f : 0.4f;
		juce::Path knob;
		juce::ColourGradient gradient{
		    juce::Colours::white.withBrightness(dimHL), radius * .75f,
		    radius * .75f,
		    slider.findColour(juce::Slider::rotarySliderFillColourId)
		        .withMultipliedAlpha(alpha),
		    radius * 1.1f, radius * 1.1f, true};
		g.setGradientFill(gradient);
		knob.addArc(rcO.getX(), rcO.getY(), rcO.getWidth(), rcO.getHeight(),
		    angle + gap, angle - gap + static_cast<float>(pi) * 2, true);
		knob.addArc(rcI.getX(), rcI.getY(), rcI.getWidth(), rcI.getHeight(),
		    angle - static_cast<float>(pi) / 2, angle + static_cast<float>(pi) / 2 -
		    static_cast<float>(pi) * 2, false);
		knob.closeSubPath();
		g.fillPath(knob);
	}

	g.setColour(slider.findColour(juce::Slider::trackColourId)
	        .withMultipliedAlpha(slider.isEnabled() ? 1.0f : 0.5f));

	{
		juce::Path filledArc;
		filledArc.addPieSegment(
		    rx, ry, rw, rw, rotaryStartAngle, rotaryEndAngle, thickness);
		g.fillPath(filledArc);
	}

	if (slider.isEnabled())
		g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId)
		        .withAlpha(isMouseOver ? 0.95f : 0.85f));

	auto fillStartAngle = rotaryStartAngle;
	if (slider.getProperties().contains("fromCentre"))
		fillStartAngle = (rotaryStartAngle + rotaryEndAngle) / 2;

	{
		juce::Path filledArc;
		filledArc.addPieSegment(
		    rx, ry, rw, rw, fillStartAngle, angle, thickness);
		g.fillPath(filledArc);
	}

	if (slider.getProperties().contains("modDepth")) {
		auto depth = static_cast<float>(slider.getProperties()["modDepth"]);
		bool bipolar = (bool)slider.getProperties()["modBipolar"];

		g.setColour(juce::Colour(0xffFFFFFF).withAlpha(0.9f));

		juce::Path filledArc;
		if (bipolar) {
			auto a = juce::jlimit(rotaryStartAngle, rotaryEndAngle,
			    angle - depth * (rotaryEndAngle - rotaryStartAngle));
			auto b = juce::jlimit(rotaryStartAngle, rotaryEndAngle,
			    angle + depth * (rotaryEndAngle - rotaryStartAngle));
			filledArc.addPieSegment(
			    rx, ry, rw, rw, std::min(a, b), std::max(a, b), thickness);
		} else {
			auto modPos = juce::jlimit(rotaryStartAngle, rotaryEndAngle,
			    angle + depth * (rotaryEndAngle - rotaryStartAngle));
			filledArc.addPieSegment(rx, ry, rw, rw, angle, modPos, thickness);
		}

		g.fillPath(filledArc);
	}

	if (slider.getProperties().contains("modValues") && slider.isEnabled()) {
		g.setColour(juce::Colour(0xffFFFFFF).withAlpha(0.9f));

		if (auto varArray = slider.getProperties()["modValues"]; varArray.isArray()) {
			for (const auto& value : *varArray.getArray()) {
				float modAngle = static_cast<float>(value) * (rotaryEndAngle - rotaryStartAngle) + rotaryStartAngle;
				float modX = centreX + std::sin(modAngle) * radius;
				float modY = centreY - std::cos(modAngle) * radius;

				g.fillEllipse(modX - 2, modY - 2, 4.0f, 4.0f);
			}
		}
	}
}

void APLNF::drawLinearSlider(juce::Graphics &g,
    int x,
    int y,
    int width,
    int height,
    float sliderPos,
    float minSliderPos,
    float maxSliderPos,
    juce::Slider::SliderStyle style,
    juce::Slider &slider)
{
	if (style != juce::Slider::LinearBarVertical) {
		gin::CopperLookAndFeel::drawLinearSlider(g, x, y, width, height,
		    sliderPos, minSliderPos, maxSliderPos, style, slider);
		return;
	}
	
    g.setColour(juce::Colours::black); // empty track
    g.fillRoundedRectangle(juce::Rectangle<float>(static_cast<float>(x) + 22.f, 5,
        7, static_cast<float>(y) + (static_cast<float>(height) - 10)), 3.f);

    const auto constrained = juce::jlimit(y+5, y + height - 5, static_cast<int>(sliderPos));
	g.setColour(slider.findColour(juce::Slider::thumbColourId));
	g.fillRoundedRectangle(juce::Rectangle<float>(static_cast<float>(x) + 22.f, constrained, 7,
	                           static_cast<float>(y) + (static_cast<float>(height) - 5 - constrained)),
	    3.f); // filled track


	g.setColour(juce::Colours::black);
	constexpr float gap{4.f};
	g.fillRoundedRectangle(static_cast<float>(x) + gap, constrained - 10.0f, static_cast<float>(width)
		- 2.f * gap, 20.0f, 5.f); // handle
	g.setColour(slider.findColour(juce::Slider::thumbColourId).darker(0.7f));
	g.drawRoundedRectangle(static_cast<float>(x) + gap, constrained - 10.0f, static_cast<float>(width)
		- 2.f * gap, 20.0f, 5.f, 1.f); // handle outline
	g.setColour(slider.findColour(juce::Slider::thumbColourId));
	g.fillRect(
	    x + gap + 5.f, static_cast<float>(constrained) - 1, static_cast<float>(width - 2.f * gap - 10.f), 3.f); // handle indicator
	
}
