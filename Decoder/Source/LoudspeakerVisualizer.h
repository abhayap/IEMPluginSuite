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

//==============================================================================
/*
*/
class LoudspeakerVisualizer    : public Component, public OpenGLRenderer
{
    struct positionAndColour {
        float position[3];
        float colourId;
    };
    
public:
    LoudspeakerVisualizer(std::vector<R3>& pts, std::vector<Tri>& tris, std::vector<Vector3D<float>>& norms) : extPoints(pts), extTriangles(tris), extNormals(norms)
    {
        openGLContext.setComponentPaintingEnabled(true);
        openGLContext.setContinuousRepainting(false);
        openGLContext.setRenderer(this);
        openGLContext.attachTo(*this);
    }

    ~LoudspeakerVisualizer()
    {
        openGLContext.detach();
        openGLContext.setRenderer(nullptr);
    }
    void setActiveSpeakerIndex (int newIdx)
    {
        activePoint = newIdx;
        updateVerticesAndIndices();
    }
    
    void initialise()
    {
        const float alpha = 0.8;
        PixelARGB colormapData[8];
//        colormapData[0] = Colours::white.withMultipliedAlpha(alpha).getPixelARGB();
//        texture.loadARGB(colormapData, 1, 1);
        
        colormapData[0] = Colours::limegreen.getPixelARGB(); // selected point colour
        colormapData[1] = Colours::cornflowerblue.getPixelARGB(); // regular point colour
        colormapData[2] = Colours::cornflowerblue.withMultipliedAlpha(alpha).getPixelARGB();
        colormapData[3] = Colours::greenyellow.withMultipliedAlpha(alpha).getPixelARGB();
        colormapData[4] = Colours::limegreen.withMultipliedAlpha(alpha).getPixelARGB();
        colormapData[5] = Colours::cornflowerblue.withMultipliedAlpha(alpha).getPixelARGB();
        colormapData[6] = Colours::orange.withMultipliedAlpha(alpha).getPixelARGB();
        colormapData[7] = Colours::red.withMultipliedAlpha(alpha).getPixelARGB();
        
        texture.loadARGB(colormapData, 8, 1);
		

        
        openGLContext.extensions.glActiveTexture (GL_TEXTURE0);
        glEnable (GL_TEXTURE_2D);
        
        
        texture.bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // linear interpolation when too small
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // linear interpolation when too bi
    }
    
    void newOpenGLContextCreated() override
    {
        createShaders();
        initialise();
        updateVerticesAndIndices();
    }
    
    void updateVerticesAndIndices()
    {
        vertices.clear();
        indices.clear();
        normals.clear();
        
        nPoints = (int) extPoints.size();
        
        for (int i = 0; i < nPoints; ++i)
        {
            float col = extPoints[i].lspNum == activePoint ? 0.0f : 0.2f;
            vertices.push_back({extPoints[i].x, extPoints[i].z, extPoints[i].y, col});
            indices.push_back(i);
            normals.push_back(1.0f);
            normals.push_back(1.0f);
            normals.push_back(1.0f);
        }
        
        //normals.insert(normals.end(), extNormals.begin(), extNormals.end());
        
        nTriangles = (int) extTriangles.size();
        for (int i = 0; i < nTriangles;  ++i)
        {
            const float col = 0.4f + 0.6f * ((float) i / nTriangles); // defining a colour
            Vector3D<float> normal = extNormals[i];
            Tri tr = extTriangles[i];
            Vector3D<float> a {extPoints[tr.a].x, extPoints[tr.a].y, extPoints[tr.a].z};
            Vector3D<float> b {extPoints[tr.b].x, extPoints[tr.b].y, extPoints[tr.b].z};
            Vector3D<float> c {extPoints[tr.c].x, extPoints[tr.c].y, extPoints[tr.c].z};
            
            // making sure that triangles are facing outward
            if (normal * ((b-a)^(c-a)) > 0.0f) // correct
            {
                vertices.push_back({a.x, a.z, a.y, col});
                vertices.push_back({b.x, b.z, b.y, col});
            }
            else // incorrect -> swap
            {
                vertices.push_back({b.x, b.z, b.y, col});
                vertices.push_back({a.x, a.z, a.y, col});
            }
            vertices.push_back({c.x, c.z, c.y, col});
            
            indices.push_back(nPoints + i*3);
            indices.push_back(nPoints + i*3 + 1);
            indices.push_back(nPoints + i*3 + 2);
            
            normals.push_back(normal.x);
            normals.push_back(normal.z);
            normals.push_back(normal.y);
            normals.push_back(normal.x);
            normals.push_back(normal.z);
            normals.push_back(normal.y);
            normals.push_back(normal.x);
            normals.push_back(normal.z);
            normals.push_back(normal.y);
        }
        
		updatedBuffers = true;
        
        openGLContext.triggerRepaint();
    }

