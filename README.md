# GDSynth

At the end of 2025 I presented a talk at GDS conference in Prague entitled “Essentials of Procedural Game Audio”.
The talk introduced the concept of procedural audio from the perspective of low-level programming in C++
by illustrating the fundamentals of digital synthesis and voice management on the architecture of a custom
micro-middleware called GDSynth. The architecture involves a command thread for managing events and an audio
thread for real-time rendering and mixing. Various audio operators are discussed, including square wave
generators (simulating retro hardware like the Atari POKEY), sine waves (usable also for a simple FM synthesis,
and noise generators.

The presentation also details the lifecycle of audio events, which transition through states like
"PLAYING," "STOP_REQUESTED," and "RELEASED". It demonstrates practical applications of these concepts through
examples such as a "Coin Pickup" beep, pitch-swept "Drops," a step sequencer for melodies, and additive
synthesis for an "Electric Car" sound that responds to RTPCs. Finally, it covers how to integrate these
procedural sounds into applications using sample buffers and event-based triggers.

In the GDSynth architecture, audio generation is handled through a hierarchy of operators and events that
manage everything from basic waveforms to complex sound behaviors.

## Individual Operators
Operators are the fundamental building blocks of sound in this middleware. They are simple signal generators that provide a getNextSample() function to be called by the engine.

 * Square Wave Operator: This operator simulates retro hardware (like the Atari 800XL POKEY). It uses integer frequency division, where parameters like Period and Duty Cycle (for PWM) are integers to mimic the constraints of old computers. Pitch is determined by the number of samples per period.
 * Sinewave Operator: A basic generator that calculates values using the formula sin(2 * PI * frequency * n / sample_rate) * volume.
 * Noise Operator: A simple generator that produces random values between -1 and 1, adjusted by a volume parameter.

## Creating and Adding Events
Events wrap operators in more complex logic, defining how a sound behaves over time and how it responds to user input.

 * Creating an Event: To create an event, you instantiate a specific event class (see any of the CEvent* classes). Each event must implement a fillFloatBuffer() method to render its audio data and can optionally respond to Real-Time Parameter Controls (RTPCs).
 * Adding/Starting an Event: Events are added to the engine using a command like engine.createEvent(new CEventName()). This typically happens on a "Command Thread," which handles requests to play, stop, or update parameters, while the "Audio Thread" performs the actual rendering.
 * Event States: Once added, an event progresses through various states such as PLAYING, STOP_REQUESTED (for sustained sounds), BEING_STOPPED, and finally RELEASED.

## Slides:
https://docs.google.com/presentation/d/13fIBSTILs6tugZs3bfDo93LEXTKgGH_3LsaZaJT6IyE

```
// GDSynth
// Copyright 2025 Adam Sporka
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the “Software”), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
```
