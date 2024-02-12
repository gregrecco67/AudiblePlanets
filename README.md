
# Audible Planets

An expressive, quasi-Ptolemaic semi-modular synthesizer.

![p1](https://github.com/gregrecco67/AudiblePlanets/assets/127459177/537a7f4a-6a9f-4248-bd12-12896b3f5863)

![p2](https://github.com/gregrecco67/AudiblePlanets/assets/127459177/7ae18ffb-5eb5-4ede-a289-a1dd21a3c9de)

![p3](https://github.com/gregrecco67/AudiblePlanets/assets/127459177/8a537c07-cd3f-495a-9bf4-f4bed093a88f)

## Features

- "Orbital Modulation" (OM) synthesis
- Randomizable modulation matrix
- Built-in effects chains

## *Raison d'être*

I wanted to know what Ptolemy's "equant" would sound like. More on that below.

## Overview

Four bodies revolve with uniform circular motion, each around one of the others or, in the case of the first body, a fixed central point. Each body around which no other body revolves serves as an oscillator, producing sound. The interpretation of these terminal bodies as oscillators depends on their positions as viewed from a point that is either at the fixed center of revolution or nearby, at a point called the "equant," more in honor of Ptolemy than in strict adherence to his system (hence "*quasi*-Ptolemaic"). In the engine's fully modulated state (i.e., with the "Demodulate" knob turned all the way down), only the angle formed by a reference line and the line connecting the equant to the terminal body matters to the sound, just as, in Ptolemy's system, the distances of the heavenly bodies are unknown. This system of sound generation closely resembles frequency modulation (FM) synthesis. The relative speeds of revolution of the various bodies have their analogues in the frequencies of so-called "carrier" and "modulator" (or "operator") waves in traditional FM synthesis. Accordingly, the interface allows both ("coarse") whole-number and ("fine") fractional variation of these relative frequencies, producing a wide array of timbres, from the pure and simple to the densely inharmonic. Many mutually modulatable parameters are configurable by the user, and a robust system of randomization facilitates sonic exploration and discovery.

### Gratitude

I'll speak later about some of the sources of inspiration for this project, but don't want to go any further without acknowledging that none of this would have been possible for someone like me&mdash;a humanist with an idea, not a software engineer&mdash;without the wealth of open-source tools, instruction, and advice provided freely by the community of professionals and enthusiasts. I'm especially grateful for the existence of the [JUCE](https://github.com/juce-framework/JUCE) framework and for the many helpful extensions to it provided by [Gin](https://github.com/FigBug/Gin) and the visualization tools in [Melatonin Blur](https://github.com/sudara/melatonin_blur). Early prototypes were built in [VCV RACK](https://vcvrack.com/), which was a relatively easy way to get started with audio programming and boasts a vibrant community. As a novice who needs a lot spelled out, I got quite a lot out of [Will Pirkle](https://www.willpirkle.com/)'s teacherly books. Finally, mostly by lurking and poring over old posts, but sometimes by posing ill-formed questions and follow-ups of my own, I learned a lot on the Discord server for [The Audio Programmer](https://www.theaudioprogrammer.com/), mostly that other people who do this kind of thing regularly know a lot more than I am ever likely to.

# Operation

Enough of that. How to use this thing?

## Installation

The plugin is available in VST3 (Windows, Mac OS, Linux) and AU (Mac OS only) formats. Since there is no installer, you will have to place the plug-in file in the right place yourself. On Windows, place the VST3 file in /Program Files/Common Files/VST3. On Mac OS, place the VST3 file in ~/Library/Audio/Plug-Ins/VST3 and/or the AU file in ~/Library/Audio/Plug-Ins/Components. (You will need the AU component if you plan to run the synth in Logic or GarageBand.) On Linux, place the VST3 file wherever your DAW looks for it, which should include ~/.vst3 as a default. More info on default VST3 file locations [here](https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical+Documentation/Locations+Format/Plugin+Locations.html)

Or, you can build it from source:
```
git clone https://github.com/gregrecco67/AudiblePlanets.git
cd AudiblePlanets
git submodule update --init --recursive
```
Then, if on Mac OS:
```
cmake -B build -G Xcode
```
If using Visual Studio, just open the folder. If you're on Linux, you probably know what to do. The CMake project defaults to producing a standalone version of the plugin, too, but on Windows at least, the performance is poor. If you don't want it or any other particular format built, just remove the entry from the `FORMAT` line in `juce_add_plugin` in `CMakeLists.txt`. The project is configured to copy the plugin to the right place after building it.

## Installing Presets

A few example presets are located [here](https://github.com/gregrecco67/Audible-Planets-Presets) and can be installed (manually). On Windows, the install location should be ```%HOMEPATH%\AppData\Roaming\com.void-star\Audible Planets\programs```. On Mac OS, it should be ```~/Library/Application Support/com.void-star/Audible Planets/programs```.

## Pages

The synth has three main pages: "Main," "FX," and "Rand." The Main page contains the main controls for sound generation. The FX page contains the built-in effects arranged in one or two chains. The Randomization page contains controls for randomizing the main parameters and modulation settings in a variety of groups, as well as a slightly different interface for the envelopes and oscillators.

### Main Page

Mostly standard envelope, oscillator, and LFO controls take up the majority of screen real-estate here. I'll describe some controls that are less often seen. Envelopes can be made to repeat, either freely ("Free") or in subdivisions or multiples of the beat ("Sync"). With the "Tones" knob set to minimum and the "Saw" switch set to "Off," the motion of the body is uniform and circular. In this mode, adjusting the relative starting points of the various oscillations (with the "Phase" control) can change the sound a lot. When "Tones" is turned up, the motion of the body is no longer uniform and circular, but is modulated with extra whole-number multiples of the main frequency up to 5. The "Saw" switch turns the motion into a sawtooth wave with its vertical and horizontal components in quadrature. Both enrich the spectrum of the oscillator, but have different effects on the final sound depending on the level of the "Demodulate" control, which effectively determines the amount of phase modulation. In addition to these ways to enrich the individual "spectrum" of each oscillator, there is a general "Blend" control in the "Timbre" section, which takes the interpretation of the bodies' positions as phases, not of a pure sine wave, but of a mix of sine, saw, and square waves. The presence of "Detune," "Spread," and "Pan" controls for each oscillator indicates that the orbital algorithm described in the overview above is computed for multiple voices over two channels, and slight variations in position and frequency between the latter can bring movement, width, and depth to the sounds produced. Finally, the LFOs have delay and fade-in controls, which affect the per-voice (or "poly") modulation source. (More on that distinction when we discuss modulation.)

In the Timbre section, in addition the "Blend" control, there's one that modifies the position of the equant, the point from which the motions of bodies are observed. The function of the "Demodulate" knob has already been described, but I will add that in effect, turning it up tends to mellow the sound, removing the sideband frequencies produced by the orbital algorithm, making it function more like a rudimentary additive synth. Finally, the "Algorithm" control switches between different arrangements of the oscillators, with the "terminal" body or bodies being indicated by parentheses. In many cases, the first algorithm 1-2-3-(4) is the harshest, and the last the least harsh, but the effects are varied, so it's worth trying out different algorithm settings as you're trying to dial in a sound.

The global parameters include controls for monosynth and legato modes, glissando and portamento at an adjustable rate, velocity sensitivity, pitchwheel range, and overall volume. There's also a dedicated per-voice filter, with cutoff, resonance, and key tracking controls, and a variety of common filter types.

The partially open circles found in various panel headers, as the section in the top right indicates, sources of modulation. The single-line versions are "mono" and the double-line versions "poly," which is to say the latter are different for each key press (or subsequent pressure, tilt, or slide, on MPE-capable MIDI controllers). To use any of them, click and drag from the source to the destination. Almost any knob can have its setting modulated, and that goes for the effects controls on the FX page as well. When a parameter is modulated, a small orange circle appears in the upper-right corner of the control. Clicking and dragging it (or using a mouse scroll wheel while hovering) will increase or decrease the modulation amount, from -1.0 to 1.0. (A more spacious set of modulation controls lives on the "Rand" page.) Right-clicking it will give the option to remove any or all of the modulation sources attached to it. A small white dot on the periphery of the control indicates the modulated value or values of the parameter for whichever voices are active.


### Effects Page

The effects (FX) page contains slots for up to eight built-in effects, arranged in one (when "FX Chain Routing" is set to "A -> B") or two chains, each with its own gain control, applied before the signal enters the chain. Ordering the effects chains differently can open up a lot of possibilities. Since the effects parameters can be modulation destinations, there's a mod sources panel on this page, too. Also, since there's room for it, there's a modulation matrix panel, showing the currently active modulations, and offering controls to bypass, adjust, or delete them. In combination, the effects on this page can add a lot of boost, especially when modulated with sources that can constructively interfere. A built-in limiter is accordingly always on and meant to keep the levels below 0 dB, but be careful nonetheless.

### Randomization Page

Sound design can at times be a game of "almost, but not quite." At other times, good results can come from unexpected synergies and, frankly, from a more kitchen-sink approach. The randomization page is meant to facilitate both: gentle nudges and total crapshoots. The large modulation matrix panel offers the most fine-grained interface for adjustment, while the buttons in the columns to its right change several or even many settings at once. Generally, things in the left column add modulation or randomization and things in the right remove it. The sliders for "Number of Mods" and "Randomization Chance/Amount" control the "how many" and "how much" of these randomization buttons. The "Inharmonic" checkbox allows modulations and randomizations to be applied to the oscillator "Fine" controls and the global "Pitch" control, which quickly produce inharmonic or at least atonal results. The "Reset Inharonic" button below it removes any such mods without deleting any others. Other buttons with arrows in their names refer to groups of parameters: "LFO" and "ENV" are self-explanatory, "Keys" includes velocity, MIDI note number, the pitchbend and mod wheels, and MPE pressure and timbre. (The latter, as well as per-note pitchbend, can be activated using the "MPE" option in the top-left menu). The parameters of whatever effects are currently active can be modulated with "Randomize FX Mods" and the order and selection of effects can be randomized with "Randomize FX Choice." The bottom row of buttons doesn't add any new modulations, but increases or decreases those that are already active. Since it's annoying to have to switch pages while one is adjusting parameters, a second set of mostly complete envelope and oscillator controls are on this page. That said, if you click once anywhere within the plugin screen, you can change tabs with the "1," "2," and "3" keys on your keyboard.


# Motivation / Inspiration

This began as a sort of recreational programming task. I learned about Ptolemy's system and wanted to understand one of the features that made it so successful for so long (millennia). Many people who know nothing else about Ptolemy know about his use of epicycles. But epicycles predate his system. What he added was a way of making the model better fit the variations in motion that gave the "planets" their name, by use of what he called the "equant" point. As I pointed out above, the equant doesn't have to be at the center of the main circle (the "deferent"), such that the planet's motion appears even more accelerated or decelerated at certain points in its trajectory than it is by the mere interaction of deferent and epicycle alone. (In Ptolemy's system of planetary motion, the equant and center of observation function differently, but with a similar effect.)

The particular two-fold question I wanted to investigate was this: "If the sine wave is the projection of uniform circular motion onto a line, what wave would be produced if we were to observe things off-center, from the perspective of something like the equant point? And what would it sound like?" It's not just a vertically displaced sine wave (a DC offset). When the planet is closer to the equant, it also appears to move faster, and slower when it's farther. This means there is a sort of temporal "wave shaping" going on: part of the sine wave is accelerated and part slowed down. If you've played with sound waves geometrically like this, you might not be surprised to learn that, sonically, that reshaping results in the same fundamental frequency and the appearance of higher harmonics. In more complex cases, the equant may affect different harmonics differently.

In the pictures below, you can see the effect of the position of the equant on the wave shape. The first picture shows a simple sine wave: I've turned all but oscillator 1's volumes to zero. The second shows the sound with the equant above the center, the third with it below. In both cases, the wave has lost its symmetry, one side is a steep ascent, the other a correspondingly lengthened decline.

![sine](https://github.com/gregrecco67/AudiblePlanets/assets/127459177/daa2e90c-5b87-4bd2-9122-d1f0cbca5a28)

![equant plus](https://github.com/gregrecco67/AudiblePlanets/assets/127459177/3adc426e-da6a-4cf4-91db-f10f24eeae05)

![equant minus](https://github.com/gregrecco67/AudiblePlanets/assets/127459177/9bc5d903-082e-401c-856b-aa52b171c021)

I don't think there exists another synth with an "equant" control, but I have to admit that, in the end, its usefulness is limited. Its effect tends to get lost when the interplay of other parameters produces sawtooth-like wave shapes, but when starting from purer tones, it can impart a (somewhat?) distinctive character to the sound. The big surprise for me was how consequential it would turn out to be to either ignore or include the orbiting bodies' distance from the observer in the way the model produces sound (as controlled by the "Demodulate" knob).

Another sort of oblique inspiration was [this cover](https://www.nytimes.com/1979/04/24/archives/computer-synthesizes-music-of-the-spheres-music-of-spheres.html) of the NYT "Science Times" from April 24, 1979. Just seeing familiar notation allegedly representing the "music of the spheres" was intensely provocative and the image stayed with me, as well as the idea that, somehow, people were using computers to investigate it and actually generate sound. I was a fan of Isao Tomita's take on Holst's "The Planets," which surely somehow contributed to my interest.

![NYT cover planets](https://github.com/gregrecco67/AudiblePlanets/assets/127459177/f82a7979-d93d-43dd-b0f2-992d408030ed)

A complementary inspiration is the world of FM synthesis. I didn't set out to create a synth that works via frequency modulation or phase modulation, but that's what interpreting the orbits as sound waves has led to. I joked with some friends that "Orbital Modulation" could be a marketing-friendly term for this "innovative new form of synthesis." In truth, you could probably make most of the sounds possible here (and many more) with any capable FM synth, and in any case, the plug-in is available free of charge. What I think I've accomplished by way of novelty is to expose a lot of controls for adjustment and modulation in an arrangement that is easier to use than some and musically or sonically relevant.

In playing the synth, I find a lot of single controls (especially the equant, demodulation, and individual oscillator volume and phase controls) have a kind of significant threshold, where a lot of change happens in a small range. The controls expose a lot of "edges." Connecting these parameters with the MIDI Polyphonic Expression (MPE) modulation sources or others opens up a lot of possibilities. I hope that makes it easy to make sounds that are more or differently expressive and dynamic, and to find the sweet spots. If you've made it this far, thank you for reading. I'd be interested to hear what you think of it all. Happy music-making and "safe journeys, space fans, wherever you are."
