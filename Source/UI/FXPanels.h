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
#include "APModAdditions.h"
#include "DSP/PluginProcessor.h"

//==============================================================================
class FXBox : public gin::ParamBox {
public:
	FXBox(const juce::String &name,
	    APAudioProcessor &proc_)  // gin::Parameter::Ptr box_)
	    : gin::ParamBox(name), proc(proc_), dynamicsMeter(proc.compressor)
	{
		setName(name);
		// setTitle(name);

		// FX controls
		// WS = 1
		addControl(wsdrive = new APKnob(proc.waveshaperParams.drive), 0, 0);
		addControl(wsgain = new APKnob(proc.waveshaperParams.gain, true), 0, 1);
		addControl(wslp = new APKnob(proc.waveshaperParams.lp), 0, 2);
		addControl(wsdry = new APKnob(proc.waveshaperParams.dry), 1, 0);
		addControl(wswet = new APKnob(proc.waveshaperParams.wet), 1, 1);
		addControl(wstype = new gin::Select(proc.waveshaperParams.type), 1, 2);
		addControl(
		    wshsfreq = new APKnob(proc.waveshaperParams.highshelf), 2, 0);
		addControl(wshsq = new APKnob(proc.waveshaperParams.hsq), 2, 1);

		// CP = 2
		addControl(
		    cpthreshold = new APKnob(proc.compressorParams.threshold), 0, 0);
		addControl(cpratio = new APKnob(proc.compressorParams.ratio), 0, 1);
		addControl(cpattack = new APKnob(proc.compressorParams.attack), 0, 2);
		addControl(cprelease = new APKnob(proc.compressorParams.release), 1, 0);
		addControl(cpknee = new APKnob(proc.compressorParams.knee), 1, 1);
		addControl(cpinput = new APKnob(proc.compressorParams.input), 2, 0);
		addControl(cpoutput = new APKnob(proc.compressorParams.output), 2, 1);
		addControl(cptype = new gin::Select(proc.compressorParams.type), 2, 2);

		// DL = 3
		addControl(
		    dltimeleft = new APKnob(proc.stereoDelayParams.timeleft), 0, 0);
		addControl(
		    dlbeatsleft = new gin::Select(proc.stereoDelayParams.beatsleft), 0,
		    0);
		addControl(
		    dltimeright = new APKnob(proc.stereoDelayParams.timeright), 0, 1);
		addControl(
		    dlbeatsright = new gin::Select(proc.stereoDelayParams.beatsright),
		    0, 1);
		addControl(
		    dltemposync = new gin::Select(proc.stereoDelayParams.temposync), 0,
		    2);
		addControl(
		    dlfeedback = new APKnob(proc.stereoDelayParams.feedback), 1, 0);
		addControl(dlcutoff = new APKnob(proc.stereoDelayParams.cutoff), 1, 1);
		addControl(
		    dlpingpong = new gin::Select(proc.stereoDelayParams.pingpong), 1,
		    2);
		addControl(dldry = new APKnob(proc.stereoDelayParams.dry), 2, 0);
		addControl(dlwet = new APKnob(proc.stereoDelayParams.wet), 2, 1);
		addControl(
		    dlfreeze = new gin::Select(proc.stereoDelayParams.freeze), 2, 2);

		// CH = 4
		addControl(chrate = new APKnob(proc.chorusParams.rate), 0, 0);
		addControl(chdepth = new APKnob(proc.chorusParams.depth), 0, 1);
		addControl(chdelay = new APKnob(proc.chorusParams.delay), 0, 2);
		addControl(chfeedback = new APKnob(proc.chorusParams.feedback), 1, 0);
		addControl(chdry = new APKnob(proc.chorusParams.dry), 1, 1);
		addControl(chwet = new APKnob(proc.chorusParams.wet), 1, 2);

		// RV = 5
		addControl(rvsize = new APKnob(proc.reverbParams.size), 0, 0);
		addControl(rvdecay = new APKnob(proc.reverbParams.decay), 0, 1);
		addControl(rvdamping = new APKnob(proc.reverbParams.damping), 0, 2);
		addControl(rvlowpass = new APKnob(proc.reverbParams.lowpass), 1, 0);
		addControl(rvpredelay = new APKnob(proc.reverbParams.predelay), 1, 1);
		addControl(rvdry = new APKnob(proc.reverbParams.dry), 1, 2);
		addControl(rvwet = new APKnob(proc.reverbParams.wet), 2, 0);

		// MB = 6
		addControl(
		    mbfilterlowshelffreq = new APKnob(proc.mbfilterParams.lowshelffreq),
		    0, 0);
		addControl(
		    mbfilterlowshelfgain = new APKnob(proc.mbfilterParams.lowshelfgain),
		    0, 1);
		addControl(
		    mbfilterlowshelfq = new APKnob(proc.mbfilterParams.lowshelfq), 0,
		    2);
		addControl(
		    mbfilterpeakfreq = new APKnob(proc.mbfilterParams.peakfreq), 1, 0);
		addControl(
		    mbfilterpeakgain = new APKnob(proc.mbfilterParams.peakgain), 1, 1);
		addControl(mbfilterpeakq = new APKnob(proc.mbfilterParams.peakq), 1, 2);
		addControl(mbfilterhighshelffreq =
		               new APKnob(proc.mbfilterParams.highshelffreq),
		    2, 0);
		addControl(mbfilterhighshelfgain =
		               new APKnob(proc.mbfilterParams.highshelfgain),
		    2, 1);
		addControl(
		    mbfilterhighshelfq = new APKnob(proc.mbfilterParams.highshelfq), 2,
		    2);

		// RM = 7
		addControl(rmmodfreq1 = new APKnob(proc.ringmodParams.modfreq1), 0, 0);
		addControl(rmshape1 = new APKnob(proc.ringmodParams.shape1), 1, 0);
		addControl(rmmix1 = new APKnob(proc.ringmodParams.mix1), 2, 0);
		addControl(rmmodfreq2 = new APKnob(proc.ringmodParams.modfreq2), 0, 1);
		addControl(rmshape2 = new APKnob(proc.ringmodParams.shape2), 1, 1);
		addControl(rmmix2 = new APKnob(proc.ringmodParams.mix2), 2, 1);
		addControl(rmspread = new APKnob(proc.ringmodParams.spread), 0, 2);
		addControl(rmlowcut = new APKnob(proc.ringmodParams.lowcut), 1, 2);
		addControl(rmhighcut = new APKnob(proc.ringmodParams.highcut), 2, 2);

		// GN = 8
		addControl(gngain = new APKnob(proc.gainParams.gain), 1, 1);

		// LDR = 9
		addControl(ldrcutoff = new APKnob(proc.ladderParams.cutoff), 0, 0);
		addControl(ldrreso = new APKnob(proc.ladderParams.reso), 0, 1);
		addControl(ldrdrive = new APKnob(proc.ladderParams.drive), 0, 2);
		addControl(ldrtype = new APKnob(proc.ladderParams.type), 1, 0);
		addControl(ldrgain = new APKnob(proc.ladderParams.gain), 1, 1);


		addAndMakeVisible(dynamicsMeter);
		addAndMakeVisible(funcImage);

		watchParam(proc.stereoDelayParams.temposync);
		watchParam(proc.waveshaperParams.type);

		setSize(300, 233);
		hideAll();
	}

