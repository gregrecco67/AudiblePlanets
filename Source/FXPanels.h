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
		 
		// WS = 1
		addControl(wsdrive = new gin::Knob(proc.waveshaperParams.drive), 0, 0);
		addControl(wsgain = new gin::Knob(proc.waveshaperParams.gain), 0, 1);
		addControl(wsdry = new gin::Knob(proc.waveshaperParams.dry), 1, 0);
		addControl(wswet = new gin::Knob(proc.waveshaperParams.wet), 1, 1);
		addControl(wstype = new gin::Select(proc.waveshaperParams.type), 0, 2, 2, 1);

		// CP = 2
		addControl(cpthreshold = new gin::Knob(proc.compressorParams.threshold), 0, 0);
		addControl(cpratio = new gin::Knob(proc.compressorParams.ratio), 0, 1);
		addControl(cpattack = new gin::Knob(proc.compressorParams.attack), 0, 2);
		addControl(cprelease = new gin::Knob(proc.compressorParams.release), 1, 0);
		addControl(cpknee = new gin::Knob(proc.compressorParams.knee), 1, 1);
		addControl(cpinput = new gin::Knob(proc.compressorParams.input), 2, 0);
		addControl(cpoutput = new gin::Knob(proc.compressorParams.output), 2, 1);
		addControl(cptype = new gin::Select(proc.compressorParams.type), 2, 2);

		// DL = 3
		addControl(dltimeleft = new gin::Knob(proc.stereoDelayParams.timeleft), 0, 0);
		addControl(dlbeatsleft = new gin::Knob(proc.stereoDelayParams.beatsleft), 0, 0);
		addControl(dltimeright = new gin::Knob(proc.stereoDelayParams.timeright), 0, 1);
		addControl(dlbeatsright = new gin::Knob(proc.stereoDelayParams.beatsright), 0, 1);
		addControl(dltemposync = new gin::Switch(proc.stereoDelayParams.temposync), 0, 2);
		addControl(dlfeedback = new gin::Knob(proc.stereoDelayParams.feedback), 1, 0);
		addControl(dlwet = new gin::Knob(proc.stereoDelayParams.wet), 1, 1);
		addControl(dlpingpong = new gin::Switch(proc.stereoDelayParams.pingpong), 1, 2);
		addControl(dlfreeze = new gin::Switch(proc.stereoDelayParams.freeze), 2, 2);

		// CH = 4
		addControl(chrate = new gin::Knob(proc.chorusParams.rate), 0, 0);
		addControl(chdepth = new gin::Knob(proc.chorusParams.depth), 0, 1);
		addControl(chdelay = new gin::Knob(proc.chorusParams.delay), 0, 2);
		addControl(chfeedback = new gin::Knob(proc.chorusParams.feedback), 1, 0);
		addControl(chdry = new gin::Knob(proc.chorusParams.dry), 1, 1);
		addControl(chwet = new gin::Knob(proc.chorusParams.wet), 1, 2);

		// RV = 5
		addControl(rvsize = new gin::Knob(proc.reverbParams.size), 0, 0);
		addControl(rvdecay = new gin::Knob(proc.reverbParams.decay), 0, 1);
		addControl(rvdamping = new gin::Knob(proc.reverbParams.damping), 0, 2);
		addControl(rvlowpass = new gin::Knob(proc.reverbParams.lowpass), 1, 0);
		addControl(rvpredelay = new gin::Knob(proc.reverbParams.predelay), 1, 1);
		addControl(rvdry = new gin::Knob(proc.reverbParams.dry), 1, 2);
		addControl(rvwet = new gin::Knob(proc.reverbParams.wet), 2, 0);

		// MB = 6
		addControl(mbfilterlowshelffreq = new gin::Knob(proc.mbfilterParams.lowshelffreq), 0, 0);
		addControl(mbfilterlowshelfgain = new gin::Knob(proc.mbfilterParams.lowshelfgain), 0, 1);
		addControl(mbfilterlowshelfq = new gin::Knob(proc.mbfilterParams.lowshelfq), 0, 2);
		addControl(mbfilterpeakfreq = new gin::Knob(proc.mbfilterParams.peakfreq), 1, 0);
		addControl(mbfilterpeakgain = new gin::Knob(proc.mbfilterParams.peakgain), 1, 1);
		addControl(mbfilterpeakq = new gin::Knob(proc.mbfilterParams.peakq), 1, 2);
		addControl(mbfilterhighshelffreq = new gin::Knob(proc.mbfilterParams.highshelffreq), 2, 0);
		addControl(mbfilterhighshelfgain = new gin::Knob(proc.mbfilterParams.highshelfgain), 2, 1);
		addControl(mbfilterhighshelfq = new gin::Knob(proc.mbfilterParams.highshelfq), 2, 2);

        // RM = 7
		addControl(rmmodfreq1 = new gin::Knob(proc.ringmodParams.modfreq1), 0, 0);
        addControl(rmshape1 = new gin::Knob(proc.ringmodParams.shape1), 1, 0);
        addControl(rmmix1 = new gin::Knob(proc.ringmodParams.mix1), 2, 0);
        addControl(rmmodfreq2 = new gin::Knob(proc.ringmodParams.modfreq2), 0, 1);
        addControl(rmshape2 = new gin::Knob(proc.ringmodParams.shape2), 1, 1);
        addControl(rmmix2 = new gin::Knob(proc.ringmodParams.mix2), 2, 1);
        addControl(rmspread = new gin::Knob(proc.ringmodParams.spread), 0, 2);
        addControl(rmlowcut = new gin::Knob(proc.ringmodParams.lowcut), 1, 2);
        addControl(rmhighcut = new gin::Knob(proc.ringmodParams.highcut), 2, 2);

		// GN = 8
		addControl(gngain = new gin::Knob(proc.gainParams.gain), 1, 1);
		
		watchParam(proc.stereoDelayParams.temposync);
		

		setSize(300, 233);
		hideAll();
	}

	void paramChanged() override
	{
		gin::ParamBox::paramChanged();

		if (currentEffect == 3)
		{
			auto choice = proc.stereoDelayParams.temposync->getUserValueBool();
			dlbeatsleft->setVisible(choice);
			dlbeatsright->setVisible(choice);
			dltimeleft->setVisible(!choice);
			dltimeright->setVisible(!choice);
		}
	}


	void setControls(int effect) {
		currentEffect = effect;
		gin::ParamBox::resized();
		hideAll();
		switch (effect) {
		case 0:
			break;
		case 1:
			wsdrive->setVisible(true);
			wsgain->setVisible(true);
			wsdry->setVisible(true);
			wswet->setVisible(true);
			wstype->setVisible(true);
			break;
		case 2:
			cpthreshold->setVisible(true);
			cpratio->setVisible(true);
			cpattack->setVisible(true);
			cprelease->setVisible(true);
			cpknee->setVisible(true);
			cpinput->setVisible(true);
			cpoutput->setVisible(true);
			cptype->setVisible(true);
			break;
		case 3:
			if (!proc.stereoDelayParams.temposync->getUserValueBool()) {
				dltimeleft->setVisible(true);
				dltimeright->setVisible(true);
			}
			else {
				dlbeatsleft->setVisible(true);
				dlbeatsright->setVisible(true);
			}
			dltemposync->setVisible(true);
			dlfeedback->setVisible(true);
			dlwet->setVisible(true);
			dlpingpong->setVisible(true);
			dlfreeze->setVisible(true);
			break;
		case 4:
			chrate->setVisible(true);
			chdepth->setVisible(true);
			chdelay->setVisible(true);
			chfeedback->setVisible(true);
			chdry->setVisible(true);
			chwet->setVisible(true);
			break;
		case 5:
			mbfilterlowshelffreq->setVisible(true);
			mbfilterlowshelfgain->setVisible(true);
			mbfilterlowshelfq->setVisible(true);
			mbfilterpeakfreq->setVisible(true);
			mbfilterpeakgain->setVisible(true);
			mbfilterpeakq->setVisible(true);
			mbfilterhighshelffreq->setVisible(true);
			mbfilterhighshelfgain->setVisible(true);
			mbfilterhighshelfq->setVisible(true);
			break;
		case 6:
			rvsize->setVisible(true);
			rvdecay->setVisible(true);
			rvdamping->setVisible(true);
			rvlowpass->setVisible(true);
			rvpredelay->setVisible(true);
			rvdry->setVisible(true);
			rvwet->setVisible(true);
			break;
		case 7:
			rmmodfreq1->setVisible(true);
			rmmodfreq2->setVisible(true);
			rmshape1->setVisible(true);
			rmshape2->setVisible(true);
			rmmix1->setVisible(true);
			rmmix2->setVisible(true);
			rmspread->setVisible(true);
			rmlowcut->setVisible(true);
			rmhighcut->setVisible(true);
			break;
		case 8:
			gngain->setVisible(true);
			break;
		}
	}
    
    void hideAll() {
		// WS = 1
		wsdrive->setVisible(false);
		wsgain->setVisible(false);
		wsdry->setVisible(false);
		wswet->setVisible(false);
		wstype->setVisible(false);
		// CP = 2
		cpthreshold->setVisible(false);
		cpratio->setVisible(false);
		cpattack->setVisible(false);
		cprelease->setVisible(false);
		cpknee->setVisible(false);
		cpinput->setVisible(false);
		cpoutput->setVisible(false);
		cptype->setVisible(false);
		// DL = 3
		dltimeleft->setVisible(false);
		dltimeright->setVisible(false);
		dlbeatsleft->setVisible(false);
		dlbeatsright->setVisible(false);
		dltemposync->setVisible(false);
		dlfeedback->setVisible(false);
		dlwet->setVisible(false);
		dlpingpong->setVisible(false);
		dlfreeze->setVisible(false);
		// CH = 4
		chrate->setVisible(false);
		chdepth->setVisible(false);
		chdelay->setVisible(false);
		chfeedback->setVisible(false);
		chdry->setVisible(false);
		chwet->setVisible(false);
		// MB = 5
		mbfilterlowshelffreq->setVisible(false);
		mbfilterlowshelfgain->setVisible(false);
		mbfilterlowshelfq->setVisible(false);
		mbfilterpeakfreq->setVisible(false);
		mbfilterpeakgain->setVisible(false);
		mbfilterpeakq->setVisible(false);
		mbfilterhighshelffreq->setVisible(false);
		mbfilterhighshelfgain->setVisible(false);
		mbfilterhighshelfq->setVisible(false);
		// RV = 6
		rvsize->setVisible(false);
		rvdecay->setVisible(false);
		rvdamping->setVisible(false);
		rvlowpass->setVisible(false);
		rvpredelay->setVisible(false);
		rvdry->setVisible(false);
		rvwet->setVisible(false);
		// RM = 7
		rmmodfreq1->setVisible(false);
        rmmodfreq2->setVisible(false);
        rmshape1->setVisible(false);
        rmshape2->setVisible(false);
        rmmix1->setVisible(false);
        rmmix2->setVisible(false);
        rmspread->setVisible(false);
        rmlowcut->setVisible(false);
        rmhighcut->setVisible(false);
		// GN = 8
		gngain->setVisible(false);
    }
	
    APAudioProcessor& proc;
    gin::ParamComponent::Ptr rmmodfreq1, rmmodfreq2, rmshape1, rmshape2, rmmix1, rmmix2, rmspread, rmlowcut, rmhighcut;
	gin::ParamComponent::Ptr wsdrive, wsgain, wsdry, wswet, wstype;
	gin::ParamComponent::Ptr gngain;
	gin::ParamComponent::Ptr cpthreshold, cpratio, cpattack, cprelease, cpknee, cpinput, cpoutput, cptype;
	gin::ParamComponent::Ptr dltimeleft, dltimeright, dlbeatsleft, dlbeatsright, dltemposync, dlfeedback, dlwet, dlpingpong, dlfreeze;
	gin::ParamComponent::Ptr chrate, chdepth, chdelay, chfeedback, chdry, chwet;
	gin::ParamComponent::Ptr rvsize, rvdecay, rvdamping, rvlowpass, rvpredelay, rvdry, rvwet;
	gin::ParamComponent::Ptr mbfilterlowshelffreq, mbfilterlowshelfgain, mbfilterlowshelfq, mbfilterpeakfreq, mbfilterpeakgain, mbfilterpeakq, mbfilterhighshelffreq, mbfilterhighshelfgain, mbfilterhighshelfq;

	int currentEffect = 0;
    gin::Parameter::Ptr box;
        
};

//==============================================================================
class FXModBox : public gin::ParamBox
{
public:
    FXModBox (const juce::String& name, APAudioProcessor& proc_)
        : gin::ParamBox (name), proc (proc_)
    {
        setName ("mod");
        setTitle("mod sources");
        addControl (new gin::ModSrcListBox (proc.modMatrix), 0, 0, 3, 4);
    }

    APAudioProcessor& proc;
};

class MatrixBox : public gin::ParamBox
{
public:
	MatrixBox(const juce::String& name, APAudioProcessor& proc_)
		: gin::ParamBox(name), proc(proc_)
	{
		setName("mtx");

		addControl(new gin::ModMatrixBox(proc, proc.modMatrix), 0, 0, 3, 2);
	}

	APAudioProcessor& proc;
};