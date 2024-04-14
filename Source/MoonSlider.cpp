#include "MoonSlider.h"

MoonSlider::MoonSlider(gin::Parameter* parameter_) : gin::PluginSlider(parameter_, Slider::SliderStyle::RotaryHorizontalVerticalDrag, Slider::NoTextBox), parameter(parameter_), value(parameter_)
{
	setName(parameter->getShortName());
	filmStrip = juce::ImageCache::getFromMemory(BinaryData::moonstrip_png, BinaryData::moonstrip_pngSize);
	if (filmStrip.isValid())
	{
		setTextBoxStyle(NoTextBox, 0, 0, 0);
		setSliderStyle(RotaryHorizontalVerticalDrag);
		setRange(0.0f, 1.0f, 0.0f);
		setDoubleClickReturnValue(true, 0.5f);
		setOpaque(true);
		setRotaryParameters(0.f, juce::MathConstants<float>::twoPi, true);
	}
}