	void paramChanged() override
	{
		gin::ParamBox::paramChanged();

		if (currentEffect == 3) {
			const auto choice = proc.stereoDelayParams.temposync->getUserValueBool();
			dlbeatsleft->setVisible(choice);
			dlbeatsright->setVisible(choice);
			dltimeleft->setVisible(!choice);
			dltimeright->setVisible(!choice);
		}

		if (currentEffect == 1) {
			switch (proc.waveshaperParams.type->getUserValueInt()) {
				case 0:  // "Soft Clip"
					funcImage.setImage(juce::ImageCache::getFromMemory(
					    BinaryData::sine_png, BinaryData::sine_pngSize));
					break;
				case 1: // "Tanh"
					funcImage.setImage(juce::ImageCache::getFromMemory(
						BinaryData::tanh4_png, BinaryData::tanh4_pngSize));
					break;
				case 2: // "Hard Clip"
                    funcImage.setImage(juce::ImageCache::getFromMemory(
                        BinaryData::clipping_png,
                        BinaryData::clipping_pngSize));
					break;
				case 3: // "Halfwave"
                    funcImage.setImage(juce::ImageCache::getFromMemory(
                        BinaryData::halfwave_png,
                        BinaryData::halfwave_pngSize));
					break;
				case 4: // "Fullwave"
                    funcImage.setImage(juce::ImageCache::getFromMemory(
                        BinaryData::fullwave_png,
                        BinaryData::fullwave_pngSize));
					break;
				case 5: // "Folder"
                    funcImage.setImage(juce::ImageCache::getFromMemory(
                        BinaryData::cheb5_png, BinaryData::cheb5_pngSize));
					break;
			}
			funcImage.setVisible(true);
		}
	}

	void resized() override
	{
		gin::ParamBox::resized();
		dynamicsMeter.setBounds(56, 163, 56, 70);
		funcImage.setBounds(56 * 2, 23 + 140, 55, 55);
	}

