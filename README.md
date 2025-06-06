
# Audible Planets (v1.2!)

An expressive, quasi-Ptolemaic semi-modular synthesizer.

![main1](https://github.com/user-attachments/assets/2e30bb64-5973-412b-8d30-1bcb8c8093c1)

![main2](https://github.com/user-attachments/assets/739b269e-6ad7-4054-8b57-316d8c0dbd64)

![MODS](https://github.com/user-attachments/assets/0a483717-896c-4fc1-8e26-182f908c3b67)

![fx](https://github.com/user-attachments/assets/f3c06b90-ab56-4163-ac65-d8b7aebc680e)

## Overview

The synthesizer produces sound based on the position of "planets" moving in a sort of orbital simulation. Each of the planets revolves around either one of the other planets or a common center. To make sound, the engine views the planets from a point that is either at that common center of revolution or nearby, at a point called the "equant," and interprets their positions in one of two ways: either as an angle (left channel) and its perpendicular (right channel) or as a position (with each coordinate in one of the channels). Because of the likeness with FM synthesis, the first is called "modulated," and the second "demod," for short. The picture below shows one channel of both sources, with the other channel on the horizontal:

![orbital model](https://github.com/user-attachments/assets/ca2d5da8-9e2d-4d1b-b7c5-1708f5f7479a)

The relative speeds of revolution of the various bodies have their analogues in the frequencies of so-called "carrier" and "modulator" (or "operator") waves in traditional FM synthesis, so the interface allows both ("coarse") whole-number and ("fine") fractional variation of these relative frequencies, producing a wide array of timbres, from the pure and simple to the densely inharmonic. An animation of a simplified orbital model (only two planets, no equant) [can be viewed here](https://www.desmos.com/calculator/avsrrqq3nn).

## Features

- "Orbital Modulation" (OM) synthesis :tm: :wink:
- Modulation matrix with monophonic and polyphonic modulation for most parameters
- Built-in modulatable effects chains
- Changes in version 1.2: simplified control scheme for oscillators, somewhat less aliasing, refined effects (waveshaper and chorus).

## *Raison d'être*

I wanted to know what Ptolemy's "equant" would sound like. More on that below.

## Demos
- Audible Planets was chosen for the KVR Forums' "One Synth Challenge" in May 2024. Competitors submit tracks made using only one synth, and it's pretty impressive what they are able to accomplish under those constraints. [Here's a playlist](https://soundcloud.com/kvrosc/sets/one-synth-challenge-183).
- [Demo Track by Riley Hutton](https://youtu.be/2T0ZzTWPuJg) - All sounds were generated by Audible Planets (excluding drums and transition FX).

## Installation
The plugin is available in a variety of formats [here](https://github.com/gregrecco67/AudiblePlanets/releases). Since there is no installer, you will have to place the plugin file in the right place yourself. On Windows, place the VST3 file in /Program Files/Common Files/VST3. On Mac OS, place the VST3 file in ~/Library/Audio/Plug-Ins/VST3 and/or the AU file in ~/Library/Audio/Plug-Ins/Components. (You will need the AU component if you plan to run the synth in Logic or GarageBand.) On Linux, place the VST3 file wherever your DAW looks for it, which should include ~/.vst3 as a default. More info on default VST3 file locations [here](https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical+Documentation/Locations+Format/Plugin+Locations.html) and LV2 locations [here](https://lv2plug.in/pages/filesystem-hierarchy-standard.html). On a Mac, you will need to take the further step of authorizing the plugin to run, either from the Privacy & Security settings panel, or by typing `xattr -dr com.apple.quarantine <file location>` in a Terminal window.

Alternatively, you can build it from source:
```
git clone --recurse-submodules https://github.com/gregrecco67/AudiblePlanets.git
cd AudiblePlanets
```
Then, if on Mac OS:
```
cmake -B build -G Xcode .
open build/AudiblePlanets.xcodeproj
```
and compile the project in Xcode.

Or, on [Linux](https://github.com/juce-framework/JUCE/blob/master/docs/Linux%20Dependencies.md) (using Ninja):
```
cmake -B ninja-build -DCMAKE_BUILD_TYPE=Release -G Ninja .
cd ninja-build
ninja
```

On Windows, using Visual Studio, just open the folder and wait for it to parse the CMake file. The project is configured to install the plugin in the right place after building it.

# Operation

## Pages

As seen above, the synth has three pages: "Main," "Mods," and "Effects." The Main page contains the main controls for sound generation. The Mods page contains controls for configurable modulation sources like LFOs and MSEGs. The Effects page contains the built-in effects arranged in one or two chains.

### Main Page

The four revolving bodies are controlled by the four panes of the OSC section. The radii of their orbits can be modulated by any of the four envelopes to the right. Their relative speeds can be adjusted with the "Coarse" and "Fine" controls, or an absolute frequency can be set with the "Fixed" control. Normally (when the wave is set to "Sine"), the motion of the body is uniform and circular. In this mode, adjusting the relative starting points of the various oscillations (with the "Phase" control) can change the sound a lot. When a different wave is selected, the sound is richer and individual phase differences matter less.

In the Timbre section, you can modify the position of the equant and the mix of modulated and demodulated signal. Since the size of all the orbits is variable, a planet or planets may pass very close to the equant, or even through it, resulting in sharp corners and/or discontinuities (and some aliasing, less in v1.2). This could (and can) be avoided by making the outer orbits smaller, but that also reduces their contribution to the overall sound. Finally, the "Algorithm" control switches between different arrangements of the oscillators, with different "terminal" bodies. In many cases, the first algorithm (1-2-3-4) is the harshest, and the last the least harsh (since the modulation is less compounded), but the effects vary depending on the relative sizes of the orbits and other factors, so it's worth trying out different algorithm settings as you're trying to dial in a sound. There's also a dedicated filter, with cutoff, resonance, and key tracking controls, and a variety of common filter types. There's also an auxiliary oscillator, for the sake of layering.

Almost every control on the main page can by modulated differently for each individual voice ("poly") or the same for all ("mono"). A quick aside about that:

<ul>On all three pages, the "Mod Sources" and "Mod Matrix" panels appear. Click on any of the icons to the right of modulation sources to select them for assignment to one or more controls, which can be done by clicking and dragging on any knob. Modulation sources and destinations will appear in the mod matrix, which offers further controls for different modulation curves and amounts and for selecting between unipolar or bipolar modulation. Note that many modulation destinations, and all effects controls, can only be modulated "mono."</ul>

### Mods Page

The "Mods" page contains the controls for the LFOs and MSEGs, as well as a few others. In the headers of most mod source panels (ENVs, LFOs, MSEGs), there are controls to create connections, either by direct selection, or by putting the synth in "Learning" mode, where clicking and dragging controls applies modulation. There are also some overall controls for the synth in "Global." Finally, the "Macros" panels allows modulation of several parameters at once by one knob, which can be assigned to respond to a MIDI CC message. Just click "Learn" beneath the knob, and then activate the control you want to assign to it on your MIDI controller. A CC number will appear below the knob to indicate the assignment was successful. Click on the "Clear" button below it to remove the assignment.

### Effects Page

The "Effects" page contains slots for up to eight built-in effects, arranged in one or two chains ("A -> B" or "A || B"), each with its own gain control, applied before the signal enters the chain. Ordering the effects chains differently can open up a lot of possibilities. Since the effects parameters can be modulation destinations, there's a mod sources panel on this page, too, as well as a modulation matrix panel, showing the currently active modulations, and offering controls to bypass, adjust, or delete them. In combination, the effects on this page can add a lot of boost, especially when modulated with sources that can constructively interfere. A built-in limiter is accordingly always on and meant to keep the levels below 0 dB, but be careful nonetheless.

## Presets

Presets can be viewed either as a flat list, by clicking on the current preset name, or in a browser that divides them by author and category, by clicking on the folder icon to the left of the current preset name. In case you want to install any manually, on Windows, the folder location should be ```%HOMEPATH%\AppData\Roaming\com.void-star\Audible Planets\programs```. On Mac OS, it should be ```~/Library/Application Support/com.void-star/Audible Planets/programs```. On Linux, it should be ```~/.config/com.void-star/Audible Planets/programs```. In any case, you can right-click on any preset name in the browser pane and select the option to show the file location. Also, if you come up with any patches you'd like to share, please feel free to make a pull request on that repo or just message me here on Github.

# Motivation / Inspiration

This began as a sort of recreational programming task. I had learned about Ptolemy's system and wanted to understand one of the features that made it so successful for so long (millennia). Many people who know nothing else about Ptolemy know about his use of epicycles. But epicycles predate his system. What he added was a way of making the model better fit the variations in motion that gave the "planets" their name, by use of what he called the "equant" point. As I pointed out above, the equant doesn't have to be at the center of the main circle (the "deferent"), with the result that the planet's motion appears even more accelerated or decelerated at certain points in its trajectory than it is by the mere interaction of deferent and epicycle alone. (In Ptolemy's system of planetary motion, the equant and center of observation function differently, but with a similar effect.) I set out to investigate this sonically.

I don't think there exists another synth with an "equant" control, but I have to admit that, in the end, its usefulness is limited. Its effect tends to get lost when the interplay of other parameters produces richer wave shapes, but when starting from purer tones, it can impart a distinctive character to the sound. The big surprise for me was how consequential it would turn out to be to either ignore or include the orbiting bodies' distance from the observer in the way the model produces sound (as controlled by the "Demod Mix" knob).

Another sort of oblique inspiration was [this cover](https://www.nytimes.com/1979/04/24/archives/computer-synthesizes-music-of-the-spheres-music-of-spheres.html) of the NYT "Science Times" from April 24, 1979. Just seeing familiar notation allegedly representing the "music of the spheres" was intensely provocative and the image stayed with me, as well as the idea that, somehow, people were using computers to investigate it and actually generate sound. I was a fan of Isao Tomita's take on Holst's "The Planets," which surely somehow contributed to my interest.

![NYT cover planets](https://github.com/gregrecco67/AudiblePlanets/assets/127459177/f82a7979-d93d-43dd-b0f2-992d408030ed)

A complementary inspiration is the world of FM (or PM) synthesis. I didn't set out to create an FM synth, but that's what interpreting the orbits as sound waves has led to. I joked with some friends that "Orbital Modulation" could be a marketing-friendly term for this "innovative new form of synthesis," though I hasten to add that :wink: OM is not really trademarked, as far as I know. In truth, you could probably make most of the sounds possible here (and many more) with any capable FM synth. But what I think I've accomplished by way of novelty is to expose a lot of musically or sonically relevant controls for adjustment and modulation in an arrangement that is easier to use than some. Also, the orbital model effectively limits the amount of phase modulation that is possible, and such restrictions can be aesthetically productive.

In playing the synth, I find a lot of single controls (especially the equant, demodulation, and individual oscillator volume and phase controls) often have a kind of significant threshold, where a lot of change happens in a small range; the controls expose a lot of "edges." Connecting these parameters with the MIDI Polyphonic Expression (MPE) modulation sources or others opens up a lot of possibilities. I hope that makes it easy to make sounds that are more or differently expressive and dynamic, and to find the sweet spots. 

If you've made it this far, thank you for reading. I'd be interested to hear what you think of it all. Happy music-making and "safe journeys, space fans, wherever you are."
