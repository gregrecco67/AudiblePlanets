#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SynthVoice.h"

static juce::String subTextFunction (const gin::Parameter&, float v)
{
    switch (int (v))
    {
        case 0: return "Sine";
        case 1: return "Triangle";
        case 2: return "Saw";
        case 3: return "Pulse";
        default:
            jassertfalse;
            return {};
    }
}

static juce::String lfoTextFunction (const gin::Parameter&, float v)
{
    switch ((gin::LFO::WaveShape)int (v))
    {
        case gin::LFO::WaveShape::none:          return "None";
        case gin::LFO::WaveShape::sine:          return "Sine";
        case gin::LFO::WaveShape::triangle:      return "Triangle";
        case gin::LFO::WaveShape::sawUp:         return "Saw Up";
        case gin::LFO::WaveShape::sawDown:       return "Saw Down";
        case gin::LFO::WaveShape::square:        return "Square";
        case gin::LFO::WaveShape::squarePos:     return "Square+";
        case gin::LFO::WaveShape::sampleAndHold: return "S&H";
        case gin::LFO::WaveShape::noise:         return "Noise";
        case gin::LFO::WaveShape::stepUp3:       return "Step Up 3";
        case gin::LFO::WaveShape::stepUp4:       return "Step Up 4";
        case gin::LFO::WaveShape::stepup8:       return "Step Up 8";
        case gin::LFO::WaveShape::stepDown3:     return "Step Down 3";
        case gin::LFO::WaveShape::stepDown4:     return "Step Down 4";
        case gin::LFO::WaveShape::stepDown8:     return "Step Down 8";
        case gin::LFO::WaveShape::pyramid3:      return "Pyramid 3";
        case gin::LFO::WaveShape::pyramid5:      return "Pyramid 5";
        case gin::LFO::WaveShape::pyramid9:      return "Pyramid 9";
        default:
            jassertfalse;
            return {};
    }
}

static juce::String enableTextFunction (const gin::Parameter&, float v)
{
    return v > 0.0f ? "On" : "Off";
}

static juce::String durationTextFunction (const gin::Parameter&, float v)
{
    return gin::NoteDuration::getNoteDurations()[size_t (v)].getName();
}

static juce::String filterTextFunction (const gin::Parameter&, float v)
{
    switch (int (v))
    {
        case 0: return "LP 12";
        case 1: return "LP 24";
        case 2: return "HP 12";
        case 3: return "HP 24";
        case 4: return "BP 12";
        case 5: return "BP 24";
        case 6: return "NT 12";
        case 7: return "NT 24";
        default:
            jassertfalse;
            return {};
    }
}

static juce::String envSelectTextFunction (const gin::Parameter&, float v)
{
	switch (int (v))
	{
		case 0: return "Env 1";
		case 1: return "Env 2";
		case 2: return "Env 3";
		case 3: return "Env 4";
		default:
			jassertfalse;
			return {};
	}
}

static juce::String syncrepeatTextFunction(const gin::Parameter&, float v)
{
	switch (int (v))
	{
		case 0: return "Off";
		case 1: return "Sync";
		case 2: return "Free";
		default:
			jassertfalse;
			return {};
	}
}

static juce::String algoTextFunction(const gin::Parameter&, float v)
{
    switch (int (v))
    {
        case 0: return String("1-2-3-(4)");
        case 1: return String("1-2-(3), 2-(4)");
        case 2: return String("1-(2), 1-3-(4)");
        case 3: return String("1-(2), 1-(3), 1-(4)");
        default:
            jassertfalse;
            return {};
    }
}

static juce::String freqTextFunction (const gin::Parameter&, float v)
{
    return juce::String (int (gin::getMidiNoteInHertz (v)));
}

static juce::String glideModeTextFunction (const gin::Parameter&, float v)
{
    switch (int (v))
    {
        case 0: return "Off";
        case 1: return "Glissando";
        case 2: return "Portamento";
        default:
            jassertfalse;
            return {};
    }
}


