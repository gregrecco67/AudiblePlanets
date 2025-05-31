#include "PluginProcessor.h"
#include "Synth.h"

APSynth::APSynth(APAudioProcessor &proc_) : proc(proc_)
{
	enableLegacyMode(12);
	setVoiceStealingEnabled(true);

	for (int i = 0; i < 16; i++) {
		auto voice = new SynthVoice3(proc);
		proc.modMatrix.addVoice(voice);
		addVoice(voice);
	}
}

void APSynth::handleMidiEvent(const juce::MidiMessage &m)
{
	MPESynthesiser::handleMidiEvent(m);

	if (m.isSysEx()) {
		if (MTS_HasMaster(proc.client))
			MTS_ParseMIDIDataU(
			    proc.client, m.getSysExData(), m.getSysExDataSize());
	}

	if (m.isNoteOn()) {
		proc.newRand();
	}

	if (m.isController()) {
		if (m.getControllerNumber() == 1) {
			proc.modMatrix.setMonoValue(
			    proc.modSrcModwheel, float(m.getControllerValue()) / 127.0f);
			return;
		}
		if (proc.macroParams.learning->getUserValueInt() > 0) {
			int n = proc.macroParams.learning->getUserValueInt();
			int cc = m.getControllerNumber();
			if (n == 1) {
				proc.macroParams.macro1cc->setUserValue(cc);
			} else if (n == 2) {
				proc.macroParams.macro2cc->setUserValue(cc);
			} else if (n == 3) {
				proc.macroParams.macro3cc->setUserValue(cc);
			}
			proc.macroParams.learning->setValue(0.f);
		}
		if (m.getControllerNumber() ==
		    proc.macroParams.macro1cc->getUserValueInt()) {
			proc.macroParams.macro1->setValue(m.getControllerValue() / 127.0f);
		}
		if (m.getControllerNumber() ==
		    proc.macroParams.macro2cc->getUserValueInt()) {
			proc.macroParams.macro2->setValue(m.getControllerValue() / 127.0f);
		}
		if (m.getControllerNumber() ==
		    proc.macroParams.macro3cc->getUserValueInt()) {
			proc.macroParams.macro3->setValue(m.getControllerValue() / 127.0f);
		}
	}
	if (m.isPitchWheel()) {
		proc.modMatrix.setMonoValue(proc.modSrcMonoPitchbend,
		    static_cast<float>(m.getPitchWheelValue()) / 0x2000 - 1.0f);
	}
	if (m.isAftertouch()) {
		for (const auto &voice : voices) {
			if (auto *svoice = dynamic_cast<SynthVoice3 *>(voice); svoice->curNote.initialNote == m.getNoteNumber()) {
				proc.modMatrix.setPolyValue(*svoice, proc.modPolyAT, m.getAfterTouchValue() / 127.0f);
			}
		}
	}
}
