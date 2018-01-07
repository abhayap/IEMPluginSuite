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
#include "ReferenceCountedMatrix.h"
#include "ReferenceCountedDecoder.h"
#include "Eigen/Eigen"
class DecoderHelper {
public:
    
    static Result parseJsonFile (const File& fileToParse, var& dest)
    {
        if (!fileToParse.exists())
            return Result::fail("File '" + fileToParse.getFullPathName() + "' does not exist!");
        
        String jsonString = fileToParse.loadFileAsString();
        Result result = JSON::parse (jsonString, dest);
        if (!result.wasOk())
            return Result::fail("File '" + fileToParse.getFullPathName() + "' could not be parsed:\n" + result.getErrorMessage());
        
        return Result::ok();
    }
    
    static Result transformationMatrixVarToMatrix (var& tmVar, ReferenceCountedMatrix::Ptr* matrix, var nameFallback = var(""), var descriptionFallback = var(""))
    {
        String name = tmVar.getProperty(Identifier("name"), nameFallback);
        String description = tmVar.getProperty(Identifier("description"), descriptionFallback);
        
        if (! tmVar.hasProperty("matrix"))
            return Result::fail("There is no 'matrix' array.");
        
        int rows, cols;
        var matrixData = tmVar.getProperty("matrix", var());
        Result result = getMatrixDataSize (matrixData, rows, cols);

        if (! result.wasOk())
            return Result::fail(result.getErrorMessage());
        
        
        ReferenceCountedMatrix::Ptr newMatrix = new ReferenceCountedMatrix(name, description, rows, cols);
        result = getMatrix(matrixData, rows, cols, newMatrix->getMatrix());
        
        if (! result.wasOk())
            return Result::fail(result.getErrorMessage());
        
        *matrix = newMatrix;
        return Result::ok();
    }
    
    static Result decoderObjectToDecoder (var& decoderObject, ReferenceCountedDecoder::Ptr* decoder, var nameFallback = var(""), var descriptionFallback = var(""))
    {
        String name = decoderObject.getProperty(Identifier("name"), nameFallback);
        String description = decoderObject.getProperty(Identifier("description"), descriptionFallback);
        
        if (! decoderObject.hasProperty("matrix"))
            return Result::fail("There is no 'matrix' array within the 'Decoder' object.");
        
        // get matrix size
        int rows, cols;
        var matrixData = decoderObject.getProperty("matrix", var());
        Result result = getMatrixDataSize (matrixData, rows, cols);
        if (! result.wasOk())
            return Result::fail(result.getErrorMessage());
        
        // create decoder and get matrix from 'Decoder' object
        ReferenceCountedDecoder::Ptr newDecoder = new ReferenceCountedDecoder(name, description, rows, cols);
        result = getMatrix(matrixData, rows, cols, newDecoder->getMatrix());
        if (! result.wasOk())
            return Result::fail(result.getErrorMessage());
        
        ReferenceCountedDecoder::Settings settings;
        // settings
        if (decoderObject.hasProperty("expectedNormalization"))
        {
            var expectedNormalization (decoderObject.getProperty("expectedNormalization", var()));
            if (expectedNormalization.toString().equalsIgnoreCase("sn3d"))
            {
                settings.expectedNormalization = ReferenceCountedDecoder::Normalization::sn3d;
            }
            else if (expectedNormalization.toString().equalsIgnoreCase("n3d"))
            {
                settings.expectedNormalization = ReferenceCountedDecoder::Normalization::n3d;
            }
            else
                return Result::fail("Could not parse 'expectedNormalization' attribute. Expected 'sn3d' or 'n3d' but got '" + expectedNormalization.toString() + "'.");
        }
        
        newDecoder->setSettings(settings);
        
        *decoder = newDecoder;
        return Result::ok();
    }
    
    static Result getMatrixDataSize (var& matrixData, int& rows, int& cols)
    {
        rows = matrixData.size();
        cols = matrixData.getArray()->getUnchecked(0).size();
        
        return Result::ok();
    }
    
    // call getMatrixDataSize() before and create the 'dest' matrix with the resulting size
    static Result getMatrix (var&  matrixData, int rows, int cols, Eigen::MatrixXf* dest)
    {
        for (int r = 0; r < rows; ++r)
        {
            var rowVar = matrixData.getArray()->getUnchecked(r);
            if (rowVar.size() != cols)
                return Result::fail("Matrix row " + String(r+1) + " has wrong length (should be " + String(cols) + ").");
            
            for (int c = 0; c < cols; ++c)
            {
                var colVar = rowVar.getArray()->getUnchecked(c);
                if (colVar.isDouble() || colVar.isInt())
                {
                    dest->coeffRef(r, c) = colVar;
                }
                else
                    return Result::fail("Datatype of matrix element (" + String(r+1) + "," + String(c+1) + ") could not be parsed.");
            }
            
        }
        return Result::ok();
    }
    
    static Result parseFileForDecoder (const File& fileToParse, ReferenceCountedDecoder::Ptr* decoder)
    {
        // parse configuration file
        var parsedJson;
        Result result = parseJsonFile(fileToParse, parsedJson);
        if (! result.wasOk())
            return Result::fail(result.getErrorMessage());

        
        // looks for 'Decoder' object
        if (! parsedJson.hasProperty("Decoder"))
            return Result::fail("No 'Decoder' object found in the configuration file.");
        
        var decoderObject = parsedJson.getProperty("Decoder", parsedJson);
        result = decoderObjectToDecoder (decoderObject, decoder,
                                                         parsedJson.getProperty("name", var("")), parsedJson.getProperty("description", var("")));
        
        if (! result.wasOk())
            return Result::fail(result.getErrorMessage());
        
        return Result::ok();
    }
    
    static Result parseFileForTransformationMatrix (const File& fileToParse, ReferenceCountedMatrix::Ptr* matrix)
    {
        // parse configuration file
        var parsedJson;
        {
        Result result = parseJsonFile(fileToParse, parsedJson);
        if (! result.wasOk())
            return Result::fail(result.getErrorMessage());
        }
        
        // looks for 'TransformationMatrix' object; if missing, it uses the 'root' to look for a 'matrix' object
        var tmVar = parsedJson.getProperty("TransformationMatrix", parsedJson);
        Result result = transformationMatrixVarToMatrix (tmVar, matrix,
                                                         parsedJson.getProperty("name", var("")), parsedJson.getProperty("description", var("")));
        
        if (! result.wasOk())
            return Result::fail(result.getErrorMessage());
        
        return Result::ok();
    }
};