//==============================================================================
void APAudioProcessor::OSCParams::setup(APAudioProcessor& p, juce::String num) // we've got 4
{
    juce::String id = "osc" + num;
    juce::String nm = "OSC" + num;

	NormalisableRange<float> osc1FineRange{ 0.0, 4.0, 0.0, 1.0 };
	NormalisableRange<float> defaultFineRange{ -2.0, 2.0, 0.0, 1.0 };
    coarse    = p.addExtParam (id + "coarse",     nm + "Coarse",      "Coarse",    "", { 1.0, 24.0, 1.0, 1.0 }, 1.0, 0.0f);
	if (num.getIntValue() == 1) {
		fine = p.addExtParam(id + "fine", nm + "Fine", "Fine", "", osc1FineRange, 0.0, 0.0f);
	}
	else {
		fine = p.addExtParam(id + "fine", nm + "Fine", "Fine", "", defaultFineRange, 0.0, 0.0f);
	}
	radius    = p.addExtParam (id + "radius",     nm + "Radius",      "Radius",    "", { 0.0, 1.0, 0.0, 1.0 }, 0.3, 0.0f);
    tones     = p.addExtParam (id + "tones",      nm + "Tones",       "Tones",     "", { 1.0, 8.0, 1.0, 1.0 }, 1.0, 0.0f);
    detune    = p.addExtParam (id + "detune",     nm + "Detune",      "Detune",    "", { 0.0, 0.5, 0.0, 1.0 }, 0.0, 0.0f);
    spread    = p.addExtParam (id + "spread",     nm + "Spread",      "Spread",    "%", { -100.0, 100.0, 0.0, 1.0 }, 0.0, 0.0f);
    pan       = p.addExtParam (id + "pan",        nm + "Pan",         "Pan",       "", { -1.0, 1.0, 0.0, 1.0 }, 0.0, 0.0f);
    env       = p.addExtParam (id + "env",        nm + "Env",         "Env",       "", { 0.0, 3.0, 1.0, 1.0 }, (float)(num.getIntValue() - 1), 0.0f, envSelectTextFunction);
	saw       = p.addExtParam (id + "saw",        nm + "Saw",         "Saw",       "", { 0.0, 1.0, 1.0, 1.0 }, 0.0, 0.0f, enableTextFunction);
	fixed     = p.addExtParam (id + "fixed",      nm + "Fixed",       "Fixed",     "", { 0.0, 1.0, 1.0, 1.0 }, 0.0, 0.0f, enableTextFunction);
}

//==============================================================================
void APAudioProcessor::FilterParams::setup (APAudioProcessor& p) // each osc has a filter, but the params are global (though it's also a poly mod dest)
{
    juce::String id = "flt";
    juce::String nm = "FLT ";

    float maxFreq = float (gin::getMidiNoteFromHertz (20000.0));

    enable           = p.addIntParam (id + "enable",  nm + "Enable",  "",      "", { 0.0, 1.0, 1.0, 1.0 }, 1.0f, 0.0f);
    type             = p.addIntParam (id + "type",    nm + "Type",    "Type",  "", { 0.0, 7.0, 1.0, 1.0 }, 0.0, 0.0f, filterTextFunction);
    keyTracking      = p.addExtParam (id + "key",     nm + "Key",     "Key",   "%", { 0.0, 100.0, 0.0, 1.0 }, 0.0, 0.0f);
    frequency        = p.addExtParam (id + "freq",    nm + "Freq",    "Freq",  "Hz", { 0.0, maxFreq, 0.0, 1.0 }, 64.0, 0.0f, freqTextFunction);
    resonance        = p.addExtParam (id + "res",     nm + "Res",     "Res",   "", { 0.0, 100.0, 0.0, 1.0 }, 0.0, 0.0f);

    keyTracking->conversionFunction      = [] (float in) { return in / 100.0f; };
}

