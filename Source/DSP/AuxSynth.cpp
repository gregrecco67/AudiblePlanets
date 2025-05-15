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

juce::Array<float> AuxSynth::getLiveFilterCutoff()
{
	juce::Array<float> values;

	for (auto v : voices) {
		if (v->isActive()) {
			auto vav = dynamic_cast<AuxSynthVoice *>(v);
			values.add(vav->getFilterCutoffNormalized());
		}
	}
	return values;
}

std::vector<float> AuxSynth::getMSEG1Phases()
{
	std::vector<float> values;

	for (auto v : voices) {
		if (v->isActive()) {
			auto vav = dynamic_cast<AuxSynthVoice *>(v);
			values.push_back(vav->getMSEG1Phase());
		}
	}
	return values;
}

std::vector<float> AuxSynth::getMSEG2Phases()
{
	std::vector<float> values;

	for (auto v : voices) {
		if (v->isActive()) {
			auto vav = dynamic_cast<AuxSynthVoice *>(v);
			values.push_back(vav->getMSEG2Phase());
		}
	}
	return values;
}

std::vector<float> AuxSynth::getMSEG3Phases()
{
	std::vector<float> values;

	for (auto v : voices) {
		if (v->isActive()) {
			auto vav = dynamic_cast<AuxSynthVoice *>(v);
			values.push_back(vav->getMSEG3Phase());
		}
	}
	return values;
}

std::vector<float> AuxSynth::getMSEG4Phases()
{
	std::vector<float> values;

	for (auto v : voices) {
		if (v->isActive()) {
			auto vav = dynamic_cast<AuxSynthVoice *>(v);
			values.push_back(vav->getMSEG4Phase());
		}
	}
	return values;
}

void AuxSynth::handleMidiEvent(const juce::MidiMessage &m)
{
	MPESynthesiser::handleMidiEvent(m);

	if (m.isAftertouch()) {
		for (auto &voice : voices) {
			AuxSynthVoice *svoice = static_cast<AuxSynthVoice *>(voice);
			if (svoice->curNote.initialNote == m.getNoteNumber()) {
				proc.modMatrix.setPolyValue(*svoice, proc.modPolyAT,
				                            m.getAfterTouchValue() / 127.0f);
			}
		}
	}
}
