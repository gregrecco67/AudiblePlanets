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

static juce::String waveshaperTypeTextFunction(const gin::Parameter&, float v)
{
	switch (int (v))
	{
		case 0: return "atan 2";
		case 1: return "atan 4";
		case 2: return "atan 6";
		case 3: return "tanh 2";
		case 4: return "tanh 4";
		case 5: return "tanh 6";
		case 6: return "cubic 3/2";
		case 7: return "cubic mid";
		case 8: return "cubic";
		case 9: return "cheb 3";
		case 10: return "cheb 5";
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

static juce::String fxListTextFunction(const gin::Parameter&, float v)
{
    switch (int (v))
    {
        case 0: return String("None");
        case 1: return String("");
        case 2: return String("");
        case 3: return String("");
        case 4: return String("");
        case 5: return String("");
        case 6: return String("");
        case 7: return String("");
        case 8: return String("");
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
	volume    = p.addExtParam (id + "volume",     nm + "Volume",      "Volume",    "", { 0.0, 1.0, 0.01, 1.0 }, 0.3, 0.0f);
    tones     = p.addExtParam (id + "tones",      nm + "Tones",       "Tones",     "", { 1.0, 5.9, 0.001, 1.0 }, 1.0, 0.0f);
    detune    = p.addExtParam (id + "detune",     nm + "Detune",      "Detune",    "", { 0.0, 0.5, 0.0, 1.0 }, 0.0, 0.0f);
    spread    = p.addExtParam (id + "spread",     nm + "Spread",      "Spread",    "%", { -100.0, 100.0, 0.0, 1.0 }, 0.0, 0.0f);
    pan       = p.addExtParam (id + "pan",        nm + "Pan",         "Pan",       "", { -1.0, 1.0, 0.0, 1.0 }, 0.0, 0.0f);
    env       = p.addExtParam (id + "env",        nm + "Env",         "Env",       "", { 0.0, 3.0, 1.0, 1.0 }, (float)(num.getIntValue() - 1), 0.0f, envSelectTextFunction);
	saw       = p.addExtParam (id + "saw",        nm + "Saw",         "Saw",       "", { 0.0, 1.0, 1.0, 1.0 }, 0.0, 0.0f, enableTextFunction);
	fixed     = p.addExtParam (id + "fixed",      nm + "Fixed",       "Fixed",     "", { 0.0, 1.0, 1.0, 1.0 }, 0.0, 0.0f, enableTextFunction);
	this->num = num.getIntValue();
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
	syncrepeat = p.addExtParam(id + "syncrepeat", id + "SyncRepeat", "Repeat", "", { 0.0, 2.0, 1.0, 1.0 }, 0.0f, 0.0f, syncrepeatTextFunction);
	time = p.addExtParam(id + "time", id + "Time", "Time", "s", { 0.0, 60.0, 0.0, 0.2f }, 0.1f, 0.0f);
	duration = p.addExtParam(id + "beat", id + "Beat", "Beat", "", { 0.0, float(notes.size() - 1), 1.0, 1.0 }, 13.0, 0.0f, durationTextFunction);

    sustain->conversionFunction = [](float in) { return in / 100.0f; };
    velocityTracking->conversionFunction = [](float in) { return in / 100.0f; };
	this->num = num.getIntValue();
}


//==============================================================================
void APAudioProcessor::GlobalParams::setup (APAudioProcessor& p)
{
    mono        = p.addIntParam ("mono",    "Mono",       "",      "",   { 0.0, 1.0, 0.0, 1.0 }, 0.0, 0.0f, enableTextFunction);
    glideMode   = p.addIntParam ("gMode",   "Glide Mode", "Glide", "",   { 0.0, 2.0, 0.0, 1.0 }, 0.0f, 0.0f, glideModeTextFunction);
    glideRate   = p.addExtParam ("gRate",   "Glide Rate", "Rate",  "s",   { 0.001f, 20.0, 0.0, 0.2f }, 0.3f, 0.0f);
    legato      = p.addIntParam ("legato",  "Legato",     "",      "",   { 0.0, 1.0, 0.0, 1.0 }, 0.0, 0.0f, enableTextFunction);
    level       = p.addExtParam ("level",   "Level",      "",      "db", { -100.0, 12.0, 1.0, 4.0f }, 0.0, 0.0f);
    voices      = p.addIntParam ("voices",  "Voices",     "",      "",   { 2.0, 40.0, 1.0, 1.0 }, 40.0f, 0.0f);
    mpe         = p.addIntParam ("mpe",     "MPE",        "",      "",   { 0.0, 1.0, 1.0, 1.0 }, 0.0f, 0.0f, enableTextFunction);

    level->conversionFunction     = [] (float in) { return juce::Decibels::decibelsToGain (in); };
}

//==============================================================================
void APAudioProcessor::TimbreParams::setup(APAudioProcessor& p)
{
	equant = p.addExtParam("equant", "Equant", "", "", { -0.5, 0.5, 0.0, 1.0 }, 0.0, 0.0f);
	pitch = p.addExtParam("pitch", "Pitch", "", "", { 0.01, 4.0, 0.0, 1.0 }, 1.0, 0.0f);
	blend = p.addExtParam("blend", "Blend", "", "", { 0.0, 1.0, 0.0, 1.0 }, 0.0, 0.0f);
	demodmix = p.addExtParam("demodmix", "Demodulate", "", "", { 0.0, 1.0, 0.0, 1.0 }, 0.0, 0.0f);
    algo = p.addExtParam("algo", "Algorithm", "", "", {0.0, 3.0, 1.0, 1.0}, 0.0, 0.f, algoTextFunction);
}

//==============================================================================
void APAudioProcessor::OrbitParams::setup(APAudioProcessor& p)
{
	speed = p.addIntParam("speed", "Speed", "", "", { 0.0, 0.2, 0.0, 1.0 }, 0.03f, 0.0f);
	scale = p.addIntParam("scale", "Scale", "", "", { 1.0, 6.0, 0.0, 1.0 }, 1.0f, 0.0f);
}


//==============================================================================
void APAudioProcessor::WaveshaperParams::setup(APAudioProcessor& p)
{
	enable = p.addIntParam("enable", "Enable", "", "", { 0.0, 1.0, 1.0, 1.0 }, 0.0f, 0.0f, enableTextFunction);
	drive = p.addExtParam("drive", "Drive", "", "", { 1.0, 40.0, 0.0, 1.0 }, 1.0, 0.0f);
	gain = p.addExtParam("gain", "Gain", "", "", { 0.03, 3.0, 0.0, 1.0 }, 1.0, 0.0f);
	dry = p.addExtParam("dry", "Dry", "", "", { 0.0, 1.0, 0.0, 1.0 }, 1.0, 0.0f);
	wet = p.addExtParam("wet", "Wet", "", "", { 0.0, 1.0, 0.0, 1.0 }, 0.25, 0.0f);
	type = p.addExtParam("func", "Function", "", "", { 0.0, 10.0, 1.0, 1.0 }, 0.0f, 0.0f, waveshaperTypeTextFunction);
}

//==============================================================================
void APAudioProcessor::CompressorParams::setup(APAudioProcessor& p)
{
	enable = p.addIntParam("enable", "Enable", "", "", { 0.0, 1.0, 1.0, 1.0 }, 0.0f, 0.0f, enableTextFunction);
	threshold = p.addExtParam("threshold", "Threshold", "", "", { -60.0, 0.0, 0.0, 1.0 }, -12.0f, 0.0f);
	ratio = p.addExtParam("ratio", "Ratio", "", "", { 1.0, 20.0, 0.0, 1.0 }, 2.0f, 0.0f);
	attack = p.addExtParam("attack", "Attack", "", "", { 0.00002, 0.1, 0.001, 1.0 }, 0.0005f, 0.0f);
	release = p.addExtParam("release", "Release", "", "", { 0.05, 1.0, 0.001, 1.0 }, 0.1f, 0.0f);
	knee = p.addExtParam("knee", "Knee", "", "", { 0.0, 20.0, 0.01, 1.0 }, 0.0f, 0.0f);
	input = p.addExtParam("input", "Input", "", "", { 0.0, 5.0, 0.0, 1.0 }, 1.0f, 0.0f);
	output = p.addExtParam("output", "Output", "", "", { 0.0, 5.0, 0.0, 1.0 }, 1.0f, 0.0f);
}

//==============================================================================
void APAudioProcessor::StereoDelayParams::setup(APAudioProcessor& p)
{
	auto notes = gin::NoteDuration::getNoteDurations();
	enable = p.addIntParam("enable", "Enable", "", "", { 0.0, 1.0, 1.0, 1.0 }, 0.0f, 0.0f, enableTextFunction);
	timeleft = p.addExtParam("timeleft", "Time Left", "", "", { 0.001, 10.0, 0.0, 1.0 }, 0.5f, 0.0f);
	timeright = p.addExtParam("timeright", "Time Right", "", "", { 0.001, 10.0, 0.0, 1.0 }, 0.5f, 0.0f);
	beatsleft = p.addExtParam("beatsleft", "Beats Left", "BeatsL", "", { 0.0, float(notes.size() - 1), 1.0, 1.0 }, 13.0, 0.0f, durationTextFunction);
	beatsright = p.addExtParam("beatsright", "Beats Right", "BeatsR", "", { 0.0, float(notes.size() - 1), 1.0, 1.0 }, 13.0, 0.0f, durationTextFunction);
	temposync = p.addIntParam("temposync", "Tempo Sync", "", "", { 0.0, 1.0, 1.0, 1.0 }, 1.0f, 0.0f, enableTextFunction);
	freeze = p.addIntParam("freeze", "Freeze", "", "", { 0.0, 1.0, 1.0, 1.0 }, 0.0f, 0.0f, enableTextFunction);
	pingpong = p.addIntParam("pingpong", "Ping Pong", "", "", { 0.0, 1.0, 1.0, 1.0 }, 0.0f, 0.0f, enableTextFunction);
	feedback = p.addExtParam("feedback", "Feedback", "", "", { 0.0, 1.0, 0.0, 1.0 }, 0.5f, 0.0f);
	wet = p.addExtParam("wet", "Wet", "", "", { 0.0, 1.0, 0.0, 1.0 }, 0.25, 0.0f);
}

//==============================================================================
void APAudioProcessor::ChorusParams::setup(APAudioProcessor& p)
{
	enable = p.addIntParam("enable", "Enable", "", "", { 0.0, 1.0, 1.0, 1.0 }, 0.0f, 0.0f, enableTextFunction);
	rate = p.addExtParam("rate", "Rate", "", "Hz", { 0.005f, 20.0f, 0.05, 1.0 }, 0.05f, 0.0f);
	depth = p.addExtParam("depth", "Depth", "", "", { 0.0, 1.0, 0.0, 1.0 }, 0.5f, 0.0f);
	delay = p.addExtParam("delay", "Delay", "", "ms", { 10.0f, 40.0f, 0.0, 1.0 }, 20.0f, 0.0f);
	feedback = p.addExtParam("feedback", "Feedback", "", "", { 0.0, 1.0, 0.0, 1.0 }, 0.25f, 0.0f);
	dry = p.addExtParam("dry", "Dry", "", "", { 0.0, 1.0, 0.0, 1.0 }, 1.0f, 0.0f);
	wet = p.addExtParam("wet", "Wet", "", "", { 0.0, 1.0, 0.0, 1.0 }, 0.25f, 0.0f);

}

//==============================================================================
void APAudioProcessor::ReverbParams::setup(APAudioProcessor& p)
{
	enable = p.addIntParam("enable", "Enable", "", "", { 0.0, 1.0, 1.0, 1.0 }, 0.0f, 0.0f, enableTextFunction);
	size = p.addExtParam("size", "Size", "", "", { 0.0, 3.0, 0.0, 1.0 }, 1.f, 0.0f);
	decay = p.addExtParam("decay", "Decay", "", "", { 0.0, 1.0, 0.0, 1.0 }, 0.5f, 0.0f);
	damping = p.addExtParam("damping", "Damping", "", "", { 0.0, 0.999f, 0.0, 1.0 }, 0.5f, 0.0f);
	predelay = p.addExtParam("predelay", "Predelay", "", "", { 0.0, 0.1f, 0.0, 1.0 }, 0.002f, 0.0f);
	dry = p.addExtParam("dry", "Dry", "", "", { 0.0, 1.0, 0.0, 1.0 }, 1.0f, 0.0f);
	wet = p.addExtParam("wet", "Wet", "", "", { 0.0, 1.0, 0.0, 1.0 }, 0.08f, 0.0f);
}

//==============================================================================
void APAudioProcessor::MBFilterParams::setup(APAudioProcessor& p)
{
	enable = p.addIntParam("enable", "Enable", "", "", { 0.0, 1.0, 1.0, 1.0 }, 1.0f, 0.0f, enableTextFunction);
	lowshelffreq = p.addExtParam("lowshelffreq", "Low Shelf Freq", "", "", { 20.0, 20000.0, 1.0, 1.0 }, 20000.0f, 0.0f);
	lowshelfgain = p.addExtParam("lowshelfgain", "Low Shelf Gain", "", "", { 0.01, 4.0, 0.01, 1.0 }, 1.0f, 0.0f);
	lowshelfq = p.addExtParam("lowshelfq", "Low Shelf Q", "", "", { 0.1, 20.0, 0.0, 1.0 }, 1.0f, 0.0f);
	peakfreq = p.addExtParam("peakfreq", "Peak Freq", "", "", { 20.0, 20000.0, 1.0, 1.0 }, 1000.0f, 0.0f);
	peakgain = p.addExtParam("peakgain", "Peak Gain", "", "", { 0.01, 4.0, 0.0, 1.0 }, 1.0f, 0.0f);
	peakq = p.addExtParam("peakq", "Peak Q", "", "", { 0.1, 20.0, 0.0, 1.0 }, 1.0f, 0.0f);
	highshelffreq = p.addExtParam("highshelffreq", "High Shelf Freq", "", "", { 20.0, 20000.0, 1.0, 1.0 }, 20.0f, 0.0f);
	highshelfgain = p.addExtParam("highshelfgain", "High Shelf Gain", "", "", { 0.01, 4.0, 0.0, 1.0 }, 1.0f, 0.0f);
	highshelfq = p.addExtParam("highshelfq", "High Shelf Q", "", "", { 0.1, 20.0, 0.0, 1.0 }, 1.0f, 0.0f);
}

//==============================================================================
void APAudioProcessor::RingModParams::setup(APAudioProcessor& p)
{
	enable = p.addIntParam("enable", "Enable", "", "", { 0.0, 1.0, 1.0, 1.0 }, 1.0f, 0.0f, enableTextFunction);
	modfreq1 = p.addExtParam("modfreq1", "Mod Freq 1", "", "", { 1.0, 12000.0, 0.0, 1.0 }, 40.0f, 0.0f);
	shape1 = p.addExtParam("shape1", "Shape 1", "", "", { 0.0, 1.0, 0.0, 1.0 }, 0.0f, 0.0f);
	mix1 = p.addExtParam("mix1", "Mix 1", "", "", { 0.0, 1.0, 0.0, 1.0 }, 0.0f, 0.0f);
	modfreq2 = p.addExtParam("modfreq2", "Mod Freq 2", "", "", { 1.0, 12000.0, 0.0, 1.0 }, 40.0f, 0.0f);
	shape2 = p.addExtParam("shape2", "Shape 2", "", "", { 0.0, 1.0, 0.0, 1.0 }, 0.0f, 0.0f);
	mix2 = p.addExtParam("mix2", "Mix 2", "", "", { 0.0, 1.0, 0.0, 1.0 }, 0.0f, 0.0f);
	spread = p.addExtParam("spread", "Spread", "", "", { 0.0, 1.0, 0.0, 1.0 }, 0.03f, 0.0f);
	lowcut = p.addExtParam("lowcut", "Low Cut", "", "", { 20.0, 20000.0, 0.0, 1.0 }, 20.0f, 0.0f);
	highcut = p.addExtParam("highcut", "High Cut", "", "", { 20.0, 20000.0, 0.0, 1.0 }, 20000.0f, 0.0f);
    
}

//==============================================================================
void APAudioProcessor::GainParams::setup(APAudioProcessor& p)
{
	enable = p.addIntParam("enable", "Enable", "", "", { 0.0, 1.0, 1.0, 1.0 }, 1.0f, 0.0f, enableTextFunction);
	gain = p.addExtParam("gain", "Gain", "", "", { -60.0, 40.0, 0.0, 1.0 }, 1.0f, 0.0f);
}

//==============================================================================
void APAudioProcessor::FXOrderParams::setup(APAudioProcessor& p)
{
    fxa1 = p.addExtParam("fxa1", "FX A1 choice", "", "", {0.0, 8.0, 1.0, 1.0}, 0.0f, 0.0f, fxListTextFunction);
    fxa2 = p.addExtParam("fxa2", "FX A2 choice", "", "", {0.0, 8.0, 1.0, 1.0}, 0.0f, 0.0f, fxListTextFunction);
    fxa3 = p.addExtParam("fxa3", "FX A3 choice", "", "", {0.0, 8.0, 1.0, 1.0}, 0.0f, 0.0f, fxListTextFunction);
    fxa4 = p.addExtParam("fxa4", "FX A4 choice", "", "", {0.0, 8.0, 1.0, 1.0}, 0.0f, 0.0f, fxListTextFunction);
    fxb1 = p.addExtParam("fxb1", "FX B1 choice", "", "", {0.0, 8.0, 1.0, 1.0}, 0.0f, 0.0f, fxListTextFunction);
    fxb2 = p.addExtParam("fxb2", "FX B2 choice", "", "", {0.0, 8.0, 1.0, 1.0}, 0.0f, 0.0f, fxListTextFunction);
    fxb3 = p.addExtParam("fxb3", "FX B3 choice", "", "", {0.0, 8.0, 1.0, 1.0}, 0.0f, 0.0f, fxListTextFunction);
    fxb4 = p.addExtParam("fxb4", "FX B4 choice", "", "", {0.0, 8.0, 1.0, 1.0}, 0.0f, 0.0f, fxListTextFunction);

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
	orbitParams.setup(*this);

	gainParams.setup(*this);
	waveshaperParams.setup(*this);
	compressorParams.setup(*this);
	stereoDelayParams.setup(*this);
	chorusParams.setup(*this);
	reverbParams.setup(*this);
	mbfilterParams.setup(*this);
	ringmodParams.setup(*this);

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

	waveshaper.reset();
	compressor.reset();
}

void APAudioProcessor::prepareToPlay (double newSampleRate, int newSamplesPerBlock)
{
    Processor::prepareToPlay (newSampleRate, newSamplesPerBlock);

	juce::dsp::ProcessSpec spec;
	spec.sampleRate = newSampleRate;
	spec.maximumBlockSize = newSamplesPerBlock;
	spec.numChannels = 2;

    setCurrentPlaybackSampleRate (newSampleRate);

    modMatrix.setSampleRate (newSampleRate);

    stereoDelay.prepare(spec);
	effectGain.prepare(spec);
	waveshaper.prepare(spec);
	compressor.setSampleRate(newSampleRate);
	compressor.setNumChannels(2);
	chorus.prepare(spec);
	reverb.prepare(spec);
	mbfilter.prepare(spec);
	ringmod.prepare(spec);



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

	levelTracker.trackBuffer(buffer);

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
    
    if (gainParams.enable->isOn())
	{
        effectGain.setGainLevel(modMatrix.getValue(gainParams.gain));
    }

    if (waveshaperParams.enable->isOn()) {
        waveshaper.setGain(modMatrix.getValue(waveshaperParams.drive), modMatrix.getValue(waveshaperParams.gain));
        waveshaper.setDry(modMatrix.getValue(waveshaperParams.dry));
        waveshaper.setWet(modMatrix.getValue(waveshaperParams.wet));
        waveshaper.setFunctionToUse(int(modMatrix.getValue(waveshaperParams.type)));
    }
    
	//setParams (float attackS, float holdS, float releaseS, float thresh, float ratio, float kneeWidth);
    if (compressorParams.enable->isOn()) {
        compressor.setParams(
                             modMatrix.getValue(compressorParams.attack),
                             0.0f,
                             modMatrix.getValue(compressorParams.release),
                             modMatrix.getValue(compressorParams.threshold),
                             modMatrix.getValue(compressorParams.ratio),
                             modMatrix.getValue(compressorParams.knee)
                             );
        compressor.setInputGain(modMatrix.getValue(compressorParams.input));
        compressor.setOutputGain(modMatrix.getValue(compressorParams.output));
        compressor.setMode((gin::Dynamics::Type)(int)modMatrix.getValue(compressorParams.type));
    }
    
	auto notes = gin::NoteDuration::getNoteDurations();
	
    if (stereoDelayParams.enable->isOn()) {
        bool tempoSync = stereoDelayParams.temposync->getProcValue() > 0.0f;
        if (!tempoSync) {
            stereoDelay.setTimeL(modMatrix.getValue(stereoDelayParams.timeleft));
            stereoDelay.setTimeR(modMatrix.getValue(stereoDelayParams.timeright));
        }
        else {
            stereoDelay.setTimeL(notes[size_t(stereoDelayParams.beatsleft->getProcValue())].toSeconds(playhead));
            stereoDelay.setTimeR(notes[size_t(stereoDelayParams.beatsright->getProcValue())].toSeconds(playhead));
        }
        stereoDelay.setFB(modMatrix.getValue(stereoDelayParams.feedback));
        stereoDelay.setWet(modMatrix.getValue(stereoDelayParams.wet));
        stereoDelay.setDry(1.0f);
        stereoDelay.setFreeze(stereoDelayParams.freeze->getProcValue() > 0.0f);
        stereoDelay.setPing(stereoDelayParams.pingpong->getProcValue() > 0.0f);
    }
    
    if (chorusParams.enable->isOn()) {
        chorus.setRate(modMatrix.getValue(chorusParams.rate));
        chorus.setDepth(modMatrix.getValue(chorusParams.depth));
        chorus.setCentreDelay(modMatrix.getValue(chorusParams.delay));
        chorus.setFeedback(modMatrix.getValue(chorusParams.feedback));
        chorus.setWet(modMatrix.getValue(chorusParams.wet));
        chorus.setDry(modMatrix.getValue(chorusParams.dry));
    }
    
    if (reverbParams.enable->isOn()) {
        reverb.setSize(modMatrix.getValue(reverbParams.size));
        reverb.setDecay(modMatrix.getValue(reverbParams.decay));
        reverb.setDamping(modMatrix.getValue(reverbParams.damping));
        reverb.setLowpass(modMatrix.getValue(reverbParams.lowpass));
        reverb.setPredelay(modMatrix.getValue(reverbParams.predelay));
        reverb.setDry(modMatrix.getValue(reverbParams.dry));
        reverb.setWet(modMatrix.getValue(reverbParams.wet));
    }
    
    if (mbfilterParams.enable->isOn()) {
        mbfilter.setParams(
                           modMatrix.getValue(mbfilterParams.lowshelffreq),
                           modMatrix.getValue(mbfilterParams.lowshelfgain),
                           modMatrix.getValue(mbfilterParams.lowshelfq),
                           modMatrix.getValue(mbfilterParams.peakfreq),
                           modMatrix.getValue(mbfilterParams.peakgain),
                           modMatrix.getValue(mbfilterParams.peakq),
                           modMatrix.getValue(mbfilterParams.highshelffreq),
                           modMatrix.getValue(mbfilterParams.highshelfgain),
                           modMatrix.getValue(mbfilterParams.highshelfq)
                           );
    }

    if (ringmodParams.enable->isOn()) {
        RingModulator::RingModParams rmparams;
        rmparams.mod1freq = modMatrix.getValue(ringmodParams.modfreq1);
        rmparams.shape1 = modMatrix.getValue(ringmodParams.shape1);
        rmparams.mix1 = modMatrix.getValue(ringmodParams.mix1);
        rmparams.mod2freq = modMatrix.getValue(ringmodParams.modfreq2);
        rmparams.shape2 = modMatrix.getValue(ringmodParams.shape2);
        rmparams.mix2 = modMatrix.getValue(ringmodParams.mix2);
        rmparams.spread = modMatrix.getValue(ringmodParams.spread);
        rmparams.lowcut = modMatrix.getValue(ringmodParams.lowcut);
        rmparams.highcut = modMatrix.getValue(ringmodParams.highcut);
        ringmod.setParams(rmparams);
    }

    // Output gain
    outputGain.setGain (modMatrix.getValue (globalParams.level));
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

