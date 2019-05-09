/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  OpenGLOcean.h
//  Stonefish
//
//  Created by Patryk Cieslak on 19/07/2017.
//  Copyright (c) 2017-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLOcean__
#define __Stonefish_OpenGLOcean__

#include <SDL2/SDL_mutex.h>
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    //! A structure holding parameters of ocean generation and rendering.
    struct OceanParams
    {
        unsigned int passes;
        unsigned int slopeVarianceSize;
        int fftSize;
        glm::vec4 gridSizes;
        float* spectrum12;
        float* spectrum34;
        bool propagate; //wave propagation
        float wind; //wind speed in meters per second (at 10m above surface)
        float omega; //sea state (inverse wave age)
        float A; //wave amplitude
        float km;
        float cm;
        float t;
    };
    
    class GLSLShader;
    
    //! A class implementing ocean simulation in OpenGL.
    class OpenGLOcean
    {
    public:
        //! A constructor.
        /*!
         \param geometricWaves the state of the ocean, if >0 the ocean is rendered with geometric waves otherwise as a plane with wave texture
         \param hydrodynamics a pointer to a mutex
         */
        OpenGLOcean(float geometricWaves, SDL_mutex* hydrodynamics);
        
        //! A destructor.
        ~OpenGLOcean();
        
        //! A method that simulates wave propagation.
        void Simulate();
        
        //! A method that updates the wave mesh.
        /*!
         \param eyePos the eye position of the active camera
         \param view the view matrix of the active camera
         \param projection the projection matrix of the active camera
         */
        void UpdateSurface(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection);
        
        //! A method that draws the surface of the ocean.
        /*!
         \param eyePos the eye position of the active camera
         \param view the view matrix of the active camera
         \param projection the projection matrix of the active camera
         \param viewport a pointer to viewport data
         */
        void DrawSurface(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection, GLint* viewport);
        
        //! A method that draws the surface of the ocean, seen from underwater.
        /*!
         \param eyePos the eye position of the active camera
         \param view the view matrix of the active camera
         \param projection the projection matrix of the active camera
         \param viewport a pointer to viewport data
         */
        void DrawBacksurface(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection, GLint* viewport);
        
        //! A method that draws the distant background of the ocean.
        /*!
         \param eyePos the eye position of the active camera
         \param view the view matrix of the active camera
         \param projection the projection matrix of the active camera
         */
        void DrawBackground(glm::vec3 eyePos, glm::mat4 view, glm::mat4 infProjection);
        
        //! A method that generates the stencil mask.
        /*!
         \param view the view matrix of the active camera
         \param projection the projection matrix of the active camera
         \param infProjection the infinite projection matrix of the active camera
         */
        void DrawUnderwaterMask(glm::mat4 view, glm::mat4 projection, glm::mat4 infProjection);
        
        //! Method that draws the particles in the ocean and the blur (scattering).
        /*!
         \param eyePos the eye position of the active camera
         \param view the view matrix of the active camera
         \param projection the projection matrix of the active camera
         \param sceneTexture an id of the color texture
         \param linearDepthTex an id of the depth map texture
         \param viewport a pointer to viewport data
         */
        void DrawVolume(glm::vec3 eyePos, glm::mat4 view, glm::mat4 projection, GLuint sceneTexture, GLuint linearDepthTex, GLint* viewport);
        
        //! A method that draws the waterline when the camera is crossing the water surface.
        void DrawWaterline();
        
        //! A method that drawsd the spectrum of waves.
        /*!
         \param viewportSize size of the active viewport
         \param rect a rectangle in which to draw the spectrum on screen
         */
        void ShowSpectrum(glm::vec2 viewportSize, glm::vec4 rect);
        
        //! A method used to display a specified texture.
        /*!
         \param id the id of the texture to display
         \param rect a rectangle in which to draw the tecture on screen
         */
        void ShowTexture(int id, glm::vec4 rect);
       
        //! A method to set the type of ocean water.
        /*!
         \param t type of water
         */
        void setWaterType(GLfloat t);
        
        //! A method to set the turbidity of ocean water.
        /*!
         \param t water turbidity
         */
        void setTurbidity(GLfloat t);
        
        //! A method to get wave height at a specified coordinate.
        /*!
         \param x the x coordinate in world frame [m]
         \param y the y coordinate in world frame [m]
         \return wave height [m]
         */
        GLfloat getWaveHeight(GLfloat x, GLfloat y);
        
        //! A method returning calculated light absorption coefficients.
        glm::vec3 getLightAbsorption();
        
        //! A method returning the turbidity of the water.
        GLfloat getTurbidity();
        
    private:
        GLfloat* ComputeButterflyLookupTable(unsigned int size, unsigned int passes);
        GLfloat ComputeInterpolatedWaveData(GLfloat x, GLfloat y, GLuint channel);
        int bitReverse(int i, int N);
        void computeWeight(int N, int k, float &Wr, float &Wi);
        float ComputeSlopeVariance();
        float GetSlopeVariance(float kx, float ky, float *spectrumSample);
        void GenerateWavesSpectrum();
        void GetSpectrumSample(int i, int j, float lengthScale, float kMin, float *result);
        float spectrum(float kx, float ky, bool omnispectrum = false);
        float omega(float k);
        float sqr(float x);
        
        std::vector<GLSLShader*> oceanShaders;
        GLuint oceanFBOs[3];
        GLuint oceanTextures[6];
        glm::vec3 absorption[64];
        
        bool waves;
        SDL_mutex* hydroMutex;
        GLfloat* fftData;
        OceanParams params;
        QuadTree* qt;
        int tesselation;
        int64_t lastTime;
        GLuint vao;
        GLuint vbo;
        GLuint vaoEdge;
        GLuint vboEdge;
        GLuint vaoMask;
        GLuint vboMask;
        glm::vec3 lightAbsorption;
        GLfloat turbidity;
    };
}

#endif
