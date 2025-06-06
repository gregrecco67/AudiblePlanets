#pragma once

#include <gin_dsp/gin_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "SynthVoice3.h"

class APAudioProcessor;

class APSynth : public gin::Synthesiser {
public:
	explicit APSynth(APAudioProcessor &proc_);
	~APSynth() override = default;

	void handleMidiEvent(const juce::MidiMessage &m) override;
	
	inline juce::Array<float> getLiveFilterCutoff() const
	{
		juce::Array<float> values;
		for (const auto v : voices) {
			if (v->isActive()) {
				const auto vav = dynamic_cast<SynthVoice3 *>(v);
				values.add(vav->getFilterCutoffNormalized());
			}
		}
		return values;
	}

	void shutItDown()
	{
		for (auto v : voices) {
			if (v->isActive()) {
				auto vav = dynamic_cast<SynthVoice3 *>(v);
				vav->setFastKill();
			}
		}
	}

	inline std::vector<float> getMSEG1Phases() const
	{
		std::vector<float> values;
		for (const auto v : voices) {
			if (v->isActive()) {
				const auto vav = dynamic_cast<SynthVoice3 *>(v);
				values.push_back(vav->getMSEG1Phase());
			}
		}
		return values;
	}

	inline std::vector<float> getMSEG2Phases() const
	{
		std::vector<float> values;
		for (const auto v : voices) {
			if (v->isActive()) {
				const auto vav = dynamic_cast<SynthVoice3 *>(v);
				values.push_back(vav->getMSEG2Phase());
			}
		}
		return values;
	}

	inline std::vector<float> getMSEG3Phases() const
	{
		std::vector<float> values;
		for (const auto v : voices) {
			if (v->isActive()) {
				const auto vav = dynamic_cast<SynthVoice3 *>(v);
				values.push_back(vav->getMSEG3Phase());
			}
		}
		return values;
	}

	inline std::vector<float> getMSEG4Phases() const
	{
		std::vector<float> values;
		for (const auto v : voices) {
			if (v->isActive()) {
				const auto vav = dynamic_cast<SynthVoice3 *>(v);
				values.push_back(vav->getMSEG4Phase());
			}
		}
		return values;
	}
	
	inline std::vector<float> getLFO1Phases() const
	{
		std::vector<float> values;
		for (const auto v : voices) {
			if (v->isActive()) {
				const auto vav = dynamic_cast<SynthVoice3 *>(v);
				values.push_back(vav->getLFO1Phase());
			}
		}
		return values;
	}

	inline std::vector<float> getLFO2Phases() const
	{
		std::vector<float> values;
		for (const auto v : voices) {
			if (v->isActive()) {
				const auto vav = dynamic_cast<SynthVoice3 *>(v);
				values.push_back(vav->getLFO2Phase());
			}
		}
		return values;
	}

	inline std::vector<float> getLFO3Phases() const
	{
		std::vector<float> values;
		for (const auto v : voices) {
			if (v->isActive()) {
				const auto vav = dynamic_cast<SynthVoice3 *>(v);
				values.push_back(vav->getLFO3Phase());
			}
		}
		return values;
	}

	inline std::vector<float> getLFO4Phases() const
	{
		std::vector<float> values;
		for (const auto v : voices) {
			if (v->isActive()) {
				const auto vav = dynamic_cast<SynthVoice3 *>(v);
				values.push_back(vav->getLFO4Phase());
			}
		}
		return values;
	}

	inline std::vector<Envelope::EnvelopeState> getENV1States() const
	{
		std::vector<Envelope::EnvelopeState> states;
		for (const auto v : voices) {
			if (v->isActive()) {
				const auto vav = dynamic_cast<SynthVoice3 *>(v);
				states.push_back(vav->getENV1State());
			}
		}
		return states;
	}

	inline std::vector<Envelope::EnvelopeState> getENV2States() const
	{
		std::vector<Envelope::EnvelopeState> states;

		for (const auto v : voices) {
			if (v->isActive()) {
				const auto vav = dynamic_cast<SynthVoice3 *>(v);
				states.push_back(vav->getENV2State());
			}
		}
		return states;
	}

	inline std::vector<Envelope::EnvelopeState> getENV3States() const
	{
		std::vector<Envelope::EnvelopeState> states;
		for (const auto v : voices) {
			if (v->isActive()) {
				const auto vav = dynamic_cast<SynthVoice3 *>(v);
				states.push_back(vav->getENV3State());
			}
		}
		return states;
	}

	inline std::vector<Envelope::EnvelopeState> getENV4States() const
	{
		std::vector<Envelope::EnvelopeState> states;
		for (const auto v : voices) {
			if (v->isActive()) {
				const auto vav = dynamic_cast<SynthVoice3 *>(v);
				states.push_back(vav->getENV4State());
			}
		}
		return states;
	}

private:
	APAudioProcessor &proc;
};