//==============================================================================
void APAudioProcessor::LFOParams::setup (APAudioProcessor& p, String num) // most complicated
{                                                                                                            
    // we've got 4 in the processor as mono sources, and four for each voice as poly sources / dests
    juce::String id = "lfo" + num;
    juce::String nm = "LFO" + num;

    auto notes = gin::NoteDuration::getNoteDurations();

	enable = p.addIntParam(id + "enable", id + "Enable", "Enable", "", { 0.0, 1.0, 1.0, 1.0 }, 0.0f, 0.0f, enableTextFunction);

    sync             = p.addIntParam (id + "sync",    id + "Sync",    "Sync",   "", { 0.0, 1.0, 1.0, 1.0 }, 1.0, 0.0f, enableTextFunction);
    wave             = p.addIntParam (id + "wave",    id + "Wave",    "Wave",   "", { 1.0, 17.0, 1.0, 1.0 }, 1.0, 0.0f, lfoTextFunction);
    rate             = p.addExtParam (id + "rate",    id + "Rate",    "Rate",   "Hz", { 0.0, 50.0, 0.0, 0.3f }, 10.0, 0.0f);
    beat             = p.addIntParam (id + "beat",    id + "Beat",    "Beat",   "", { 0.0, float (notes.size() - 1), 1.0, 1.0 }, 13.0, 0.0f, durationTextFunction);
    depth            = p.addExtParam (id + "depth",   id + "Depth",   "Depth",  "", { -1.0, 1.0, 0.0, 1.0 }, 1.0, 0.0f);
    phase            = p.addExtParam (id + "phase",   id + "Phase",   "Phase",  "", { -1.0, 1.0, 0.0, 1.0 }, 0.0, 0.0f);
    offset           = p.addExtParam (id + "offset",  id + "Offset",  "Offset", "", { -1.0, 1.0, 0.0, 1.0 }, 0.0, 0.0f);
    fade             = p.addExtParam (id + "fade",    id + "Fade",    "Fade",   "s", { -60.0, 60.0, 0.0, 0.2f, true }, 0.0f, 0.0f);
    delay            = p.addExtParam (id + "delay",   id + "Delay",   "Delay",  "s", { 0.0, 60.0, 0.0, 0.2f }, 0.0f, 0.0f);

    this->num = num.getIntValue();
}

//==============================================================================
void APAudioProcessor::ENVParams::setup(APAudioProcessor& p, String num) // 
{
    String id = "env" + num;
	auto notes = gin::NoteDuration::getNoteDurations();

    velocityTracking = p.addExtParam(id + "vel", id + "Vel", "Vel", "", { 0.0, 100.0, 0.0, 1.0 }, 100.0, 0.0f);
    attack = p.addExtParam(id + "attack", id + "Attack", "A", "s", { 0.0, 60.0, 0.0, 0.2f }, 0.1f, 0.0f);
    decay = p.addExtParam(id + "decay", id + "Decay", "D", "s", { 0.0, 60.0, 0.0, 0.2f }, 0.1f, 0.0f);
    sustain = p.addExtParam(id + "sustain", id + "Sustain", "S", "%", { 0.0, 100.0, 0.0, 1.0 }, 80.0f, 0.0f);
    release = p.addExtParam(id + "release", id + "Release", "R", "s", { 0.0, 60.0, 0.0, 0.2f }, 0.1f, 0.0f);
	acurve = p.addExtParam(id + "acurve", id + "ACurve", "A Curve", "", { -1.0, 1.0, 0.0, 1.0 }, 1.0f, 0.0f);
	drcurve = p.addExtParam(id + "drcurve", id + "DRCurve", "DR Curve", "", { -1.0, 1.0, 0.0, 1.0 }, -1.0f, 0.0f);
	syncrepeat = p.addIntParam(id + "syncrepeat", id + "SyncRepeat", "Repeat", "", { 0.0, 2.0, 1.0, 1.0 }, 0.0f, 0.0f, syncrepeatTextFunction);
	time = p.addExtParam(id + "time", id + "Time", "Time", "s", { 0.0, 60.0, 0.0, 0.2f }, 0.1f, 0.0f);
	duration = p.addIntParam(id + "beat", id + "Beat", "Beat", "", { 0.0, float(notes.size() - 1), 1.0, 1.0 }, 13.0, 0.0f, durationTextFunction);

    sustain->conversionFunction = [](float in) { return in / 100.0f; };
    velocityTracking->conversionFunction = [](float in) { return in / 100.0f; };
}


//==============================================================================
void APAudioProcessor::GlobalParams::setup (APAudioProcessor& p)
{
    mono        = p.addIntParam ("mono",    "Mono",       "",      "",   { 0.0, 1.0, 0.0, 1.0 }, 0.0, 0.0f, enableTextFunction);
    glideMode   = p.addIntParam ("gMode",   "Glide Mode", "Glide", "",   { 0.0, 2.0, 0.0, 1.0 }, 0.0f, 0.0f, glideModeTextFunction);
    glideRate   = p.addExtParam ("gRate",   "Glide Rate", "Rate",  "s",   { 0.001f, 20.0, 0.0, 0.2f }, 0.3f, 0.0f);
    legato      = p.addIntParam ("legato",  "Legato",     "",      "",   { 0.0, 1.0, 0.0, 1.0 }, 0.0, 0.0f, enableTextFunction);
    level       = p.addExtParam ("level",   "Level",      "",      "db", { -100.0, 0.0, 1.0, 4.0f }, 0.0, 0.0f);
    voices      = p.addIntParam ("voices",  "Voices",     "",      "",   { 2.0, 40.0, 1.0, 1.0 }, 40.0f, 0.0f);
    mpe         = p.addIntParam ("mpe",     "MPE",        "",      "",   { 0.0, 1.0, 1.0, 1.0 }, 0.0f, 0.0f, enableTextFunction);

    level->conversionFunction     = [] (float in) { return juce::Decibels::decibelsToGain (in); };
}

