#include "Synth.h"
#include "PluginProcessor.h"

APSynth::APSynth(APAudioProcessor& proc_) : proc(proc_)
{
    enableLegacyMode(12);
    setVoiceStealingEnabled(true);

    for (int i = 0; i < 10; i++)
    {
        auto voice = new SynthVoice(proc);
        proc.modMatrix.addVoice(voice);
        addVoice(voice);
    }
}

juce::Array<float> APSynth::getLiveFilterCutoff() {
    juce::Array<float> values;
    
    for (auto v : voices)
    {
        if (v->isActive())
        {
            auto vav = dynamic_cast<SynthVoice*>(v);
            values.add (vav->getFilterCutoffNormalized());
        }
    }
    return values;
}

std::vector<float> APSynth::getMSEG1Phases() {
	std::vector<float> values;

	for (auto v : voices)
	{
		if (v->isActive())
		{
			auto vav = dynamic_cast<SynthVoice*>(v);
			values.push_back(vav->getMSEG1Phase());
		}
	}
	return values;
}

std::vector<float> APSynth::getMSEG2Phases() {
	std::vector<float> values;

	for (auto v : voices)
	{
		if (v->isActive())
		{
			auto vav = dynamic_cast<SynthVoice*>(v);
			values.push_back(vav->getMSEG2Phase());
		}
	}
	return values;
}

std::vector<float> APSynth::getMSEG3Phases() {
	std::vector<float> values;

	for (auto v : voices)
	{
		if (v->isActive())
		{
			auto vav = dynamic_cast<SynthVoice*>(v);
			values.push_back(vav->getMSEG3Phase());
		}
	}
	return values;
}

std::vector<float> APSynth::getMSEG4Phases() {
	std::vector<float> values;

	for (auto v : voices)
	{
		if (v->isActive())
		{
			auto vav = dynamic_cast<SynthVoice*>(v);
			values.push_back(vav->getMSEG4Phase());
		}
	}
	return values;
}


void APSynth::handleMidiEvent(const juce::MidiMessage& m) {
    MPESynthesiser::handleMidiEvent(m);

	if (m.isSysEx()) {
		MTS_ParseMIDIDataU(proc.client, m.getSysExData(), m.getSysExDataSize());
	}

    if (m.isController()) {
		if (m.getControllerNumber() == 1) {
			proc.modMatrix.setMonoValue(proc.modSrcModwheel, float(m.getControllerValue()) / 127.0f);
		}
    }
	if (m.isPitchWheel()) {
		proc.modMatrix.setMonoValue(proc.modSrcMonoPitchbend, float(m.getPitchWheelValue()) / 0x2000 - 1.0f);
	}
    if (m.isAftertouch()) {
        for (auto& voice : voices) {
            SynthVoice* svoice = static_cast<SynthVoice*>(voice);
            if (svoice->curNote.initialNote == m.getNoteNumber()) {
                proc.modMatrix.setPolyValue(*svoice, proc.modPolyAT, m.getAfterTouchValue() / 127.0f);
            }
        }
        
    }
}
