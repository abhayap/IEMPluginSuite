/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
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

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
ToolBoxAudioProcessor::ToolBoxAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::discreteChannels(64), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::discreteChannels(64), true)
                     #endif
                       ),
#endif
parameters(*this, nullptr), oscParams (parameters), flipXMask(int64(0)), flipYMask(int64(0)), flipZMask(int64(0))
{
    oscParams.createAndAddParameter ("inputOrderSetting", "Input Ambisonic Order", "",
                                      NormalisableRange<float> (0.0f, 8.0f, 1.0f), 0.0f,
                                      [](float value) {
                                          if (value >= 0.5f && value < 1.5f) return "0th";
                                          else if (value >= 1.5f && value < 2.5f) return "1st";
                                          else if (value >= 2.5f && value < 3.5f) return "2nd";
                                          else if (value >= 3.5f && value < 4.5f) return "3rd";
                                          else if (value >= 4.5f && value < 5.5f) return "4th";
                                          else if (value >= 5.5f && value < 6.5f) return "5th";
                                          else if (value >= 6.5f && value < 7.5f) return "6th";
                                          else if (value >= 7.5f) return "7th";
                                          else return "Auto";},
                                      nullptr);

    oscParams.createAndAddParameter ("useSn3dInput", "Input Normalization", "",
                                     NormalisableRange<float>(0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) {
                                         if (value >= 0.5f) return "SN3D";
                                         else return "N3D";
                                     }, nullptr);

    oscParams.createAndAddParameter ("outputOrderSetting", "Output Ambisonic Order", "",
                                      NormalisableRange<float> (0.0f, 8.0f, 1.0f), 0.0f,
                                      [](float value) {
                                          if (value >= 0.5f && value < 1.5f) return "0th";
                                          else if (value >= 1.5f && value < 2.5f) return "1st";
                                          else if (value >= 2.5f && value < 3.5f) return "2nd";
                                          else if (value >= 3.5f && value < 4.5f) return "3rd";
                                          else if (value >= 4.5f && value < 5.5f) return "4th";
                                          else if (value >= 5.5f && value < 6.5f) return "5th";
                                          else if (value >= 6.5f && value < 7.5f) return "6th";
                                          else if (value >= 7.5f) return "7th";
                                          else return "Auto";},
                                      nullptr);

    oscParams.createAndAddParameter ("useSn3dOutput", "Output Normalization", "",
                                     NormalisableRange<float>(0.0f, 1.0f, 1.0f), 1.0f,
                                     [](float value) {
                                         if (value >= 0.5f) return "SN3D";
                                         else return "N3D";
                                     }, nullptr);

    oscParams.createAndAddParameter ("flipX", "Flip X axis", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0f,
                                     [](float value) {if (value >= 0.5f) return "ON";
                                         else return "OFF";}, nullptr);

    oscParams.createAndAddParameter ("flipY", "Flip Y axis", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0f,
                                     [](float value) {if (value >= 0.5f) return "ON";
                                         else return "OFF";}, nullptr);

    oscParams.createAndAddParameter ("flipZ", "Flip Z axis", "",
                                     NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0f,
                                     [](float value) {if (value >= 0.5f) return "ON";
                                         else return "OFF";}, nullptr);

    oscParams.createAndAddParameter ("loaWeights", "Lower Order Ambisonic Weighting", "",
                                      NormalisableRange<float> (0.0f, 2.0f, 1.0f), 0.0f,
                                      [](float value) {
                                          if (value >= 0.5f && value < 1.5f) return "maxrE";
                                          else if (value >= 1.5f) return "inPhase";
                                          else return "none";},
                                      nullptr);


    // this must be initialised after all calls to createAndAddParameter().
    parameters.state = ValueTree (Identifier ("ToolBox"));
    // tip: you can also add other values to parameters.state, which are also saved and restored when the session is closed/reopened


    // get pointers to the parameters
    inputOrderSetting = parameters.getRawParameterValue("inputOrderSetting");
    outputOrderSetting = parameters.getRawParameterValue ("outputOrderSetting");
    useSn3dInput = parameters.getRawParameterValue ("useSn3dInput");
    useSn3dOutput = parameters.getRawParameterValue ("useSn3dOutput");
    flipX = parameters.getRawParameterValue ("flipX");
    flipY = parameters.getRawParameterValue ("flipY");
    flipZ = parameters.getRawParameterValue ("flipZ");
    loaWeights = parameters.getRawParameterValue("loaWeights");


    // add listeners to parameter changes
    parameters.addParameterListener ("inputOrderSetting", this);
    parameters.addParameterListener ("outputOrderSetting", this);
    parameters.addParameterListener ("flipX", this);
    parameters.addParameterListener ("flipY", this);
    parameters.addParameterListener ("flipZ", this);


    // calculate flip masks


    for (int ch = 0; ch < 64; ++ch)
    {
        int l, m;
        ACNtoLM(ch, l, m);

        if (((m < 0) && (m % 2 == 0)) || ((m > 0) && (m % 2 == 1)))
            flipXMask.setBit(ch);

        if (m < 0)
            flipYMask.setBit(ch);

        if ((l + m) % 2)
            flipZMask.setBit(ch);
    }

    oscReceiver.addListener (this);
}

ToolBoxAudioProcessor::~ToolBoxAudioProcessor()
{
}