void APAudioProcessor::TimbreParams::setup(APAudioProcessor& p)
{
	equant = p.addExtParam("equant", "Equant", "", "", { 0.0, 1.0, 0.0, 1.0 }, 0.0, 0.0f);
	pitch = p.addExtParam("pitch", "Pitch", "", "", { 0.01, 4.0, 0.0, 1.0 }, 1.0, 0.0f);
	blend = p.addExtParam("blend", "Blend", "", "", { 0.0, 1.0, 0.0, 1.0 }, 0.0, 0.0f);
	demodmix = p.addExtParam("demodmix", "Demodulate", "", "", { 0.0, 1.0, 0.0, 1.0 }, 0.0, 0.0f);
    algo = p.addIntParam("algo", "Algorithm", "", "", {0.0, 3.0, 1.0, 1.0}, 0.0, 0.f, algoTextFunction);
}


//==============================================================================
APAudioProcessor::APAudioProcessor() : gin::Processor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true), false)
{
    enableLegacyMode();
    setVoiceStealingEnabled (true);

    for (int i = 0; i < 50; i++)
    {
        auto voice = new SynthVoice (*this);
        modMatrix.addVoice (voice);
        addVoice (voice);
    }


	osc1Params.setup(*this, String{ "1" });
	osc2Params.setup(*this, String{ "2" });
	osc3Params.setup(*this, String{ "3" });
	osc4Params.setup(*this, String{ "4" });
	
	env1Params.setup(*this, String{ "1" });
	env2Params.setup(*this, String{ "2" });
	env3Params.setup(*this, String{ "3" });
	env4Params.setup(*this, String{ "4" });
	
	lfo1Params.setup(*this, String{ "1" });
	lfo2Params.setup(*this, String{ "2" });
	lfo3Params.setup(*this, String{ "3" });
	lfo4Params.setup(*this, String{ "4" });

	globalParams.setup (*this);
	filterParams.setup(*this);
	timbreParams.setup(*this);


    setupModMatrix();
    init();
}

APAudioProcessor::~APAudioProcessor()
{
}

//==============================================================================
void APAudioProcessor::setupModMatrix()
{
    modSrcPressure  = modMatrix.addPolyModSource ("mpep", "MPE Pressure", false);
    modSrcTimbre    = modMatrix.addPolyModSource ("mpet", "MPE Timbre", false);

    modScrPitchBend = modMatrix.addMonoModSource ("pb", "Pitch Bend", true);

    modSrcNote      = modMatrix.addPolyModSource ("note", "MIDI Note Number", false);
    modSrcVelocity  = modMatrix.addPolyModSource ("vel", "MIDI Velocity", false);

	modSrcMonoLFO1 = modMatrix.addMonoModSource("mlfo1", "mono LFO1", true);
	modSrcMonoLFO2 = modMatrix.addMonoModSource("mlfo2", "mono LFO2", true);
	modSrcMonoLFO3 = modMatrix.addMonoModSource("mlfo3", "mono LFO3", true);
	modSrcMonoLFO4 = modMatrix.addMonoModSource("mlfo4", "mono LFO4", true);

	modSrcLFO1 = modMatrix.addPolyModSource("lfo1", "poly LFO1", true);
	modSrcLFO2 = modMatrix.addPolyModSource("lfo2", "poly LFO2", true);
	modSrcLFO3 = modMatrix.addPolyModSource("lfo3", "poly LFO3", true);
	modSrcLFO4 = modMatrix.addPolyModSource("lfo4", "poly LFO4", true);

	modSrcEnv1 = modMatrix.addPolyModSource("env1", "Env1", true);
	modSrcEnv2 = modMatrix.addPolyModSource("env2", "Env2", true);
	modSrcEnv3 = modMatrix.addPolyModSource("env3", "Env3", true);
	modSrcEnv4 = modMatrix.addPolyModSource("env4", "Env4", true);
    
    auto firstMonoParam = globalParams.mono;
    bool polyParam = true;
    for (auto pp : getPluginParameters())
    {
        if (pp == firstMonoParam)
            polyParam = false;

        if (! pp->isInternal())
            modMatrix.addParameter (pp, polyParam);
    }

    modMatrix.build();
}