	void setControls(int effect)
	{
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
				wslp->setVisible(true);
				wswet->setVisible(true);
				wstype->setVisible(true);
				wshsfreq->setVisible(true);
				wshsq->setVisible(true);
				paramChanged();  // ensure function image is initialized
				funcImage.setVisible(true);
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
				dynamicsMeter.setVisible(true);
				break;
			case 3:
				if (!proc.stereoDelayParams.temposync->getUserValueBool()) {
					dltimeleft->setVisible(true);
					dltimeright->setVisible(true);
				} else {
					dlbeatsleft->setVisible(true);
					dlbeatsright->setVisible(true);
				}
				dltemposync->setVisible(true);
				dlfeedback->setVisible(true);
				dlwet->setVisible(true);
				dldry->setVisible(true);
				dlpingpong->setVisible(true);
				dlfreeze->setVisible(true);
				dlcutoff->setVisible(true);
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
			case 9:
				ldrcutoff->setVisible(true);
				ldrreso->setVisible(true);
				ldrdrive->setVisible(true);
				ldrtype->setVisible(true);
				ldrgain->setVisible(true);
				break;
		}
	}

	void hideAll()
	{
		// WS = 1
		wsdrive->setVisible(false);
		wsgain->setVisible(false);
		wsdry->setVisible(false);
		wslp->setVisible(false);
		wswet->setVisible(false);
		wstype->setVisible(false);
		wshsfreq->setVisible(false);
		wshsq->setVisible(false);
		funcImage.setVisible(false);
		// CP = 2
		cpthreshold->setVisible(false);
		cpratio->setVisible(false);
		cpattack->setVisible(false);
		cprelease->setVisible(false);
		cpknee->setVisible(false);
		cpinput->setVisible(false);
		cpoutput->setVisible(false);
		cptype->setVisible(false);
		dynamicsMeter.setVisible(false);
		// DL = 3
		dltimeleft->setVisible(false);
		dltimeright->setVisible(false);
		dlbeatsleft->setVisible(false);
		dlbeatsright->setVisible(false);
		dltemposync->setVisible(false);
		dlfeedback->setVisible(false);
		dlwet->setVisible(false);
		dldry->setVisible(false);
		dlpingpong->setVisible(false);
		dlfreeze->setVisible(false);
		dlcutoff->setVisible(false);
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
		// LDR = 9
		ldrcutoff->setVisible(false);
		ldrreso->setVisible(false);
		ldrdrive->setVisible(false);
		ldrtype->setVisible(false);
		ldrgain->setVisible(false);
	}

	APAudioProcessor &proc;
	gin::ParamComponent::Ptr rmmodfreq1, rmmodfreq2, rmshape1, rmshape2, rmmix1,
	    rmmix2, rmspread, rmlowcut, rmhighcut;
	gin::ParamComponent::Ptr wsdrive, wsgain, wsdry, wslp, wswet, wstype,
	    wshsfreq, wshsq;
	gin::ParamComponent::Ptr gngain;
	gin::ParamComponent::Ptr cpthreshold, cpratio, cpattack, cprelease, cpknee,
	    cpinput, cpoutput, cptype;
	gin::ParamComponent::Ptr dltimeleft, dltimeright, dlbeatsleft, dlbeatsright,
	    dltemposync, dlfeedback, dldry, dlwet, dlpingpong, dlfreeze, dlcutoff;
	gin::ParamComponent::Ptr chrate, chdepth, chdelay, chfeedback, chdry, chwet;
	gin::ParamComponent::Ptr rvsize, rvdecay, rvdamping, rvlowpass, rvpredelay,
	    rvdry, rvwet;
	gin::ParamComponent::Ptr mbfilterlowshelffreq, mbfilterlowshelfgain,
	    mbfilterlowshelfq, mbfilterpeakfreq, mbfilterpeakgain, mbfilterpeakq,
	    mbfilterhighshelffreq, mbfilterhighshelfgain, mbfilterhighshelfq;
	gin::ParamComponent::Ptr ldrcutoff, ldrreso, ldrdrive, ldrtype, ldrgain;
	gin::DynamicsMeter dynamicsMeter;
	juce::ImageComponent funcImage{"function"};
	int currentEffect = 0;
};

//==============================================================================
class FXModBox : public gin::ParamBox {
public:
	FXModBox(const juce::String &name, APAudioProcessor &proc_)
	    : gin::ParamBox(name), proc(proc_)
	{
		setName("mod");
		setTitle("mod sources");
		addControl(
		    srclist = new gin::ModSrcListBox(proc.modMatrix), 0, 0, 5, 4);
		srclist->setRowHeight(20);
	}
	gin::ModSrcListBox *srclist;
	APAudioProcessor &proc;
};

class MatrixBox : public gin::ParamBox {
public:
	MatrixBox(const juce::String &name, APAudioProcessor &proc_)
	    : gin::ParamBox(name), proc(proc_)
	{
		setName("mtx");

		addControl(new APModMatrixBox(proc, proc.modMatrix), 0, 0, 5, 4);
		addAndMakeVisible(clearAllButton);
		clearAllButton.onClick = [this] { clearAll(); };
	}

	void resized() override
	{
		gin::ParamBox::resized();
		clearAllButton.setBounds(getWidth() - 60, 0, 55, 23);
	}

	void clearAll()
	{
		auto &pluginParams = proc.getPluginParameters();
		for (auto *param : pluginParams) {
			if (param->getModIndex() == -1)
				continue;
			if (proc.modMatrix.isModulated(
			        gin::ModDstId(param->getModIndex()))) {
				auto modSrcs = proc.modMatrix.getModSources(param);
				for (const auto &modSrc : modSrcs) {
					proc.modMatrix.clearModDepth(modSrc, gin::ModDstId(param->getModIndex()));
				}
			}
		}
	}

	juce::TextButton clearAllButton{"Clear All"};

	APAudioProcessor &proc;
};

