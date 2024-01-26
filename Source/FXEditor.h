#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "BinaryData.h"
#include "FXPanels.h"

//==============================================================================
class FXEditor : public juce::Component, public gin::Parameter::ParameterListener
{
public:
	FXEditor(APAudioProcessor& proc_);
	void resized() override;
	void valueUpdated(gin::Parameter* param);
	juce::Label hello{ "Hello", "Hello" };
	void sayHello(bool shouldSayHello) {
		if (shouldSayHello)
		{
			hello.setText("Hello", juce::dontSendNotification);
		}
		else
		{
			hello.setText("Goodbye", juce::dontSendNotification);
		}
	}
private:
	APAudioProcessor& proc;
	FXBox fxa1Box{ "A1", proc, proc.fxOrderParams.fxa1 },
		fxa2Box{ "A2", proc, proc.fxOrderParams.fxa2 },
		fxa3Box{ "A3", proc, proc.fxOrderParams.fxa3 },
		fxa4Box{ "A4", proc, proc.fxOrderParams.fxa4 },
		fxb1Box{ "B1", proc, proc.fxOrderParams.fxb1 },
		fxb2Box{ "B2", proc, proc.fxOrderParams.fxb2 },
		fxb3Box{ "B3", proc, proc.fxOrderParams.fxb3 },
		fxb4Box{ "B4", proc, proc.fxOrderParams.fxb4 };
	gin::Select chainSetting{ proc.fxOrderParams.chainAtoB };
	gin::Select fxa1Selector{proc.fxOrderParams.fxa1},
		fxa2Selector{ proc.fxOrderParams.fxa2 },
		fxa3Selector{ proc.fxOrderParams.fxa3 },
		fxa4Selector{ proc.fxOrderParams.fxa4 },
		fxb1Selector{ proc.fxOrderParams.fxb1 },
		fxb2Selector{ proc.fxOrderParams.fxb2 },
		fxb3Selector{ proc.fxOrderParams.fxb3 },
		fxb4Selector{ proc.fxOrderParams.fxb4 };
	//gin::Layout layout{ *this };
};
