#include "AuxSynth.h"
#include "PluginProcessor.h"

AuxSynth::AuxSynth(APAudioProcessor &proc_) : proc(proc_)
{
	enableLegacyMode(12);
	setVoiceStealingEnabled(true);

	for (int i = 0; i < 16; i++) {
		auto voice = new AuxSynthVoice(proc);
		proc.modMatrix.addVoice(voice);
		addVoice(voice);
	}
}

void AuxSynth::handleMidiEvent(const juce::MidiMessage &m)
{
	MPESynthesiser::handleMidiEvent(m);

	if (m.isAftertouch()) {
		for (const auto &voice : voices) {
			if (auto *svoice = dynamic_cast<AuxSynthVoice *>(voice); svoice->curNote.initialNote == m.getNoteNumber()) {
				proc.modMatrix.setPolyValue(*svoice, proc.modPolyAT, m.getAfterTouchValue() / 127.0f);
			}
		}
	}
}
