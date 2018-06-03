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

#include "../../OmniCompressor/JuceLibraryCode/JuceHeader.h"
#include "../Compressor.h"

//==============================================================================
/*
*/
class CompressorVisualizer    : public Component
{
    class GridAndLabels : public Component
    {
    public:
        GridAndLabels (float minDB) : minDecibels (minDB)
        {
            setBufferedToImage(true);
            createGrid();
        }

        ~GridAndLabels ()
        {

        }

        void createGrid ()
        {
            const float step = 10.0f;

            grid.clear();

            for (int val = - step; val > minDecibels; val -= step)
            {
                // add horizontal line
                grid.startNewSubPath (minDecibels, val);
                grid.lineTo (0.0f, val);

                // add vertical line
                grid.startNewSubPath (val, minDecibels);
                grid.lineTo (val, 0.0f);
            }
        }

        void paint (Graphics& g) override
        {

            g.setColour (Colours::steelblue.withMultipliedAlpha (0.8f));
            g.strokePath (grid, PathStrokeType (0.5f), contentTransform);

            g.setColour (Colours::white.withMultipliedAlpha (0.5f));
            g.drawRect (contentBounds, 1.0f);

            Line<float> unity (minDecibels + 0.5f, minDecibels  + 0.5f, -0.5f, -0.5f);
            unity.applyTransform(contentTransform);
            float dashLengths[2];
            dashLengths[0] = 2.0f;
            dashLengths[1] = 2.0f;
            g.drawDashedLine(unity, dashLengths, 2, 0.5f);

        }

        void resized() override
        {
            auto bounds = getLocalBounds();
            bounds.removeFromTop(20);
            bounds.removeFromRight(50);
            bounds.removeFromLeft(10);
            bounds.removeFromBottom(5);
            contentBounds = bounds;

            contentTransform = AffineTransform::fromTargetPoints(Point<int>(minDecibels, minDecibels), contentBounds.getBottomLeft(), Point<int>(0, 0), contentBounds.getTopRight(), Point<int>(0, minDecibels), contentBounds.getBottomRight());
        }

        Rectangle<int> getBoundsForContent()
        {
            return contentBounds;
        }

        AffineTransform getTransformForContent()
        {
            return contentTransform;
        }

    private:
        const float minDecibels;
        Path grid;

        Rectangle<int> contentBounds {0, 0, 1, 1};
        AffineTransform contentTransform;
    };

    class Characteristic : public Component
    {
    public:
        Characteristic (Compressor& compressorToGetCharacteristicFrom, float minDB) : compressor(compressorToGetCharacteristicFrom), minDecibels (minDB)
        {
            setBufferedToImage(true);
        }

        ~Characteristic()
        {

        }

        void updateCharacteristic ()
        {
            const float threshold = compressor.getTreshold();
            const float knee = compressor.getKnee();

            const float kneeStart = threshold - knee / 2.0f;
            const float kneeEnd = threshold + knee / 2.0f;

            characteristic.clear();
            characteristic.startNewSubPath (minDecibels - 1.0f, minDecibels - 1.0f);
            characteristic.lineTo (minDecibels, compressor.getCharacteristicSample(minDecibels));

            characteristic.lineTo (kneeStart, compressor.getCharacteristicSample(kneeStart));

            const int kneeSamples = jmax(1, static_cast<int> (knee));
            float val = kneeStart;
            float step = knee / kneeSamples;
            for (int i = 0; i < kneeSamples; ++i)
            {
                val += step;
                characteristic.lineTo (val, compressor.getCharacteristicSample(val));
            }
            characteristic.lineTo (kneeEnd, compressor.getCharacteristicSample(kneeEnd));
            characteristic.lineTo (0.0f, compressor.getCharacteristicSample(0.0f));
            characteristic.lineTo (1.0f, compressor.getCharacteristicSample(0.0f));
            characteristic.lineTo (1.0f, minDecibels - 1.0f);
            characteristic.closeSubPath();

            repaint();
        }

        void paint (Graphics& g) override
        {
            g.setColour (Colours::white);
            g.strokePath (characteristic, PathStrokeType(2.0f), transform);

            g.setColour (Colours::steelblue.withMultipliedAlpha (0.3f));
            g.fillPath (characteristic, transform);
        }

        void resized() override
        {
        }

        void setTransformForContent (AffineTransform newTransform)
        {
            transform = newTransform;
        }

    private:
        Compressor& compressor;
        const float minDecibels;

        Path characteristic;

        AffineTransform transform;
    };


public:
    CompressorVisualizer (Compressor& compressorToVisualize) : compressor(compressorToVisualize), minDecibels(-60.0f), gridAndLabels(minDecibels), characteristic(compressor, minDecibels)
    {
        init();
    }

    CompressorVisualizer (Compressor& compressorToVisualize, const float rangeInDecibels) : compressor(compressorToVisualize), minDecibels (-1.0f * rangeInDecibels), gridAndLabels(minDecibels), characteristic(compressor, minDecibels)
    {
        init();
    }

    ~CompressorVisualizer()
    {
    }

    void init()
    {
        addAndMakeVisible(gridAndLabels);
        addAndMakeVisible(characteristic);
    }

    void updateCharacteristic ()
    {
        characteristic.updateCharacteristic();
    }

    void setMarkerLevels (const float inputLevel, const float gainReduction)
    {
        const float makeUpGain = compressor.getMakeUpGain();
        inLevel = inputLevel;
        outLevel = inLevel + gainReduction + makeUpGain;
    }

    void paint (Graphics& g) override
    {

    }

    void paintOverChildren (Graphics& g) override
    {
        g.setColour (Colours::grey);
        g.drawRect (getLocalBounds(), 1);   // draw an outline around the component


        Rectangle<float> circle (0.0f, 0.0f, 10.0f, 10.0f);

        float x = inLevel;
        float y = outLevel;
        transform.transformPoint(x, y);
        circle.setCentre(x, y);

        g.setColour (Colours::cornflowerblue);
        g.fillRoundedRectangle(circle, 5.0f);

    }

    void resized() override
    {
        gridAndLabels.setBounds (getLocalBounds());
        const Rectangle<int> contentBounds = gridAndLabels.getBoundsForContent();
        transform = gridAndLabels.getTransformForContent();

        characteristic.setTransformForContent (transform.translated(- contentBounds.getX(), - contentBounds.getY()));
        characteristic.setBounds (contentBounds);
    }

private:
    Compressor& compressor;
    const float minDecibels;
    GridAndLabels gridAndLabels;
    Characteristic characteristic;
    AffineTransform transform;

    float inLevel = 0.0f;
    float outLevel = 0.0f;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorVisualizer)
};
