
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

Four bodies revolve with uniform circular motion, each around a center defined by one of the others or around a fixed central point. Each body around which no other body revolves serves as an oscillator, producing sound. The interpretation of these terminal bodies as oscillators depends on their positions as viewed from a point that is either at the fixed center of revolution or nearby, at a point called the "equant," more in honor of Ptolemy than in strict adherence to his system (hence "*quasi*-Ptolemaic"). In the engine's fully modulated state (i.e., with the "Demodulate" knob turned all the way down), only the angle formed by a reference line and the line connecting the equant to the terminal body matters to the sound, just as, in Ptolemy's system, the distances of the heavenly bodies are unknown. I call this the engine's "modulated" state because it quickly became apparent that this system of sound generation closely resembles frequency modulation (FM) synthesis. The relative speeds of revolution of the various bodies correspond to the frequencies of so-called "carrier" and "modulator" waves in traditional FM synthesis. Like FM synths, this one allows both ("coarse") whole-number and ("fine") fractional variation of these relative frequencies, producing a variety of timbres, from the pure and simple to the densely inharmonic. Myriad mutually modulatable parameters are configurable by the user, and a robust system of randomization facilitates sonic exploration and discovery.

### Gratitude

I'll speak later about some of the sources of inspiration for this project, but don't want to go any further without acknowledging that none of this would have been possible for someone like me&mdash;a humanist with an idea, not a software engineer&mdash;without the wealth of open-source tools, instruction, and advice provided by the community of enthusiasts. I'm especially grateful for the existence of the [JUCE](https://github.com/juce-framework/JUCE) framework and for the many helpful extensions provided by [Gin](https://github.com/FigBug/Gin) and the visualization tools in [Melatonin Blur](https://github.com/sudara/melatonin_blur). Early prototypes were built in [VCV RACK](https://vcvrack.com/), which was a relatively easy way to get started. As someone who needs a lot spelled out, I got quite a lot out [Will Pirkle](https://www.willpirkle.com/)'s books. Finally, mostly by lurking and poring over old posts, but sometimes by posing ill-formed questions and follow-ups of my own, I learned a lot on the Discord server for [The Audio Programmer](https://www.theaudioprogrammer.com/).

# Installation

The plugin is available in VST3 (Windows, Mac OS, Linux) and AU (Mac OS only) formats. The default install locations are as follows: On Windows, place the VST3 file in /Program Files/Common Files/VST3. On Mac OS, place the VST3 file in /Library/Audio/Plug-Ins/VST3 and the AU file in /Library/Audio/Plug-Ins/Components. On Linux, place the VST3 file in /usr/lib/vst3.

# Operation

Enough of that. How does this work? The synth has three main pages: "Main," "FX," and "Rand." The "Main" page contains the main controls for sound generation. The "FX" page contains the built-in effects arranged in one or two chains.

## Main Page

The "Main" page contains the main controls for sound generation. In the left column, there are four envelope generators, each assigned to its corresponding oscillator, as well as being available for other modulations on the "Mods" page. In the central column, we have the oscillator controls. Each has a coarse and fine knob: "Coarse" indicates the whole-number multiple of the base frequency, and "Fine" a fractional addition to that. In normal operation, the base frequency is whatever corresponds to the MIDI input. But if the "Fixed" option is selected, the oscillator will have a frequency equal to 100 times the sum of the "Coarse" and "Fine" settings. The "Vol" knob represents the size of the cycle or epicycle, which can have different effects on the sound, depending on the position of the "Demod" slider. More on that later! And the "Saw" option replaces the smooth, sinusoidal oscillation with the brighter (or raspier) sawtooth wave. The moon icon in the lower left of each oscillator's controls controls the relative phase of the oscillator, which for purer tones (without the "Saw" option) can have a significant effect on the sound.

In the right column are the low-frequency oscillators used for modulation. The "Rate" knobs indicate the frequency in Hz, while the "Min %" and "Max %" knobs control the range of the LFOs' output, normally from 0 to 1, but -1 to 1 if the "Bipolar" option is checked. If the "Sync" option is checked, the LFO resets with each MIDI key press.

In the lower portion of the left column are found two additional sections. The first contains controls all of which directly affect the timbre of the sound. "Equant" refers to the position of the blue "vantage point" seen in the orbit visualization pictured above. "Cutoff" and "Reso(nance)" control the built-in low-pass filter. And "Sine/Square" controls the mix of sine, saw, and square waves in the output. The second section called "Randomize" contains controls to affect all the other controls on the Main page. The slider below the "Randomize" button controls the amount of randomization, while the "LFOs," "ENVs," and "OSCs" controls randomize only their respective settings. The "chaos (X)" option will additionally randomize the oscillators' "Fine" frequency controls. When those have any value besides a whole number, things can get chaotic quick! "Reset" returns every control to its default position.

## Modulation Page

The matrix of knobs that cover most of the page send modulation signals from the sources listed in the left column to the destinations listed across the top. The first 12 of these are to the main controls for each oscillator. Modulations sent to the "CRS" or "Coarse" destinations will be quantized to whole numbers. Space is limited, so the other controls are wedged in the corners. In the top right, "+?" and "-?" buttons add and subtract random modulations, according to the strength of the randomization slider in the lower right, just above the big reset ("0") button. In the lower left, buttons for randomizing the LFOs and oscillators (equivalent to controls on the main page). Also, each row and column has its own buttons to add randomization ("?") and reset all mods ("0").

## Effects Page

The effects (FX) page contains seven built-in effects. The combo boxes at the left allow you to swap the current effect with any you select. Ordering the effects chain differently can open up a lot of possibilities. Check boxes in the upper left of each effect's controls turn the effect on or off. A built-in limiter is meant to keep the levels below 0 dB, but be careful nonetheless.

# Theory

## An "Orbital Modulation" Synthesizer

A brief description of the algorithm: the center of each epicycle is located on the circumference of the next one down the chain, oscillator 4 carries the "planet" around a center that revolves with the frequency of oscillator 3, while oscillator 3's center is located on the next lowest epicycle, controlled by oscillator 2, et cetera. The orbital visualization on the main page makes this relationship clearer than words can easily do. The blue line indicates the distance of the planet from the center of observation. Now, there are two closely related, but subtly different modes of sound generation, controlled by the "Demod" mix slider. When it is in its default position, to the left, the angle of the blue line is read as the phase angle for an oscillation. When it is all the way to the right, *both the angle and the length* of the blue line matter to the sound. (To see this in code, see the latter portion of the `renderNextBlock` function in the `SynthVoice.cpp` file.) This simple difference in interpretation of the orbit has surprising effects on the sound.

## Motivation

I'm just a humanist with an idea for a synth. I learned about Ptolemy's system and was curious how to understand one of the features that made it so successful for so long. Many people who know nothing else about Ptolemy know about epicycles. But epicycles predate his system. What he added was a way of making the model better fit the variations in motion that gave the "planets" their name. As I pointed out above, the equant doesn't have to be at the center of the main circle (the "deferent").

The question that started me on this path was "If the sine wave is the projection of uniform circular motion onto a line, what wave would be produced if we were to observe things off-center? And what would it sound like?" It's not just a vertically displaced sine wave (a DC offset). When the planet is closer to the equant, it also appears to move faster, and slower when it's farther from the equant. Since its position is fixed, this means there is, as it were, a temporal wave shaping going on: part of the sine wave is accelerated and part slowed down. What that produces sonically is the same fundamental frequency and the appearance of higher harmonics. 

In the pictures below, you can see the effect of the position of the equant. The first picture shows a simple sine wave: I've turned all but oscillator 1's volumes to zero. The second shows the sound with the equant above the center, the third with it below. In both cases, the wave has lost its symmetry, one side is a steep ascent, the other a correspondingly lengthened decline.

![sine](https://github.com/gregrecco67/EpicyclesSynth/assets/127459177/17983b86-4da6-44e7-93b6-829a66e77dee)


![equant plus](https://github.com/gregrecco67/EpicyclesSynth/assets/127459177/6d365cd3-93f4-47fb-8c73-5695d90976b2)


![equant minus](https://github.com/gregrecco67/EpicyclesSynth/assets/127459177/4f768d04-7e92-4064-a87c-880f15582478)

## Modulation

Another significant inspiration is the world of modular synthesis. It's a famously expensive habit, but fortunately for all of us who can't or don't want to spend that way, there is VCV Rack. In addition to being very easy to get started with, and boasting a vibrant, creative, and massively helpful community, it's also relatively easy to develop for. As I said, I'm a humanist, not a professional programmer, but I was able to make big strides in prototyping this synth by creating a module for VCV Rack. In any case, I came to appreciate the sonic possibilities of wide-open modulation, and did my best to make some available here.

A competing inspiration is the world of FM synthesis. I didn't set out to create a synth that works via frequency modulation or phase modulation, but that's what interpreting the orbits as sound waves has led to. I joked with some friends that "Orbital Modulation" could be a marketing-friendly term for this "innovative new form of synthesis." In truth, you could probably make most of the sounds possible here (and many more) with any capable FM synth.

What I hope I've accomplished is to expose a lot of significant controls for modulation, in a format that is partly familiar and partly unfamiliar. In playing the synth, I find a lot of single controls have a kind of significant threshold, where a lot of change happens in a small range. The controls expose a lot of "edges," I want to call them. I hope that makes it easy to make sounds that are more expressive and dynamic, and to find the sweet spots.