void APAudioProcessor::reset()
{
    Processor::reset();

    lfo1.reset();
	lfo2.reset();
	lfo3.reset();
	lfo4.reset();
}

void APAudioProcessor::prepareToPlay (double newSampleRate, int newSamplesPerBlock)
{
    Processor::prepareToPlay (newSampleRate, newSamplesPerBlock);

    setCurrentPlaybackSampleRate (newSampleRate);

    modMatrix.setSampleRate (newSampleRate);

    stereoDelay.setSampleRate (newSampleRate);
	lfo1.setSampleRate(newSampleRate);
	lfo2.setSampleRate(newSampleRate);
	lfo3.setSampleRate(newSampleRate);
	lfo4.setSampleRate(newSampleRate);

    analogTables.setSampleRate (newSampleRate);
}

void APAudioProcessor::releaseResources()
{
}

void APAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    if (presetLoaded)
    {
        presetLoaded = false;
        turnOffAllVoices (false);
    }

    startBlock();
    setMPE (globalParams.mpe->isOn());

    playhead = getPlayHead();

    int pos = 0;
    int todo = buffer.getNumSamples();

    buffer.clear();

    setMono (globalParams.mono->isOn());
    setLegato (globalParams.legato->isOn());
    setGlissando (globalParams.glideMode->getProcValue() == 1.0f);
    setPortamento (globalParams.glideMode->getProcValue() == 2.0f);
    setGlideRate (globalParams.glideRate->getProcValue());
    setNumVoices (int (globalParams.voices->getProcValue()));

    while (todo > 0)
    {
        int thisBlock = std::min (todo, 32);

        updateParams (thisBlock);

        renderNextBlock (buffer, midi, pos, thisBlock);

        auto bufferSlice = gin::sliceBuffer (buffer, pos, thisBlock);
        applyEffects (bufferSlice);

        modMatrix.finishBlock (thisBlock);

        pos += thisBlock;
        todo -= thisBlock;
    }

    playhead = nullptr;

    endBlock (buffer.getNumSamples());
}

juce::Array<float> APAudioProcessor::getLiveFilterCutoff()
{
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

void APAudioProcessor::applyEffects (juce::AudioSampleBuffer& buffer)
{
    // Apply Delay
    //if (delayParams.enable->isOn())
    //    stereoDelay.process (buffer);

    // Output gain
    outputGain.process (buffer);
}



void APAudioProcessor::updateParams (int newBlockSize)
{
    // Update Mono LFOs
    for(auto lfoparams : {&lfo1Params, &lfo2Params, &lfo3Params, &lfo4Params})
	{
		gin::LFO::Parameters classparams;
		auto& lfo = this->monoLFOs[lfoparams->num-1];
		float freq = 0;
		if (lfoparams->sync->getProcValue() > 0.0f)
			freq = 1.0f / gin::NoteDuration::getNoteDurations()[size_t(lfoparams->beat->getProcValue())].toSeconds(playhead);
		else
			freq = modMatrix.getValue(lfoparams->rate);

		classparams.waveShape = (gin::LFO::WaveShape) int(lfoparams->wave->getProcValue());
		classparams.frequency = freq;
		classparams.phase = modMatrix.getValue(lfoparams->phase);
		classparams.offset = modMatrix.getValue(lfoparams->offset);
		classparams.depth = modMatrix.getValue(lfoparams->depth);
		classparams.delay = 0;
		classparams.fade = 0;

		lfo->setParameters(classparams);
		lfo->process(newBlockSize);

		modMatrix.setMonoValue(*(this->lfoIds[lfoparams->num - 1]), lfo->getOutput());
    }

    // Output gain
    //outputGain.setGain (modMatrix.getValue (globalParams.level));
}

void APAudioProcessor::handleMidiEvent (const juce::MidiMessage& m)
{
    MPESynthesiser::handleMidiEvent (m);

    if (m.isPitchWheel())
        modMatrix.setMonoValue (modScrPitchBend, float (m.getPitchWheelValue()) / 0x2000 - 1.0f);
}

//==============================================================================
bool APAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* APAudioProcessor::createEditor()
{
    return new gin::ScaledPluginEditor (new APAudioProcessorEditor (*this), state);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new APAudioProcessor();
}

