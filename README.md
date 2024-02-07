
# Audible Planets

An expressive, quasi-Ptolemaic semi-modular synthesizer.

![main-page](https://github.com/gregrecco67/AudiblePlanets/assets/127459177/3287a8a0-837d-4fd8-8670-84d802ce3b7c)
![fx-page](https://github.com/gregrecco67/AudiblePlanets/assets/127459177/8d2ab258-e7f8-4b68-99ab-0363f46b075e)
![rand-page](https://github.com/gregrecco67/AudiblePlanets/assets/127459177/7a47de6d-e101-44be-b80f-0336484f7af6)

## Features

- "Orbital Modulation" (OM) synthesis
- Randomizable modulation matrix
- Built-in effects chain

## *Raison d'être*

I wanted to know what Ptolemy's "equant" would sound like. More on that below.

## Overview

Four bodies revolve with uniform circular motion, each around one of the others or, in the case of the first body, a fixed central point. Each body around which no other body revolves serves as an oscillator, producing sound. The interpretation of these terminal bodies as oscillators depends on their positions as viewed from a point that is either at the fixed center of revolution or nearby, at a point called the "equant," more in honor of Ptolemy than in strict adherence to his system (hence "*quasi*-Ptolemaic"). In the engine's fully modulated state (i.e., with the "Demodulate" knob turned all the way down), only the angle formed by a reference line and the line connecting the equant to the terminal body matters to the sound, just as, in Ptolemy's system, the distances of the heavenly bodies are unknown. I call this state "modulated" because it quickly became apparent that this system of sound generation closely resembles frequency modulation (FM) synthesis. The relative speeds of revolution of the various bodies have their analogues in the frequencies of so-called "carrier" and "modulator" (or "operator") waves in traditional FM synthesis. Like many FM synths, this one allows both ("coarse") whole-number and ("fine") fractional variation of these relative frequencies, producing a wide array of timbres, from the pure and simple to the densely inharmonic. Many mutually modulatable parameters are configurable by the user, and a robust system of randomization facilitates sonic exploration and discovery.

### Gratitude

I'll speak later about some of the sources of inspiration for this project, but don't want to go any further without acknowledging that none of this would have been possible for someone like me&mdash;a humanist with an idea, not a software engineer&mdash;without the wealth of open-source tools, instruction, and advice provided freely by the community of professionals and enthusiasts. I'm especially grateful for the existence of the [JUCE](https://github.com/juce-framework/JUCE) framework and for the many helpful extensions provided by [Gin](https://github.com/FigBug/Gin) and the visualization tools in [Melatonin Blur](https://github.com/sudara/melatonin_blur). Early prototypes were built in [VCV RACK](https://vcvrack.com/), which was a relatively easy way to get started with audio programming and boasts a vibrant community. As someone who needs a lot spelled out, I got quite a lot out of [Will Pirkle](https://www.willpirkle.com/)'s books. Finally, mostly by lurking and poring over old posts, but sometimes by posing ill-formed questions and follow-ups of my own, I learned a lot on the Discord server for [The Audio Programmer](https://www.theaudioprogrammer.com/).


# Operation

Enough of that. How to use this thing?

## Installation

The plugin is available in VST3 (Windows, Mac OS, Linux) and AU (Mac OS only) formats. The default install locations are as follows: On Windows, place the VST3 file in /Program Files/Common Files/VST3. On Mac OS, place the VST3 file in /Library/Audio/Plug-Ins/VST3 and/or the AU file in /Library/Audio/Plug-Ins/Components. (You will need the AU component if you plan to run the synth in Logic.) On Linux, place the VST3 file in /usr/lib/vst3.

## Pages

The synth has three main pages: "Main," "FX," and "Rand." The Main page contains the main controls for sound generation. The FX page contains the built-in effects arranged in one or two chains. The Randomization page contains controls for randomizing the main parameters and modulation settings in a variety of groups, as well as a slightly different interface for the envelopes and oscillators.

### Main Page

Mostly standard envelope, oscillator, and LFO controls take up the majority of screen real-estate here. I'll describe what is less often seen. Envelopes can be made to repeat, either freely or in subdivisions or multiples of the beat. The main oscillator controls are colored to match the orbit visuzalization in the lower right. With the "Tones" knob set to 1.0 and the "Saw" switch set to "Off," the motion of the body is uniform and circular, but when the former is turned up, it is modulated with extra whole-number multiples of the main frequency up to 5. The "Saw" switch turns the motion into a sawtooth wave in the vertical direction. In addition to these ways to enrich the individual spectum of each oscillator, there is a general "Blend" control in the "Timbre" section, which takes the interpretation of the bodies' positions as phases, not of a pure sine wave, but of a mix of sine, saw, and square waves. "Detune," "Spread," and "Pan" controls should make it clear that the orbital algorithm described in the overview above is computed for multiple voices over two channels, and slight variations between the latter can bring movement, width, and depth to the sounds produced. Finally, the LFOs have delay and fade-in controls, which affect the per-voice (or "poly") modulation source. (More on that distinction when we discuss modulation.)

In the Timbre section, in addition the "Blend" control, there's one that modifies the position of the equant, the point from which the motions of bodies are observed. The function of the "Demodulate" knob has already been described, but I will add that in general, turning it up mellows the sound, tending to remove the sideband frequencies produced by the orbital algorithm, making it function more like a rudimentary additive synth. Finally, the "Algorithm" control switches between different arrangements of the oscillators, with the "terminal" body or bodies being indicated by parentheses. In many cases, the first algorithm 1-2-3-(4) is the harshest, and the last the least harsh, so it's worth trying out variations as you're trying to dial in a sound.

The global parameters include a switch for monosynth mode, glissando and portamento at an adjustable rate, velocity sensitivity and overall volume. There's also a dedicated per-voice filter, with cutoff, resonance, and key tracking controls, and a variety of common types.

The partially open circles strewn about the interface are, as the section in the top right indicates, sources of modulation. The single-line versions are "mono" and the double-line versions "poly," which is to say the latter are different for each key press (or subsequent pressure, tilt, or slide, on MPE-capable MIDI controllers). To use any of them, click and drag from the source to the destination. Almost any knob can have its setting modulated, and that goes for the effects controls on the FX page as well. When a parameter is modulated, a small orange circle appears in the upper-right corner of the control. Clicking and dragging it (or using a mouse scroll wheel while hovering) will increase or decrease the modulation amount, from -1.0 to 1.0. (A more spacious set of modulation controls lives on the "Rand" page.) Right-clicking it will give the option to remove any or all of the modulation sources attached to it. A small white dot on the periphery of the control indicates the modulated value or values of the parameter for whichever voices are active.


### Effects Page

The effects (FX) page contains slots for up to eight built-in effects, arranged in one (when "A -> B" is on) or two chains . Ordering the effects chains differently can open up a lot of possibilities. Since the effects parameters can be modulation destinations, there's a mod sources panel on this page, too. Also, since there's room for it, there's a modulation matrix panel, showing the currently active modulations, and offering controls to bypass, adjust, or delete them. In combination, the effects on this page can add a lot of boost, especially when modulated with sources that can constructively interfer. A built-in limiter is accordingly always on and meant to keep the levels below 0 dB, but be careful nonetheless.

### Randomization Page

Sound design can at times be a game of "almost, but not quite." At other times, good results can come from unexpected synergies and, frankly, from a more kitchen-sink approach. The randomization page is meant to facilitate both: gentle nudges and total crapshoots. The large modulation matrix panel offers the most fine-grained interface for adjustment, while the buttons in the column to its right change several or even many settings at once. Generally, things in the left column add modulation or randomization and things in the right remove it. The sliders for "Number of Mods" and "Randomization Chance/Amount" control the "how many" and "how much" of the randomization buttons. The "Inharmonic" checkbox allows modulations and randomizations to be applied to the oscillator "Fine" controls and the global "Pitch" control, which quickly produce inharmonic or at least atonal results. The "Reset Inharonic" button below it removes any such mods without deleting any others. Other buttons with arrows in their names refer to groups of parameters: "LFO" and "ENV" are self-explanatory, "Keys" includes velocity, MIDI note number, pitch bend, MPE pressure and timbre and the mod wheel. The parameters of whatever effects are currently active can be modulated with "Randomize FX Mods" and the order and selection of effects can be randomized with "Randomize FX Choice." The bottom row of buttons doesn't add any new modulations, but increases or decreases those that are already active. Since it's annoying to have to switch pages while one is adjusting parameters, a second set of mostly complete envelope and oscillator controls are on this page. That said, if you click once anywhere within the plugin screen, you can change tabs with the "1," "2," and "3" keys.


# Motivation / Inspiration

I learned about Ptolemy's system and was curious how to understand one of the features that made it so successful for so long (millenia). Many people who know nothing else about Ptolemy know about epicycles. But epicycles predate his system. What he added was a way of making the model better fit the variations in motion that gave the "planets" their name. As I pointed out above, the equant doesn't have to be at the center of the main circle (the "deferent"), such that the planet's motion appears even more accelerated or decelerated at certain points in its trajectory.

The question that started me on this path was "If the sine wave is the projection of uniform circular motion onto a line, what wave would be produced if we were to observe things off-center, from the perspective of the equant point? And what would it sound like?" It's not just a vertically displaced sine wave (a DC offset). Again, when the planet is closer to the equant, it also appears to move faster, and slower when it's farther from the equant. This means there is, as it were, a sort of temporal wave shaping going on: part of the sine wave is accelerated and part slowed down. What that produces sonically in the simplest case is the same fundamental frequency and the appearance of higher harmonics. In more complex cases, the equant may affect different harmonics differently.

In the pictures below, you can see the effect of the position of the equant. The first picture shows a simple sine wave: I've turned all but oscillator 1's volumes to zero. The second shows the sound with the equant above the center, the third with it below. In both cases, the wave has lost its symmetry, one side is a steep ascent, the other a correspondingly lengthened decline.

![sine](https://github.com/gregrecco67/AudiblePlanets/assets/127459177/daa2e90c-5b87-4bd2-9122-d1f0cbca5a28)

![equant plus](https://github.com/gregrecco67/AudiblePlanets/assets/127459177/3adc426e-da6a-4cf4-91db-f10f24eeae05)

![equant minus](https://github.com/gregrecco67/AudiblePlanets/assets/127459177/9bc5d903-082e-401c-856b-aa52b171c021)

I don't think there exists another synth with an "equant" control, but I have to admit that, in the end, its usefulness is limited. Its effect tends to get lost when the interplay of other parameters produces sawtooth-like wave shapes, but when starting from purer tones, it can impart a distinctive character to the sound. The big surprise for me was how consequential it would turn out to be to either ignore or include the orbiting bodies' distance from the observer in the way the model produces sound (as controlled by the "Demodulate" knob).

Another sort of oblique inspiration was [this cover](https://www.nytimes.com/1979/04/24/archives/computer-synthesizes-music-of-the-spheres-music-of-spheres.html) of the NYT "Science Times" from April 24, 1979. Just seeing familiar notation allegedly representing the "music of the spheres" was intensely provocative and the image stayed with me, as well as the idea that, somehow, people were using computers to investigate it.

![NYT cover planets](https://github.com/gregrecco67/AudiblePlanets/assets/127459177/f82a7979-d93d-43dd-b0f2-992d408030ed)

A complementary inspiration is the world of FM synthesis. I didn't set out to create a synth that works via frequency modulation or phase modulation, but that's what interpreting the orbits as sound waves has led to. I joked with some friends that "Orbital Modulation" could be a marketing-friendly term for this "innovative new form of synthesis." In truth, you could probably make most of the sounds possible here (and many more) with any capable FM synth. What I hope I've accomplished by way of novelty is to expose a lot of significant controls for modulation, in a format that is musically or sonically significant. 

In playing the synth, I find a lot of single controls (especially the equant, demodulation, and individual oscillator volume controls) have a kind of significant threshold, where a lot of change happens in a small range. The controls expose a lot of "edges." Connecting these parameters with the MIDI Polyphonic Expression modulation sources or others should open up a lot of possibilities. I hope that makes it easy to make sounds that are more expressive and dynamic, and to find the sweet spots. If you've made it this far in your reading, thank you for your interest. I'd be interested to hear what you think of it all. Happy music-making and "safe journeys, space fans, wherever you are."
