#include <JuceHeader.h>
#include "PluginProcessor.h"

class RandMatrixBox : public gin::ParamBox
{
public:
	RandMatrixBox(const juce::String& name, APAudioProcessor& proc_)
		: gin::ParamBox(name), proc(proc_)
	{
		setName("mtx");

		addControl(new gin::ModMatrixBox(proc, proc.modMatrix), 0, 0, 5, 8);
	}

	APAudioProcessor& proc;
};