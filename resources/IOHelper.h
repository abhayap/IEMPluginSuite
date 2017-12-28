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

#pragma once
#include "ambisonicTools.h"

namespace IOTypes {
    class Nothing
    {
    public:
        Nothing() {};
        bool check(AudioProcessor* p, int setting, bool isInput) {return false;};
        int getSize() { return 0; }
    };
    
    template <int maxNumberOfInputChannels = 64>
    class AudioChannels
    {
    public:
        AudioChannels() {};
        ~AudioChannels() {};
        
        bool check(AudioProcessor* p, int setting, bool isInput)
        {
            int previous = nChannels;
            int maxNumInputs = jmin(isInput ? p->getTotalNumInputChannels() : p->getTotalNumOutputChannels(), maxNumberOfInputChannels);
            if (setting == 0 || setting > maxNumberOfInputChannels) nChannels = maxNumInputs; // Auto setting or requested order exceeds highest possible order
            else nChannels = setting;
            maxSize = maxNumInputs;
            return previous != nChannels;
        };
        
        int getMaxSize() { return maxSize; }
        int getSize() { return nChannels; }
        
    private:
        int nChannels;
        int maxSize = maxNumberOfInputChannels;
    };
    
    template <int highestOrder = 7>
    class Ambisonics
    {
    public:
        Ambisonics() {};
        ~Ambisonics() {};
        
        bool check(AudioProcessor* p, int setting, bool isInput)
        {
            int previousOrder = order;
            --setting;
            
            int maxPossibleOrder = jmin(isqrt(isInput ? p->getTotalNumInputChannels() : p->getTotalNumOutputChannels())-1, highestOrder);
            if (setting == -1 || setting > maxPossibleOrder) order = maxPossibleOrder; // Auto setting or requested order exceeds highest possible order
            else order = setting;
            nChannels = square(order+1);
            maxSize = maxPossibleOrder;
            return previousOrder != order;
        };
        
        int getMaxSize() { return maxSize; }
        int getSize() { return getOrder(); }
        int getOrder() { return order; }
        int getNumberOfChannels() { return nChannels; }
        
    private:
        int order;
        int nChannels;
        int maxSize = highestOrder;
    };
}

template <class Input, class Output>
class IOHelper
{
public:
    IOHelper() {}
    virtual ~IOHelper() {}
    
    
    Input input;
    Output output;
    
    bool inputSizeHasChanged;
    bool outputSizeHasChanged;
    
    /** Call to check number available of input and output channels
     
     @inputSetting and @outputSetting should hold the user's setting:
     Audio: 0 equals auto mode, >=1 equals number of channels
     Ambisonics: 0 equals auto mode, >=1 equals Ambisonic order - 1
     E.g.: Ambisonic: 5 -> user set 4th order.
     
     This function should be called in every call of prepareToPlay()
     and at the beginning of the processBlock() with a check if
     the user has changed the input/output settings.
     */
    void checkInputAndOutput(AudioProcessor* p, unsigned int inputSetting, unsigned int outputSetting, bool force = false) {
        if (force || userChangedIOSettings)
        {
            inputSizeHasChanged = false;
            outputSizeHasChanged = false;
            DBG("IOHelper: processors I/O channel counts: " << p->getTotalNumInputChannels() << "/" << p->getTotalNumOutputChannels());
            inputSizeHasChanged = input.check(p, inputSetting, true);
            outputSizeHasChanged = output.check(p, outputSetting, false);
            
            if (inputSizeHasChanged || outputSizeHasChanged)
            {
                DBG("IOHelper:  I/O sizes have changed. calling updateBuffers()");
                updateBuffers();
            }
            
            userChangedIOSettings = false;
        }
    }
    
    void getMaxSize(int& maxInputSize, int& maxOutputSize)
    {
        maxInputSize = input.getMaxSize();
        maxOutputSize = output.getMaxSize();
    }
    
    bool userChangedIOSettings = true;
    
private:
    
    /** Update buffers
     @inputSetting and @outputSetting should hold the user's setting:
     Audio: 0 equals auto mode, >=1 equals number of channels
     Ambisonics: 0 equals auto mode, >=1 equals Ambisonic order - 1
     E.g.: Ambisonic: 5 -> user set 4th order.
     
     This function should be called in every call of prepareToPlay()
     and at the beginning of the processBlock() with a check if
     the user has changed the input/output settings.
     */
    virtual void updateBuffers() {
        DBG("IOHelper:  input size: " << input.getSize());
        DBG("IOHelper: output size: " << output.getSize());
    };
};
