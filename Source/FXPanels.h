#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class FXBox : public gin::ParamBox
{
public:
	FXBox(const juce::String& name, APAudioProcessor& proc_, gin::Parameter::Ptr box_)
		: gin::ParamBox(name), proc(proc_), box(box_)
	{
		setName(name);
		addControl(new gin::Select(box), 0, 0, 3, 1); // remove? will count on parent to indicate which effect is selected


		
		setSize(300, 233);
	}
	void valueUpdated(gin::Parameter* param) override {
		if (param == box)
		{
			if (param->getUserValue() == 0.0f)
			{
			}
			else
			{
				
			}
		}
	}
	void resized() override {
		gin::ParamBox::resized();
	}
	gin::Parameter::Ptr box;
	APAudioProcessor& proc;
};