/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Sebastian Grill
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
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

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../resources/FeedbackDelayNetwork.h"

// ===== OSC ====
#include "../../resources/OSCParameterInterface.h"
#include "../../resources/OSCReceiverPlus.h"

//==============================================================================
/**
*/
class FdnReverbAudioProcessor  : public AudioProcessor,
                                public AudioProcessorValueTreeState::Listener,
                                public VSTCallbackHandler,
                                private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>
{
public:
//==============================================================================
    FdnReverbAudioProcessor();
    ~FdnReverbAudioProcessor();

//==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override;

//==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

//==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

//==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

//==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void parameterChanged (const String &parameterID, float newValue) override;

    //======== PluginCanDo =========================================================
    pointer_sized_int handleVstManufacturerSpecific (int32 index, pointer_sized_int value,
                                                     void* ptr, float opt) override { return 0; };
    pointer_sized_int handleVstPluginCanDo (int32 index, pointer_sized_int value,
                                            void* ptr, float opt) override;
    //==============================================================================

    //======== OSC =================================================================
    void oscMessageReceived (const OSCMessage &message) override;
    void oscBundleReceived (const OSCBundle &bundle) override;
    OSCReceiverPlus& getOSCReceiver () { return oscReceiver; }
    //==============================================================================

    //======= Parameters ===========================================================
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    //==============================================================================


//==============================================================================

    int maxPossibleChannels = 64;

    void setFreezeMode (bool freezeState);
    void updateFilterParameters ();
    void getT60ForFrequencyArray (double* frequencies, double* t60Data, size_t numSamples);

private:
    //==============================================================================
    OSCParameterInterface oscParams;
    OSCReceiverPlus oscReceiver;
    AudioProcessorValueTreeState parameters;

	AudioBuffer<float> copyBuffer;

    // parameters (from GUI)
    float *revTime;
	float *fadeInTime;
    float *delayLength;

    float *highCutoff;
    float *highQ;
    float *highGain;
    float *lowCutoff;
    float *lowQ;
    float *lowGain;
    float *wet;


    FeedbackDelayNetwork fdn, fdnFade;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FdnReverbAudioProcessor)
};
