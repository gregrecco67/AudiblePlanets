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
#include "MacrosPanels.h"

 //==============================================================================
class MacrosEditor : public juce::Component
{
public:
	MacrosEditor(APAudioProcessor& proc_);

	void resized() override;
	//void timerCallback() override;

	class ParameterSelector : public juce::Label
	{
	public:
		ParameterSelector(APAudioProcessor& p_) : proc(p_) {
			setEditable(false, true, false);
			setJustificationType(Justification::left);
			setText("Select", dontSendNotification);

			// build hierarchical popup menu of parameters
			PopupMenu oscsMenu, osc1Menu, osc2Menu, osc3Menu, osc4Menu;
			PopupMenu lfosMenu, lfo1Menu, lfo2Menu, lfo3Menu, lfo4Menu;
			PopupMenu envsMenu, env1Menu, env2Menu, env3Menu, env4Menu;
			PopupMenu filterMenu;
			PopupMenu msegsMenu, mseg1Menu, mseg2Menu, mseg3Menu, mseg4Menu;
			PopupMenu timbreMenu; // equant, pitch, blend, demodmix, demodvol, algo
			PopupMenu globalMenu; // level, squash
			PopupMenu fxMenu;
			PopupMenu waveshaperMenu;
			PopupMenu compressorMenu;
			PopupMenu delayMenu;
			PopupMenu reverbMenu;
			PopupMenu mbeqMenu;
			PopupMenu chorusMenu;
			PopupMenu ringmodMenu;

			osc1Menu.addItem("OSC1 Coarse", [this]() { setDest(proc.osc1Params.coarse); });
			osc1Menu.addItem("OSC1 Fine", [this]() { setDest(proc.osc1Params.fine); });
			osc1Menu.addItem("OSC1 Volume", [this]() { setDest(proc.osc1Params.volume); });
			osc1Menu.addItem("OSC1 Tones", [this]() { setDest(proc.osc1Params.tones); });
			osc1Menu.addItem("OSC1 Phase", [this]() { setDest(proc.osc1Params.phase); });
			osc1Menu.addItem("OSC1 Detune", [this]() { setDest(proc.osc1Params.detune); });
			osc1Menu.addItem("OSC1 Spread", [this]() { setDest(proc.osc1Params.spread); });
			osc1Menu.addItem("OSC1 Pan", [this]() { setDest(proc.osc1Params.pan); });

			osc2Menu.addItem("OSC2 Coarse", [this]() { setDest(proc.osc2Params.coarse); });
			osc2Menu.addItem("OSC2 Fine", [this]() { setDest(proc.osc2Params.fine); });
			osc2Menu.addItem("OSC2 Volume", [this]() { setDest(proc.osc2Params.volume); });
			osc2Menu.addItem("OSC2 Tones", [this]() { setDest(proc.osc2Params.tones); });
			osc2Menu.addItem("OSC2 Phase", [this]() { setDest(proc.osc2Params.phase); });
			osc2Menu.addItem("OSC2 Detune", [this]() { setDest(proc.osc2Params.detune); });
			osc2Menu.addItem("OSC2 Spread", [this]() { setDest(proc.osc2Params.spread); });
			osc2Menu.addItem("OSC2 Pan", [this]() { setDest(proc.osc2Params.pan); });

			osc3Menu.addItem("OSC3 Coarse", [this]() { setDest(proc.osc3Params.coarse); });
			osc3Menu.addItem("OSC3 Fine", [this]() { setDest(proc.osc3Params.fine); });
			osc3Menu.addItem("OSC3 Volume", [this]() { setDest(proc.osc3Params.volume); });
			osc3Menu.addItem("OSC3 Tones", [this]() { setDest(proc.osc3Params.tones); });
			osc3Menu.addItem("OSC3 Phase", [this]() { setDest(proc.osc3Params.phase); });
			osc3Menu.addItem("OSC3 Detune", [this]() { setDest(proc.osc3Params.detune); });
			osc3Menu.addItem("OSC3 Spread", [this]() { setDest(proc.osc3Params.spread); });
			osc3Menu.addItem("OSC3 Pan", [this]() { setDest(proc.osc3Params.pan); });

			osc4Menu.addItem("OSC4 Coarse", [this]() { setDest(proc.osc4Params.coarse); });
			osc4Menu.addItem("OSC4 Fine", [this]() { setDest(proc.osc4Params.fine); });
			osc4Menu.addItem("OSC4 Volume", [this]() { setDest(proc.osc4Params.volume); });
			osc4Menu.addItem("OSC4 Tones", [this]() { setDest(proc.osc4Params.tones); });
			osc4Menu.addItem("OSC4 Phase", [this]() { setDest(proc.osc4Params.phase); });
			osc4Menu.addItem("OSC4 Detune", [this]() { setDest(proc.osc4Params.detune); });
			osc4Menu.addItem("OSC4 Spread", [this]() { setDest(proc.osc4Params.spread); });
			osc4Menu.addItem("OSC4 Pan", [this]() { setDest(proc.osc4Params.pan); });

			oscsMenu.addSubMenu("OSC1", osc1Menu);
			oscsMenu.addSubMenu("OSC2", osc2Menu);
			oscsMenu.addSubMenu("OSC3", osc3Menu);
			oscsMenu.addSubMenu("OSC4", osc4Menu);

			lfo1Menu.addItem("LFO1 Rate", [this]() { setDest(proc.lfo1Params.rate); });
			lfo1Menu.addItem("LFO1 Beats", [this]() { setDest(proc.lfo1Params.beat); });
			lfo1Menu.addItem("LFO1 Depth", [this]() { setDest(proc.lfo1Params.depth); });
			lfo1Menu.addItem("LFO1 Offset", [this]() { setDest(proc.lfo1Params.offset); });

			lfo2Menu.addItem("LFO2 Rate", [this]() { setDest(proc.lfo2Params.rate); });
			lfo2Menu.addItem("LFO2 Beats", [this]() { setDest(proc.lfo2Params.beat); });
			lfo2Menu.addItem("LFO2 Depth", [this]() { setDest(proc.lfo2Params.depth); });
			lfo2Menu.addItem("LFO2 Offset", [this]() { setDest(proc.lfo2Params.offset); });

			lfo3Menu.addItem("LFO3 Rate", [this]() { setDest(proc.lfo3Params.rate); });
			lfo3Menu.addItem("LFO3 Beats", [this]() { setDest(proc.lfo3Params.beat); });
			lfo3Menu.addItem("LFO3 Depth", [this]() { setDest(proc.lfo3Params.depth); });
			lfo3Menu.addItem("LFO3 Offset", [this]() { setDest(proc.lfo3Params.offset); });

			lfo4Menu.addItem("LFO4 Rate", [this]() { setDest(proc.lfo4Params.rate); });
			lfo4Menu.addItem("LFO4 Beats", [this]() { setDest(proc.lfo4Params.beat); });
			lfo4Menu.addItem("LFO4 Depth", [this]() { setDest(proc.lfo4Params.depth); });
			lfo4Menu.addItem("LFO4 Offset", [this]() { setDest(proc.lfo4Params.offset); });

			lfosMenu.addSubMenu("LFO1", lfo1Menu);
			lfosMenu.addSubMenu("LFO2", lfo2Menu);
			lfosMenu.addSubMenu("LFO3", lfo3Menu);
			lfosMenu.addSubMenu("LFO4", lfo4Menu);

			env1Menu.addItem("ENV1 Attack", [this]() { setDest(proc.env1Params.attack); });
			env1Menu.addItem("ENV1 Decay", [this]() { setDest(proc.env1Params.decay); });
			env1Menu.addItem("ENV1 Sustain", [this]() { setDest(proc.env1Params.sustain); });
			env1Menu.addItem("ENV1 Release", [this]() { setDest(proc.env1Params.release); });
			env1Menu.addItem("ENV1 Time", [this]() { setDest(proc.env1Params.time); });
			env1Menu.addItem("ENV1 Beats", [this]() { setDest(proc.env1Params.duration); });
			env1Menu.addItem("ENV1 Atk Curve", [this]() { setDest(proc.env1Params.acurve); });
			env1Menu.addItem("ENV1 D/R Curve", [this]() { setDest(proc.env1Params.drcurve); });

			env2Menu.addItem("ENV2 Attack", [this]() { setDest(proc.env2Params.attack); });
			env2Menu.addItem("ENV2 Decay", [this]() { setDest(proc.env2Params.decay); });
			env2Menu.addItem("ENV2 Sustain", [this]() { setDest(proc.env2Params.sustain); });
			env2Menu.addItem("ENV2 Release", [this]() { setDest(proc.env2Params.release); });
			env2Menu.addItem("ENV2 Time", [this]() { setDest(proc.env2Params.time); });
			env2Menu.addItem("ENV2 Beats", [this]() { setDest(proc.env2Params.duration); });
			env2Menu.addItem("ENV2 Atk Curve", [this]() { setDest(proc.env2Params.acurve); });
			env2Menu.addItem("ENV2 D/R Curve", [this]() { setDest(proc.env2Params.drcurve); });

			env3Menu.addItem("ENV3 Attack", [this]() { setDest(proc.env3Params.attack); });
			env3Menu.addItem("ENV3 Decay", [this]() { setDest(proc.env3Params.decay); });
			env3Menu.addItem("ENV3 Sustain", [this]() { setDest(proc.env3Params.sustain); });
			env3Menu.addItem("ENV3 Release", [this]() { setDest(proc.env3Params.release); });
			env3Menu.addItem("ENV3 Time", [this]() { setDest(proc.env3Params.time); });
			env3Menu.addItem("ENV3 Beats", [this]() { setDest(proc.env3Params.duration); });
			env3Menu.addItem("ENV3 Atk Curve", [this]() { setDest(proc.env3Params.acurve); });
			env3Menu.addItem("ENV3 D/R Curve", [this]() { setDest(proc.env3Params.drcurve); });

			env4Menu.addItem("ENV4 Attack", [this]() { setDest(proc.env4Params.attack); });
			env4Menu.addItem("ENV4 Decay", [this]() { setDest(proc.env4Params.decay); });
			env4Menu.addItem("ENV4 Sustain", [this]() { setDest(proc.env4Params.sustain); });
			env4Menu.addItem("ENV4 Release", [this]() { setDest(proc.env4Params.release); });
			env4Menu.addItem("ENV4 Time", [this]() { setDest(proc.env4Params.time); });
			env4Menu.addItem("ENV4 Beats", [this]() { setDest(proc.env4Params.duration); });
			env4Menu.addItem("ENV4 Atk Curve", [this]() { setDest(proc.env4Params.acurve); });
			env4Menu.addItem("ENV4 D/R Curve", [this]() { setDest(proc.env4Params.drcurve); });

			envsMenu.addSubMenu("ENV1", env1Menu);
			envsMenu.addSubMenu("ENV2", env2Menu);
			envsMenu.addSubMenu("ENV3", env3Menu);
			envsMenu.addSubMenu("ENV4", env4Menu);


			m.addSubMenu("Oscillators", oscsMenu);
			m.addSubMenu("Envelopes", envsMenu);
			m.addSubMenu("LFOs", lfosMenu);

		}

		void setDest(gin::Parameter::Ptr p) {
			setText(p->getName(50), dontSendNotification);
		}

		void mouseDown(const MouseEvent& e) override {
			m.show();
		}

		PopupMenu m;
		std::function<void(MacrosEditor&)> onClick;
		APAudioProcessor& proc;
		

	};

private:
	APAudioProcessor& proc;
	
	MacrosMatrixBox matrix{ "matrix", proc };

	gin::Layout layout{ *this };

	ParameterSelector paramSelector1{ proc }, paramSelector2{ proc }, paramSelector3{ proc }, paramSelector4{ proc };
};

// TODO: change those mod depth sliders