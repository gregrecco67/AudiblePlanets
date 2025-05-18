#pragma once
#include <gin_dsp/gin_dsp.h>
#include <gin_plugin/gin_plugin.h>
#include "AuxSynthVoice.h"

class APAudioProcessor;

class AuxSynth : public gin::Synthesiser {
public:
	AuxSynth(APAudioProcessor &proc_);
	~AuxSynth() override = default;

	void handleMidiEvent(const juce::MidiMessage &m) override;

	inline juce::Array<float> getLiveFilterCutoff() const
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


	inline std::vector<float> getMSEG1Phases() const
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

	inline std::vector<float> getMSEG2Phases() const
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

	inline std::vector<float> getMSEG3Phases() const
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

	inline std::vector<float> getMSEG4Phases() const
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

private:
	APAudioProcessor &proc;
};
