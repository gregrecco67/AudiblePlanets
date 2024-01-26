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

        //    //==============================================================================
        //    void APAudioProcessor::GainParams::setup(APAudioProcessor& p)
        //        gain = p.addExtParam("gain", "Gain", "", "", { -60.0, 40.0, 0.0, 1.0 }, 1.0f, 0.0f);
        addControl(modfreq1 = new gin::Knob(proc.ringmodParams.modfreq1), 0, 0);
        addControl(shape1 = new gin::Knob(proc.ringmodParams.shape1), 1, 0);
        addControl(mix1 = new gin::Knob(proc.ringmodParams.mix1), 2, 0);
        addControl(modfreq2 = new gin::Knob(proc.ringmodParams.modfreq2), 0, 1);
        addControl(shape2 = new gin::Knob(proc.ringmodParams.shape2), 1, 1);
        addControl(mix2 = new gin::Knob(proc.ringmodParams.mix2), 2, 1);
        addControl(spread = new gin::Knob(proc.ringmodParams.spread), 0, 2);
        addControl(lowcut = new gin::Knob(proc.ringmodParams.lowcut), 1, 2);
        addControl(highcut = new gin::Knob(proc.ringmodParams.highcut), 2, 2);
		
		setSize(300, 233);
        hideAll();
	}

    
	void setControls(int effect) {
		gin::ParamBox::resized();
        if (effect == 0)
            hideAll();
        if (effect == 7) {
            modfreq1->setVisible(true);
            modfreq2->setVisible(true);
            shape1->setVisible(true);
            shape2->setVisible(true);
            mix1->setVisible(true);
            mix2->setVisible(true);
            spread->setVisible(true);
            lowcut->setVisible(true);
            highcut->setVisible(true);
        }
	}
    
    void hideAll() {
        modfreq1->setVisible(false);
        modfreq2->setVisible(false);
        shape1->setVisible(false);
        shape2->setVisible(false);
        mix1->setVisible(false);
        mix2->setVisible(false);
        spread->setVisible(false);
        lowcut->setVisible(false);
        highcut->setVisible(false);
    }
	
    APAudioProcessor& proc;
    gin::ParamComponent::Ptr modfreq1, modfreq2, shape1, shape2, mix1, mix2, spread, lowcut, highcut;
    gin::Parameter::Ptr box;
    std::array<gin::ParamComponent*, 9> ringmodComponents { modfreq1, modfreq2, shape1, shape2, mix1, mix2,
        spread, lowcut, highcut};

    
};