//==============================================================================
const String ToolBoxAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ToolBoxAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ToolBoxAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ToolBoxAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ToolBoxAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ToolBoxAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ToolBoxAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ToolBoxAudioProcessor::setCurrentProgram (int index)
{
}

const String ToolBoxAudioProcessor::getProgramName (int index)
{
    return {};
}

void ToolBoxAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void ToolBoxAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkInputAndOutput(this, *inputOrderSetting, *outputOrderSetting, true);

    doFlipX = *flipX >= 0.5f;
    doFlipY = *flipY >= 0.5f;
    doFlipZ = *flipZ >= 0.5f;
}

void ToolBoxAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ToolBoxAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void ToolBoxAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    checkInputAndOutput(this, *inputOrderSetting, *outputOrderSetting, false);
    ScopedNoDenormals noDenormals;

    const int nChIn = jmin(buffer.getNumChannels(), input.getNumberOfChannels());
    const int nChOut = jmin(buffer.getNumChannels(), output.getNumberOfChannels());
    const int nCh = jmin(nChIn, nChOut);

    const int L = buffer.getNumSamples();

    const int orderIn = input.getOrder();
    const int orderOut = output.getOrder();

    float weights[64];
    FloatVectorOperations::fill(weights, 1.0f, nCh);

    // create mask for all flips
    if (doFlipX || doFlipY || doFlipZ)
    {
        BigInteger mask (int64(0));
        if (doFlipX)
            mask ^= flipXMask;
        if (doFlipY)
            mask ^= flipYMask;
        if (doFlipZ)
            mask ^= flipZMask;

        for (int ch = 0; ch < nCh; ++ch)
            if (mask[ch])
                weights[ch] *= -1.0f;
    }


    // lower order ambisonics weighting
    if (orderIn < orderOut)
    {
        const int weightType = roundToInt(*loaWeights);
        if (weightType == 1) // maxrE
        {
            FloatVectorOperations::multiply(weights, getMaxRELUT(orderIn), nChIn);
            const float* deWeights = getMaxRELUT(orderOut);
            for (int i = 0; i < nChIn; ++i)
                weights[i] /= deWeights[i];
        }
        else if (weightType == 2) // inPhase
        {
            FloatVectorOperations::multiply(weights, getInPhaseLUT(orderIn), nChIn);
            const float* deWeights = getInPhaseLUT(orderOut);
            for (int i = 0; i < nChIn; ++i)
                weights[i] /= deWeights[i];
        }
    }


    // normalization
    bool inSN3D = *useSn3dInput >= 0.5f;
    bool outSN3D = *useSn3dOutput >= 0.5f;
    if (inSN3D != outSN3D)
    {
        if (inSN3D)
            FloatVectorOperations::multiply(weights, sn3d2n3d, nCh);
        else
            FloatVectorOperations::multiply(weights, n3d2sn3d, nCh);
    }



    // apply weights;
    for (int ch = 0; ch < nCh; ++ch)
    {
        if (weights[ch] != 1.0f)
        {
            FloatVectorOperations::multiply(buffer.getWritePointer(ch), weights[ch], L);
        }
    }

    // clear not used channels
    for (int ch = nCh; ch < buffer.getNumChannels(); ++ch)
        buffer.clear(ch, 0, L);



}

//==============================================================================
bool ToolBoxAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ToolBoxAudioProcessor::createEditor()
{
    return new ToolBoxAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void ToolBoxAudioProcessor::getStateInformation (MemoryBlock &destData)
{
    auto state = parameters.copyState();
    state.setProperty ("OSCPort", var(oscReceiver.getPortNumber()), nullptr);
    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void ToolBoxAudioProcessor::setStateInformation (const void *data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (ValueTree::fromXml (*xmlState));
            if (parameters.state.hasProperty ("OSCPort"))
            {
                oscReceiver.connect (parameters.state.getProperty ("OSCPort", var (-1)));
            }
        }
}

//==============================================================================
void ToolBoxAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    DBG("Parameter with ID " << parameterID << " has changed. New value: " << newValue);

    if (parameterID == "inputOrderSetting" || parameterID == "outputOrderSetting" )
        userChangedIOSettings = true;
    else if (parameterID == "flipX")
        doFlipX = newValue >= 0.5f;
    else if (parameterID == "flipY")
        doFlipY = newValue >= 0.5f;
    else if (parameterID == "flipZ")
        doFlipZ = newValue >= 0.5f;

}

void ToolBoxAudioProcessor::updateBuffers()
{
    DBG("IOHelper:  input size: " << input.getSize());
    DBG("IOHelper: output size: " << output.getSize());
}

//==============================================================================
pointer_sized_int ToolBoxAudioProcessor::handleVstPluginCanDo (int32 index,
                                                                     pointer_sized_int value, void* ptr, float opt)
{
    auto text = (const char*) ptr;
    auto matches = [=](const char* s) { return strcmp (text, s) == 0; };

    if (matches ("wantsChannelCountNotifications"))
        return 1;
    return 0;
}

//==============================================================================
void ToolBoxAudioProcessor::oscMessageReceived (const OSCMessage &message)
{
    String prefix ("/" + String(JucePlugin_Name));
    if (! message.getAddressPattern().toString().startsWith (prefix))
        return;

    OSCMessage msg (message);
    msg.setAddressPattern (message.getAddressPattern().toString().substring(String(JucePlugin_Name).length() + 1));

    oscParams.processOSCMessage (msg);
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ToolBoxAudioProcessor();
}