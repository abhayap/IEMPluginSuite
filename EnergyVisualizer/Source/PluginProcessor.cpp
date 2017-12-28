/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 https://www.iem.at
 
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
EnergyVisualizerAudioProcessor::EnergyVisualizerAudioProcessor()
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
parameters(*this, nullptr)
{
    parameters.createAndAddParameter ("orderSetting", "Ambisonics Order", "",
                                      NormalisableRange<float> (0.0f, 8.0f, 1.0f), 0.0f,
                                      [](float value)
                                      {
                                          if (value >= 0.5f && value < 1.5f) return "0th";
                                          else if (value >= 1.5f && value < 2.5f) return "1st";
                                          else if (value >= 2.5f && value < 3.5f) return "2nd";
                                          else if (value >= 3.5f && value < 4.5f) return "3rd";
                                          else if (value >= 4.5f && value < 5.5f) return "4th";
                                          else if (value >= 5.5f && value < 6.5f) return "5th";
                                          else if (value >= 6.5f && value < 7.5f) return "6th";
                                          else if (value >= 7.5f) return "7th";
                                          else return "Auto";
                                      }, nullptr);
    
    parameters.createAndAddParameter ("useSN3D", "Normalization", "",
                                      NormalisableRange<float> (0.0f, 1.0f, 1.0f), 1.0f,
                                      [](float value)
                                      {
                                          if (value >= 0.5f ) return "SN3D";
                                          else return "N3D";
                                      }, nullptr);
    
    parameters.createAndAddParameter("peakLevel", "Peak level", "dB",
                                     NormalisableRange<float> (-50.0f, 10.0f, 0.1f), 0.0,
                                     [](float value) {return String(value);}, nullptr);

    
    orderSetting = parameters.getRawParameterValue ("orderSetting");
    useSN3D = parameters.getRawParameterValue ("useSN3D");
    peakLevel = parameters.getRawParameterValue ("peakLevel");
    
    parameters.addParameterListener ("orderSetting", this);
    
    parameters.state = ValueTree (Identifier ("EnergyVisualizer"));
    
    Eigen::Matrix<float,64,nSamplePoints> Y;
    // calc Y and YH
    for (int point=0; point<nSamplePoints; ++point)
    {
        SHEval(7, hammerAitovSampleX[point], hammerAitovSampleY[point], hammerAitovSampleZ[point], Y.data()+point*64);
        FloatVectorOperations::multiply(Y.data()+point*64, Y.data()+point*64, sn3d2n3d, 64); //expecting sn3d normalization -> converting it to n3d
    }
    Y *= 1.0f/correction(7); // revert 7th order correction
    YH = Y.transpose();
    
//    DBG(hammerAitovSampleX[218] << " - " << hammerAitovSampleY[218] << " - " << hammerAitovSampleZ[218]);
    rms.resize(nSamplePoints);
}

EnergyVisualizerAudioProcessor::~EnergyVisualizerAudioProcessor()
{
}

//==============================================================================
const String EnergyVisualizerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EnergyVisualizerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EnergyVisualizerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EnergyVisualizerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EnergyVisualizerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EnergyVisualizerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EnergyVisualizerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EnergyVisualizerAudioProcessor::setCurrentProgram (int index)
{
}

const String EnergyVisualizerAudioProcessor::getProgramName (int index)
{
    return {};
}

void EnergyVisualizerAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void EnergyVisualizerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    checkOrderUpdateBuffers(roundFloatToInt(*orderSetting - 1));

    timeConstant = exp(-1.0/(sampleRate*0.1/samplesPerBlock)); //multiplicated value after sampleRate is rms time
    sampledSignals.setSize(nSamplePoints, samplesPerBlock);
}

void EnergyVisualizerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EnergyVisualizerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}
#endif

void EnergyVisualizerAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON); // alternative?: fesetenv(FE_DFL_DISABLE_SSE_DENORMS_ENV);
    
    if (userChangedOrderSettings) checkOrderUpdateBuffers(roundFloatToInt(*orderSetting - 1));
    
    //const int nCh = buffer.getNumChannels();
    const int L = buffer.getNumSamples();
    const int workingOrder = jmin(isqrt(buffer.getNumChannels())-1, ambisonicOrder);
    
    const int nCh = squares[workingOrder+1];
    //DBG(buffer.getNumChannels() << " - " << workingOrder << " - " << nCh);
    
    
    Eigen::Map<const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> inpMatrix (buffer.getReadPointer(0),nCh,L);
    
    Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> outMatrix (sampledSignals.getWritePointer(0), nSamplePoints,L);

    //outMatrix = YH.block(0,0,tDesignN,nCh) * inpMatrix;
    FloatVectorOperations::clear(&maxReWeights.diagonal()[0],64);
    copyMaxRE(workingOrder, &maxReWeights.diagonal()[0]);
    FloatVectorOperations::multiply(&maxReWeights.diagonal()[0], maxRECorrection[workingOrder] * correction(workingOrder), nCh);
    
    if (*useSN3D < 0.5f)
    {
        FloatVectorOperations::multiply(&maxReWeights.diagonal()[0], n3d2sn3d, 64);
    }
    workingMatrix = YH * maxReWeights;
    //workingMatrix = YH; // * maxReWeights;
    outMatrix = workingMatrix.block(0, 0, nSamplePoints, nCh) * inpMatrix;
    
    float* pRms = rms.getRawDataPointer();
    float oneMinusTimeConstant = 1.0f - timeConstant;
    for (int i = 0; i < nSamplePoints; ++i)
    {
        pRms[i] = timeConstant * pRms[i] + oneMinusTimeConstant * ((Decibels::gainToDecibels(sampledSignals.getRMSLevel(i, 0, L)) - *peakLevel) / 35.0f + 1.0f);
    }
    
    FloatVectorOperations::clip(rms.getRawDataPointer(), rms.getRawDataPointer(), 0.0f, 1.0f, nSamplePoints);
}

//==============================================================================
bool EnergyVisualizerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* EnergyVisualizerAudioProcessor::createEditor()
{
    return new EnergyVisualizerAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void EnergyVisualizerAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void EnergyVisualizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
void EnergyVisualizerAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    if (parameterID == "orderSetting") userChangedOrderSettings = true;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EnergyVisualizerAudioProcessor();
}

void EnergyVisualizerAudioProcessor::checkOrderUpdateBuffers(int userSetInputOrder) {
    userChangedOrderSettings = false;
    //old values;
    _nChannels = nChannels;
    _ambisonicOrder = ambisonicOrder;
    DBG(getTotalNumOutputChannels());
    maxPossibleOrder = isqrt(getTotalNumInputChannels())-1;
    if (userSetInputOrder == -1 || userSetInputOrder > maxPossibleOrder) ambisonicOrder = maxPossibleOrder; // Auto setting or requested order exceeds highest possible order
    else ambisonicOrder = userSetInputOrder;

    if (ambisonicOrder != _ambisonicOrder) {
        nChannels = squares[ambisonicOrder+1];
        DBG("Used order has changed! Order: " << ambisonicOrder << ", numCH: " << nChannels);
        DBG("Now updating filters and buffers.");
    }
}