	void uploadBuffers() // this should only be called by the openGl thread
	{
		openGLContext.extensions.glDeleteBuffers(1, &vertexBuffer);
		openGLContext.extensions.glDeleteBuffers(1, &indexBuffer);
		openGLContext.extensions.glDeleteBuffers(1, &normalsBuffer);

		openGLContext.extensions.glGenBuffers(1, &vertexBuffer);
		openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(positionAndColour), &vertices[0], GL_STATIC_DRAW);

		openGLContext.extensions.glGenBuffers(1, &indexBuffer);
		openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		openGLContext.extensions.glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);

		openGLContext.extensions.glGenBuffers(1, &normalsBuffer);
		openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
		openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);
	}
    
    void renderOpenGL() override
    {
        jassert (OpenGLHelpers::isContextActive());
        
        OpenGLHelpers::clear (Colour(0xFF2D2D2D));

        const float desktopScale = (float) openGLContext.getRenderingScale();
        glViewport (0, 0, roundToInt (desktopScale * getWidth()), roundToInt (desktopScale * getHeight()));
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glEnable (GL_DEPTH_TEST);
        
#ifdef JUCE_OPENGL3
        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
#endif
        
        openGLContext.extensions.glActiveTexture (GL_TEXTURE0);
        glEnable (GL_TEXTURE_2D);
        texture.bind();

		if (updatedBuffers)
		{
			updatedBuffers = false;
			uploadBuffers();
		}

        shader->use();
        GLint programID = shader->getProgramID();
        
        if (viewHasChanged)
        {
            viewHasChanged = false;
            if (projectionMatrix != nullptr)
                projectionMatrix->setMatrix4 (getProjectionMatrix().mat, 1, false);
            if (viewMatrix != nullptr)
                viewMatrix->setMatrix4 (getViewMatrix().mat, 1, false);
        }
        
        
        GLint attributeID;
        
        // 1st attribute buffer : vertices
        attributeID = openGLContext.extensions.glGetAttribLocation(programID, "position");
        openGLContext.extensions.glEnableVertexAttribArray(attributeID);
        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        
        openGLContext.extensions.glVertexAttribPointer(
                                                       attributeID,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                                                       3,                  // size
                                                       GL_FLOAT,           // type
                                                       GL_FALSE,           // normalized?
                                                       sizeof(positionAndColour),                  // stride
                                                       (void*)0            // array buffer offset
                                                       );
        
        // 2nd attribute buffer : normals
        attributeID = openGLContext.extensions.glGetAttribLocation(programID, "normals");
        openGLContext.extensions.glEnableVertexAttribArray(attributeID);
        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
        
        openGLContext.extensions.glVertexAttribPointer(
                                                       attributeID,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                                                       3,                  // size
                                                       GL_FLOAT,           // type
                                                       GL_FALSE,           // normalized?
                                                       0,                  // stride
                                                       (void*)0            // array buffer offset
                                                       );
        
        // 3rd attribute buffer : colors
        attributeID = openGLContext.extensions.glGetAttribLocation(programID, "colormapDepthIn");
        openGLContext.extensions.glEnableVertexAttribArray(attributeID);
        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        
        openGLContext.extensions.glVertexAttribPointer(
                                                       attributeID,            // attribute
                                                       1,                      // size
                                                       GL_FLOAT,               // type
                                                       GL_TRUE,               // normalized?
                                                       sizeof(positionAndColour),                      // stride
                                                       (void*) offsetof(positionAndColour, colourId)                // array buffer offset
                                                       );
 
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
        if (blackFlag != nullptr)
            blackFlag->set(0.0f);
        if (drawPointsFlag != nullptr)
            drawPointsFlag->set(0.0f);

        // render with alpha = 0, to prime the depth buffer
        
        if (alpha != nullptr)
            alpha->set(0.0f);
        glDisable(GL_CULL_FACE);
        glDepthFunc(GL_ALWAYS); // priming depth buffer
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(
                       GL_TRIANGLES,      // mode
                       nTriangles * 3,    // count
                       GL_UNSIGNED_INT,   // type
                       (void*) (nPoints * sizeof(int))           // element array buffer offset
                       );
        
        // setting alpha factor to 1 to actually draw something
        if (alpha != nullptr)
            alpha->set(1.0f);
        
        // draw points
        if (drawPointsFlag != nullptr)
            drawPointsFlag->set(1.0f);
        glPointSize(15.0);
        glDepthFunc(GL_ALWAYS);
        glDrawElements(GL_POINTS, nPoints, GL_UNSIGNED_INT, (void*)0);  // Draw points!
        if (drawPointsFlag != nullptr)
            drawPointsFlag->set(0.0f);
        // draw background first (inward facing triangles in the background -> looking towards us)
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glDepthFunc(GL_ALWAYS);
        glDrawElements(
                       GL_TRIANGLES,      // mode
                       nTriangles * 3,    // count
                       GL_UNSIGNED_INT,   // type
                       (void*) (nPoints * sizeof(int))           // element array buffer offset
                       );
        
        // render black lines (all)
        glLineWidth(5.0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        if (blackFlag != nullptr)
            blackFlag->set(1.0f);
        
        glDrawElements(
                       GL_TRIANGLES,      // mode
                       nTriangles * 3,    // count
                       GL_UNSIGNED_INT,   // type
                       (void*) (nPoints * sizeof(int))           // element array buffer offset
                       );
        
        
        if (blackFlag != nullptr)
            blackFlag->set(0.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL
                      );
        
        // draw foreground triangles
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glDepthFunc(GL_ALWAYS);
        glDrawElements(
                       GL_TRIANGLES,      // mode
                       nTriangles * 3,    // count
                       GL_UNSIGNED_INT,   // type
                       (void*) (nPoints * sizeof(int))           // element array buffer offset
                       );
        
        // render black lines (foreground)
        glLineWidth(5.0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDepthFunc(GL_LEQUAL);
        if (blackFlag != nullptr)
            blackFlag->set(1.0f);
        
        glDrawElements(
                       GL_TRIANGLES,      // mode
                       nTriangles * 3,    // count
                       GL_UNSIGNED_INT,   // type
                       (void*) (nPoints * sizeof(int))           // element array buffer offset
                       );
        
        // draw foreground points
        if (drawPointsFlag != nullptr)
            drawPointsFlag->set(1.0f);
        if (blackFlag != nullptr)
            blackFlag->set(0.0f);
        glPointSize(15.0);
        glDepthFunc(GL_LEQUAL);
        glDrawElements(GL_POINTS, nPoints, GL_UNSIGNED_INT, (void*)0);  // Draw points!
        if (drawPointsFlag != nullptr)
            drawPointsFlag->set(0.0f);
        
        openGLContext.extensions.glDisableVertexAttribArray(openGLContext.extensions.glGetAttribLocation(programID, "position"));
        openGLContext.extensions.glDisableVertexAttribArray(openGLContext.extensions.glGetAttribLocation(programID, "normals"));
        openGLContext.extensions.glDisableVertexAttribArray(openGLContext.extensions.glGetAttribLocation(programID, "colormapDepthIn"));
        
        openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
        openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    
    void openGLContextClosing() override {
        openGLContext.extensions.glDeleteBuffers (1, &vertexBuffer);
        openGLContext.extensions.glDeleteBuffers (1, &indexBuffer);
		openGLContext.extensions.glDeleteBuffers(1, &normalsBuffer);
		texture.release();
    }
    
    void mouseWheelMove (const MouseEvent &e, const MouseWheelDetails &wheel) override
    {
        const double delta = (std::abs (wheel.deltaX) > std::abs (wheel.deltaY) ? -wheel.deltaX : wheel.deltaY);
        
        zoom += delta;
        zoom = jmin(zoom, 5.0f);
        zoom = jmax(zoom, 2.5f);
        viewHasChanged = true;
        openGLContext.triggerRepaint();
    }
    
    void mouseDown (const MouseEvent& e) override
    {
        tiltBeforeDrag = tilt;
        yawBeforeDrag = yaw;
    }
    
    void mouseDrag (const MouseEvent& e) override {
        float deltaY = (float) e.getDistanceFromDragStartY() / 100;
        tilt = tiltBeforeDrag + deltaY;
        tilt = jmin(tilt, (float) M_PI / 2.0f);
        tilt = jmax(tilt, 0.0f);
        
        float deltaX = (float) e.getDistanceFromDragStartX() / 100;
        yaw = yawBeforeDrag + deltaX;
        viewHasChanged = true;
        openGLContext.triggerRepaint();
    }
    
    void createShaders()
    {
        // NOTE: the two lights below are single light sources which produce diffuse light on the body
        // adding speculars would be great
        vertexShader =
        "attribute vec3 position;\n"
        "attribute vec3 normals;\n"
        "attribute float colormapDepthIn;\n"
        "\n"
        "uniform mat4 projectionMatrix;\n"
        "uniform mat4 viewMatrix;\n"
        "uniform float blackFlag;\n"
        "uniform float alpha;\n"
        "uniform float drawPointsFlag;\n"
        "\n"
        "varying float colormapDepthOut;\n"
        "varying float lightIntensity;\n"
        "varying float blackFlagOut;\n"
        "varying float alphaOut;\n"
        "varying float drawPointsFlagOut;\n"
        "void main()\n"
        "{\n"
        "   gl_Position.xyz = 50.0 * position;\n" // scaling leads to a better result
        "   gl_Position.w = 1.0;\n"
        "   gl_Position = projectionMatrix * viewMatrix * gl_Position;\n"
        "   vec4 normal;\n"
        "   normal.xyz = normals;\n"
        "   normal.w = 0.0;\n"
        "   vec4 light = normalize(vec4(-0.8, 0.4, 0.8, 0.0));\n"
	    "   float value;\n"
        "   value = dot (light , viewMatrix * normal);\n"
	    "   lightIntensity = (value > 0.0) ? value : 0.0;\n"
		"   colormapDepthOut = colormapDepthIn;\n"
        "   blackFlagOut = blackFlag;\n"
        "   alphaOut = alpha;\n"
        "   drawPointsFlagOut = drawPointsFlag;\n"
        "}";
        
        fragmentShader =
        "varying float colormapDepthOut;\n"
        "varying float lightIntensity;\n"
        "varying float blackFlagOut;\n"
        "varying float alphaOut;\n"
        "varying float drawPointsFlagOut;\n"
        "uniform sampler2D tex0;\n"
        "void main()\n"
        "{\n"
        "      gl_FragColor = texture2D(tex0, vec2(colormapDepthOut, 0));\n"
        "      if (drawPointsFlagOut != 1.0) gl_FragColor.xyz = gl_FragColor.xyz * (0.2/0.9 + lightIntensity * 0.8/0.9);\n"
        "      if (blackFlagOut == 1.0) gl_FragColor = vec4(0, 0, 0, 1);"
        "      gl_FragColor.w = alphaOut * gl_FragColor.w;\n"
        "}";
        
        ScopedPointer<OpenGLShaderProgram> newShader (new OpenGLShaderProgram (openGLContext));
        String statusText;
        
        if (newShader->addVertexShader (OpenGLHelpers::translateVertexShaderToV3 (vertexShader))
            && newShader->addFragmentShader (OpenGLHelpers::translateFragmentShaderToV3 (fragmentShader))
            && newShader->link())
        {
            shader = newShader;
            shader->use();
            statusText = "GLSL: v" + String (OpenGLShaderProgram::getLanguageVersion(), 2);
            
            projectionMatrix = createUniform (openGLContext, *shader, "projectionMatrix");
            viewMatrix       = createUniform (openGLContext, *shader, "viewMatrix");
            alpha = createUniform (openGLContext, *shader, "alpha");
            blackFlag       = createUniform (openGLContext, *shader, "blackFlag");
            drawPointsFlag       = createUniform (openGLContext, *shader, "drawPointsFlag");
        }
        else
        {
            statusText = newShader->getLastError();
        }
    }
    
    Matrix3D<float> getProjectionMatrix() const
    {
        const float near = 1.0f;
        const float ratio = 1.0f / 3.0f; // similar to focal length (maybe reciprocal)
        const float w = near * ratio;
        const float h = w * getLocalBounds().toFloat().getAspectRatio (false);
        return Matrix3D<float>::fromFrustum (-w, w, -h, h, near, 10000.0f);
    }
    
    Matrix3D<float> createRotationMatrix (Vector3D<float> eulerAngleRadians) const noexcept
    {
        const float cx = std::cos (eulerAngleRadians.x),  sx = std::sin (eulerAngleRadians.x),
        cy = std::cos (eulerAngleRadians.y),  sy = std::sin (eulerAngleRadians.y),
        cz = std::cos (eulerAngleRadians.z),  sz = std::sin (eulerAngleRadians.z);
        
        return Matrix3D<float> ((cy * cz) + (sx * sy * sz), cx * sz, (cy * sx * sz) - (cz * sy), 0.0f,
                                (cz * sx * sy) - (cy * sz), cx * cz, (cy * cz * sx) + (sy * sz), 0.0f,
                                cx * sy, -sx, cx * cy, 0.0f,
                                0.0f, 0.0f, 0.0f, 1.0f);
    }
    
    Matrix3D<float> getViewMatrix()
    {
        Matrix3D<float> translationMatrix (Vector3D<float> (0.0f, 0.0f, -50.0f * zoom)); // move object further away
        Matrix3D<float> tiltMatrix = createRotationMatrix (Vector3D<float> (tilt, 0.0f, 0.0f));
        Matrix3D<float> rotationMatrix = createRotationMatrix (Vector3D<float> (0.0f, yaw, 0.0f));
        return rotationMatrix * tiltMatrix  * translationMatrix;
    }
    
private:
    GLuint vertexBuffer=0, indexBuffer=0, normalsBuffer=0;
    const char* vertexShader;
    const char* fragmentShader;
    ScopedPointer<OpenGLShaderProgram> shader;
    ScopedPointer<OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix, alpha, blackFlag, drawPointsFlag;
    
    static OpenGLShaderProgram::Uniform* createUniform (OpenGLContext& openGLContext, OpenGLShaderProgram& shaderProgram, const char* uniformName)
    {
        if (openGLContext.extensions.glGetUniformLocation (shaderProgram.getProgramID(), uniformName) < 0)
            return nullptr; // Return if error
        return new OpenGLShaderProgram::Uniform (shaderProgram, uniformName);
    }
    
	bool updatedBuffers;
    std::vector<R3>& extPoints;
    std::vector<Tri>& extTriangles;
    std::vector<Vector3D<float>>& extNormals;
    
    std::vector<positionAndColour> vertices;
    std::vector<int> indices;
    std::vector<float> normals;
    
    bool viewHasChanged = true;
    int nVertices;
    int nPoints = 0;
    int activePoint = -1;
    int nTriangles;
    
    float zoom = 5.0f;
    float tilt = 0.0f;
    float tiltBeforeDrag;
    
    float yaw = 0.0f;
    float yawBeforeDrag;
    
    OpenGLTexture texture;
    
    OpenGLContext openGLContext;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoudspeakerVisualizer)
};
