#include "APSampler.h"
#include "APSamplerVoice.h"
#include "PluginProcessor.h"
#define MAX_SAMPLE_LENGTH_S 10

APSampler::APSampler(APAudioProcessor& proc_) : proc(proc_)
{
    enableLegacyMode(12);
    setVoiceStealingEnabled(true);

    for (int i = 0; i < 16; i++)
    {
        auto voice = new APSamplerVoice(proc);
        proc.modMatrix.addVoice(voice);
        addVoice(voice);
    }
    
    formatManager.registerBasicFormats();
}

bool APSampler::loadSound(const juce::String& path) {
    reader = formatManager.createReaderFor(juce::File(path));

    sound.sourceSampleRate = reader->sampleRate;
    sound.length = jmin((int) reader->lengthInSamples,
                   (int) (MAX_SAMPLE_LENGTH_S * sound.sourceSampleRate));

    sound.data.reset(new AudioBuffer<float>(jmin (2, (int)reader->numChannels), sound.length + 4));
    sound.midiRootNote = proc.samplerParams.key->getUserValueInt();
    sound.name = path;
    
    if (reader->read(sound.data.get(), 0, sound.length + 4, 0, true, true)) {
        for (auto voice : voices)
        {
            auto vav = dynamic_cast<APSamplerVoice*>(voice);
            vav->setSound(&sound);
        }
        return true;
    }
    else { return false; }
}

void APSampler::clearSound()
{
	sound = {};
	for (auto voice : voices)
	{
		auto vav = dynamic_cast<APSamplerVoice*>(voice);
		vav->setSound(nullptr);
	}
}

void APSampler::updateBaseNote(int note)
{
	sound.midiRootNote = note;
	for (auto voice : voices)
	{
		auto vav = dynamic_cast<APSamplerVoice*>(voice);
		vav->setSound(&sound);
	}
}

void APSampler::handleMidiEvent(const juce::MidiMessage& m) {
    MPESynthesiser::handleMidiEvent(m);
}
