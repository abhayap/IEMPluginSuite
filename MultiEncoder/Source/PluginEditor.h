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

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "../../resources/Quaternion.h"
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/lookAndFeel/IEM_LaF.h"
#include "../../resources/customComponents/TitleBar.h"
#include "../../resources/customComponents/SimpleLabel.h"
#include "../../resources/customComponents/MuteSoloButton.h"
#include "../../resources/customComponents/SpherePanner.h"
#include "EncoderList.h"



typedef ReverseSlider::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
*/
class MultiEncoderAudioProcessorEditor  : public AudioProcessorEditor,
private Timer,
private SpherePanner::Listener,
private AudioProcessorValueTreeState::Listener
{
public:
    MultiEncoderAudioProcessorEditor (MultiEncoderAudioProcessor&, AudioProcessorValueTreeState&);
    ~MultiEncoderAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    void parameterChanged (const String &parameterID, float newValue) override;
private:
    LaF globalLaF;
    TitleBar<AudioChannelsIOWidget<maxNumberOfInputs>, AmbisonicIOWidget<>> title;
    Footer footer;
    
    void timerCallback() override;
    void mouseWheelOnSpherePannerMoved (SpherePanner* sphere, const MouseEvent &event, const MouseWheelDetails &wheel) override;

    MultiEncoderAudioProcessor& processor;

    GroupComponent quatGroup,ypGroup,settingsGroup;
    ReverseSlider slMasterYaw, slMasterPitch, slMasterRoll;
    
    ToggleButton tbLockedToMaster;
    ComboBox inputChooser;

    SpherePanner sphere;
    SpherePanner::Element masterElement;
    
    AudioProcessorValueTreeState& valueTreeState;
    
    ScopedPointer<SliderAttachment> slMasterYawAttachment;
    ScopedPointer<SliderAttachment> slMasterPitchAttachment;
    ScopedPointer<SliderAttachment> slMasterRollAttachment;
    ScopedPointer<ButtonAttachment> tbLockedToMasterAttachment;
    
    ScopedPointer<ComboBoxAttachment> cbNumInputChannelsAttachment, cbNormalizationAtachment;
    ScopedPointer<ComboBoxAttachment> cbOrderAtachment;

    Viewport viewport;
    EncoderList encoderList;
    
    TooltipWindow toolTipWin;

    int maxPossibleOrder = -1;
    int maxNumInputs = -1;
    int lastSetNumChIn = -1;
    
    // labels
    SimpleLabel lbNum, lbYaw, lbPitch, lbGain;
    SimpleLabel lbMasterYaw, lbMasterPitch, lbMasterRoll;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiEncoderAudioProcessorEditor)
};

