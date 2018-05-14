/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2018 - Institute of Electronic Music and Acoustics (IEM)
 https://iem.at

 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <https://www.gnu.org/licenses/>.
 ==============================================================================
 */

#pragma once

using namespace dsp;
class NoiseBurst
{
public:
    NoiseBurst()
    {
        originalNoise.setSize(1, L);

        // create noise
        Random random;
        for (int i = 0; i < L; ++i)
            originalNoise.setSample(0, i, random.nextFloat() * 2.0f - 1.0f);

        ProcessSpec spec;
        spec.sampleRate = 44100.0f;
        spec.maximumBlockSize = L;
        spec.numChannels = 1;

        IIR::Filter<float> filter;
        filter.coefficients = IIR::Coefficients<float>::makeHighPass(spec.sampleRate, 200.0f);
        AudioBlock<float> ab (originalNoise);
        ProcessContextReplacing<float> context(ab);

        filter.prepare(spec);
        filter.process(context);

        filter.coefficients = IIR::Coefficients<float>::makeFirstOrderLowPass(spec.sampleRate, 200.0f);
        filter.prepare(spec);
        filter.reset();
        filter.process(context);

        // fade-in/-out
        originalNoise.applyGainRamp(0, 0, 1000, 0.0f, 1.0f);
        originalNoise.applyGainRamp(0, L - 10000, 10000, 1.0f, 0.0f);

        // level
        originalNoise.applyGain(Decibels::decibelsToGain(-10.0f));

        // copy buffer
        resampledNoise = originalNoise;
        resampledL = L;
    }

    ~NoiseBurst() {}

    void prepare (const juce::dsp::ProcessSpec spec)
    {
        if (sampleRate != spec.sampleRate)
            resampleNoise (spec.sampleRate);
        sampleRate = spec.sampleRate;

        activeChannel = -1;
        active = false;
    }

    void resampleNoise (double newSampleRate)
    {
        double factorReading = 44100.0 / newSampleRate;
        resampledL = roundToInt (L / factorReading + 0.49);

        MemoryAudioSource memorySource (originalNoise, false, false);
        ResamplingAudioSource resamplingSource (&memorySource, false, 1);

        resamplingSource.setResamplingRatio (factorReading);
        resamplingSource.prepareToPlay (L, 44100.0);

        resampledNoise.setSize(1, resampledL);
        AudioSourceChannelInfo info;
        info.startSample = 0;
        info.numSamples = resampledL;
        info.buffer = &resampledNoise;

        resamplingSource.getNextAudioBlock (info);
    }

    void setChannel (const int channel)
    {
        if (! active.get())
            if (channel > 0 && channel < 65)
            {
                activeChannel = channel;
                active = true;
                currentPosition = 0;
            }
    }

    const bool isActive()
    {
        return active.get();
    }

    void processBuffer (AudioBuffer<float> buffer)
    {
        if (active.get())
        {
            if (activeChannel != -1 && buffer.getNumChannels() >= activeChannel)
            {
                const int bufferSize = buffer.getNumSamples();
                const int copyL = jmin(bufferSize, resampledL - currentPosition);

                FloatVectorOperations::add(buffer.getWritePointer(activeChannel - 1), resampledNoise.getReadPointer(0, currentPosition), copyL);

                currentPosition += copyL;
                if (currentPosition >= resampledL)
                {
                    active = false;
                    activeChannel = -1;
                }
            }
        }
    }

private:
    double sampleRate;

    const int L = 22050;
    int currentPosition;

    int resampledL;
    Atomic<bool> active;
    int activeChannel;

    AudioBuffer<float> originalNoise;
    AudioBuffer<float> resampledNoise;
};
